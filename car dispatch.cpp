#include<iostream>
#include<fstream>
#include<string>
#include<queue>
#include<unordered_map>
#include<functional>
#include<vector>
#include<list>
#include<algorithm>
using namespace std;
struct car_sort
{
	int id,time;
	friend bool operator<(const car_sort &c1, const car_sort &c2)
	{
		if (c1.time >c2.time) return true;
		else if (c1.time == c2.time&&c1.id>c2.id) return true;
		return false;
	}
};
struct car_message
{
	int start, end, highspeed, time, actual_start_time,actual_end_time, condition, next_direction, next_road, next_single_road,can_not_move,wait_car_id;//condition=tΪ��ֹ״̬������ȴ�.next_directionΪ�ĳ�����һ���ķ���0δ����1��գ�2ֱ�У�3�ҹ�,next_single_road=-1,1��ʾ��������������
	list<int> path_road;
	list<int> path_cross;

};
struct car_in_road
{
	vector<vector<int>> lane;//����ռ�����
	int car_number;
	int termination_number;//��ֹ����
	int cannot_move_number;//���ε��ȸõ�·������ֹ��ǰ�����ڵȴ��������ƶ��ĳ�����(��ȥ���ȴ����еĳ���������
	car_in_road(){ car_number = 0; termination_number = 0; cannot_move_number = 0; }
};
struct road_message
{
	int length, limit_speed, channel, start, end, isDuplex;
	car_in_road forward_road,reverse_road;
};
struct cross_message
{
	vector<int> road;
	int x, y;
	cross_message(){ x = 0; y = 0; }
};
struct road_in_cross//·���ڸ���·����·ID������е���
{
	int index, id;
	road_in_cross(){};
	road_in_cross(int i, int j){ index = i; id = j; }
	friend bool operator<(const road_in_cross &r1, const road_in_cross &r2)
	{
		if (r1.id>r2.id) return true;
		return false;
	}
};
struct car_direct_confirm//����ȷ��������һ����ʻ����
{
	int next_direction, road_id,single_road, select_next_single_road_channel, first_blockcar_IN_select_next_single_road_channel;//single_road=-1,1��ʾ��������������select_next_single_road_channelҪ�ߵĳ�����š�first_blockcar_IN_select_next_single_road_channel��¼��һ���谭�����ڵĳ�����
	double cost;
	bool confirmed = false;//����ó�֮ǰ��û��ȷ���������п��ܸ�ʱ��Ƭ�ڳ���ȷ������Ϊ��գ���������ֱ�ж����ã�ʱ��Ƭ����һ���ֵ��ó�����ʱ����������ȷ������ֻҪ���������Ϣ����
	bool have_wait_car;
	car_direct_confirm(){}
	car_direct_confirm(int i, int j, double k, int m, int n, int p, bool bo1, bool bo2){ next_direction = i; road_id = j; cost = k; single_road = m; select_next_single_road_channel = n; first_blockcar_IN_select_next_single_road_channel = p; confirmed = bo1; have_wait_car = bo2; }
	friend bool operator<(const car_direct_confirm &r1, const car_direct_confirm &r2)
	{
		if (r1.cost>r2.cost) return true;
		return false;
	}
};
unordered_map<int, road_message> map_road;
unordered_map<int, car_message> map_car;
priority_queue<car_sort> que_car;
vector<cross_message> v_cross;
vector<int> v_car_id;
vector<list<int>> v_neighbour;
int sum_line;//��·����
int sum_rank;//��·����
int sum_car = 0;//�ܳ�����
int arrived_car = 0;//�ѵ���Ŀ�ĵس�����
int sum_carINroad = 0;//��·�ϵĳ�������
int t = 0;//��ʼʱ��
int temp_mination_number = 0;//��·����ֹ״̬������Ŀ
int road_capacity = 0;
int last_time = 0;
bool bo_dead_lock = false;
int dead_clock_number = 0;
void read_txt()
{
	//��ȡ����
	ifstream infile_car("F:\\car.txt", ios::in);
	while (!infile_car.eof())
	{
		string str;
		getline(infile_car, str);
		if (str[0] == '#') continue;
		str.erase(str.begin());
		str.erase(--str.end());
		char b[30];
		strcpy_s(b, str.c_str());
		//str.copy(b, str.length(), 0);
		car_sort c;
		car_message car;
		c.id = atoi(strtok(b, ","));
		v_car_id.push_back(c.id);
		car.start = atoi(strtok(NULL, ","));
		car.end = atoi(strtok(NULL, ","));
		car.highspeed = atoi(strtok(NULL, ","));
		car.time=c.time = atoi(strtok(NULL, ","));
		last_time = max(last_time, car.time);
		car.actual_start_time = 0;
		car.actual_end_time = 0;
		car.condition = 0;
		car.wait_car_id = 0;
		car.next_direction = 0;
		car.next_road = 0;
		car.next_single_road = 0;
		car.can_not_move = 0;
		que_car.push(c);
		map_car.insert(make_pair(c.id, car));
	}
	infile_car.close();
	//��ȡ��·
	ifstream infile_road("F:\\road.txt", ios::in);
	while (!infile_road.eof())
	{
		string str;
		getline(infile_road, str);
		if (str[0] == '#') continue;
		str.erase(str.begin());
		str.erase(--str.end());
		char b[30];
		strcpy_s(b, str.c_str());
		//str.copy(b, str.length(), 0);
		road_message r;
		int id;
		id = atoi(strtok(b, ","));
		r.length= atoi(strtok(NULL, ","));
		r.limit_speed= atoi(strtok(NULL, ","));
		r.channel = atoi(strtok(NULL, ","));
		r.start= atoi(strtok(NULL, ","));
		r.end = atoi(strtok(NULL, ","));
		r.isDuplex = atoi(strtok(NULL, ","));
		vector<vector<int>> t_v(r.length, vector<int>(r.channel, 0));
		car_in_road in_r;
		in_r.lane = t_v;
		r.forward_road = in_r;
		r.reverse_road = in_r;
		if (r.isDuplex==1)
		   road_capacity += (r.channel*r.length * 2);
		else road_capacity += (r.channel*r.length * 1);
		map_road.insert(make_pair(id, r));
	}
	infile_road.close();
	//��ȡ·��
	cross_message t_c;
    vector<int> v1(4,0);
	t_c.road = v1;
	v_cross.push_back(t_c);
	ifstream infile_cross("F:\\cross.txt", ios::in);
	while (!infile_cross.eof())
	{
		string str;
		getline(infile_cross, str);
		if (str[0] == '#') continue;
		str.erase(str.begin());
		str.erase(--str.end());
		char b[30];
		strcpy_s(b, str.c_str());
		//str.copy(b, str.length(), 0);
		vector<int> v(4, -1);
		int id= atoi(strtok(b, ","));
		v[0] = atoi(strtok(NULL, ","));
		v[1]= atoi(strtok(NULL, ","));
		v[2]= atoi(strtok(NULL, ","));
		v[3]= atoi(strtok(NULL, ","));
		t_c.road = v;
		v_cross.push_back(t_c);
	}
	infile_cross.close();
}
void calculate_lineANDrank()//�����·������
{
	for (int nowid = 1, nextid = 0;;)
	{
		for (int i = 0; i < 4; i++)
		{
			int temp_id = v_cross[nowid].road[i];
			if (temp_id == -1) continue;
			road_message temp_oroad = map_road[temp_id];
			if (temp_oroad.start == nowid || temp_oroad.isDuplex == 1)
			{
				nextid = temp_oroad.start + temp_oroad.end - nowid;
				if (nextid == (nowid + 1)) break;
			}
		}
		if (nextid == (nowid + 1))
		{
			nowid = nextid;
			sum_line = nextid;
		}
		else
			break;
	}
	sum_rank = (v_cross.size() - 1) / sum_line;
	for (int i = 1; i < v_cross.size(); i++)
	{
		v_cross[i].x = (i-1)/ sum_line;
		v_cross[i].y = (i-1)% sum_line;
	}

}
void creat_network()
{
	v_neighbour.resize(v_cross.size());
	for (int i = 1; i < v_cross.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (v_cross[i].road[j] != -1)
			{
				road_message temp_road = map_road[v_cross[i].road[j]];
				if (temp_road.start == i)
					v_neighbour[i].push_back(temp_road.end);
				else if (temp_road.isDuplex==1)
					v_neighbour[i].push_back(temp_road.start);
			}
		}
	}
}
void first_traverse_road(road_message &temp_road,car_in_road &temp_car_in_road)
{
	for (int i = (temp_road.length) - 1; i >= 0; i--)
	{
		if (temp_car_in_road.car_number == temp_car_in_road.termination_number) break;
		for (int j = 0; j < temp_road.channel; j++)
		{
			if (temp_car_in_road.lane[i][j] == 0)continue;//�����λ���޳�������ѭ��
			car_message &temp_car = map_car[temp_car_in_road.lane[i][j]];
			if (temp_car.condition == t)continue;//�����λ�ó�Ϊ��ֹ״̬������ѭ��
			int drive_speed = min(temp_car.highspeed, temp_road.limit_speed);
			bool block = false;
			int block_car_id = 0;
			int k = i + 1;
			for (; k <= i + drive_speed&&k < temp_road.length; k++)
			if (temp_car_in_road.lane[k][j] != 0)
			{
				block = true;
				block_car_id = temp_car_in_road.lane[k][j];
				break;
			}
			if (i + drive_speed < temp_road.length&&!block)//���������ʻ�����У������·��,�����ڵ�
			{
				
				
					//���������ʻ�����У�ǰ��û���赲����Ҳ�����·��,�ó������Ϊ��ֹ״̬��
					// ���������ʻ�����У�����ǰ���г����赲�����赲�ĳ���Ϊ��ֹ�������������Ҳ�����Ϊ��ֹ����
					//���������ʻ�����У�����ǰ���г����赲�����赲�ĳ���Ϊ�ȴ��������������Ҳ�����Ϊ�ȴ���ʻ����
					
					temp_car_in_road.lane[i + drive_speed][j] = temp_car_in_road.lane[i][j];
					temp_car_in_road.lane[i][j] = 0;
					temp_car.condition = t;
					temp_car_in_road.termination_number++;
					temp_car_in_road.cannot_move_number++;
					temp_mination_number++;
				
			}
			else if (block&&map_car[block_car_id].condition == t)
			{
				temp_car_in_road.lane[k-1][j] = temp_car_in_road.lane[i][j];
				if(k-1!=i) temp_car_in_road.lane[i][j] = 0;
				temp_car.condition = t;
				temp_car_in_road.termination_number++;
				temp_car_in_road.cannot_move_number++;
				temp_mination_number++;
			}

				
		}
	}
	//int a = 3;
}
void first_dispatch_car_in_road()
{
	if (temp_mination_number == sum_carINroad) return;
	for (unordered_map<int, road_message>::iterator it = map_road.begin(); it != map_road.end(); it++)
	{
		road_message &temp_road = it->second;
		//���������·
		first_traverse_road(temp_road, temp_road.forward_road);
		//���������·
		if (temp_road.isDuplex == 0) continue;
		first_traverse_road(temp_road, temp_road.reverse_road);
	}
}
//һ����һ���ȴ�����ͨ��·�ڶ���Ϊ��ֹ״̬�����Ըó���C�����г�������һ�ε��ȣ���������õ�·�ó��������ڸó�������ʻ���Ϊ��ֹ״̬�ĳ��������ڵ��Ⱥ���Ȼ�ǵȴ�״̬�ĳ��������е��ȣ�����Ȼ���Ϊ�ȴ�״̬��
void deal_same_channel_after_car(bool befor_car, int max_blank_position_id, int i,int j, road_message &dealing_road_message, car_in_road &dealing_single_road, car_message &temp_car)//befor_car=trueǰ��ͨ����false��·��ǰ�ĵط���ֹ
{
	bool after_first_car = true;
	for (int w = i-1; w >= 0; w--)
	{
		if (dealing_single_road.lane[w][j] == 0)continue;
		else
		{
		
			car_message &t_1_car = map_car[dealing_single_road.lane[w][j]];
			if (t_1_car.condition == t) return;
			if (befor_car&&after_first_car&&min(t_1_car.highspeed, dealing_road_message.limit_speed) + w >= dealing_road_message.length) break;
			int next_position = min(min(t_1_car.highspeed, dealing_road_message.limit_speed) + w, max_blank_position_id);
			dealing_single_road.lane[next_position][j] = dealing_single_road.lane[w][j];
			if (next_position!=w)dealing_single_road.lane[w][j] = 0;
			t_1_car.condition = t;
			dealing_single_road.cannot_move_number++;
			t_1_car.next_direction = 0;
			t_1_car.next_road = 0;
			t_1_car.can_not_move = 0;
			dealing_single_road.termination_number++;
			temp_mination_number++;
			max_blank_position_id = next_position - 1;
			after_first_car = false;
		}
	}
	
}
bool test_dead_clock(int i, int now_road_id, int next_road_id, car_in_road  temp_next_single_road, road_message temp_next_road_message, car_message temp_car, bool whether_start_cat)
{
	if (whether_start_cat) return false;
	road_message dealing_road_message = map_road[now_road_id];
	car_message next_rode_first_can_move_car;
	bool find_dead_clock = false;
	int drive_distance_in_now_road = dealing_road_message.length - 1 - i;
	int drive_distance_in_next_road = min(map_road[next_road_id].limit_speed, temp_car.highspeed) - drive_distance_in_now_road;
	if (drive_distance_in_next_road <= 0) return false;
	bool bo_have_wait_car = false;
	for (int h = 0; h <= (temp_next_road_message.length) - 1 && h<drive_distance_in_next_road; h++)
	{
		for (int j = 0; j < temp_next_road_message.channel; j++)
		{
			if (temp_next_single_road.lane[h][j] == 0)continue;//�����λ���޳�������ѭ��
			car_message t_car = map_car[temp_next_single_road.lane[h][j]];
			if (t_car.condition != t)
			{
				bo_have_wait_car = true;
				break;
			}
		}
		if (bo_have_wait_car) break;
	}
	if (!bo_have_wait_car) return false;
	while (1)
	{
		bool bo = false;
		for (int h = (temp_next_road_message.length) - 1; h >= 0; h--)
		{
			
			for (int j = 0; j < temp_next_road_message.channel; j++)
			{
				if (temp_next_single_road.lane[h][j] == 0)continue;//�����λ���޳�������ѭ��
				car_message t_car = map_car[temp_next_single_road.lane[h][j]];
				if (t_car.next_road == 0)continue;//�����λ�ó��޷��򣬼���ѭ��
				else
				{
					next_rode_first_can_move_car = t_car;
					bo = true;
					break;
				}
			}
			if (bo) break;
		}
		if (bo)
		{
			if (next_rode_first_can_move_car.next_road == now_road_id)
			{
				find_dead_clock = true;
				dead_clock_number++;
				return find_dead_clock;
			}
			temp_next_road_message = map_road[next_rode_first_can_move_car.next_road];
			temp_next_single_road = next_rode_first_can_move_car.next_single_road == 1 ? temp_next_road_message.forward_road : temp_next_road_message.reverse_road;
		}
		else
			return find_dead_clock;
	}
	
}
bool test_to_end(int next_cross_id, car_message &temp_car)
{
	if (next_cross_id == temp_car.end) return true;
	if (temp_car.path_cross.empty()) return true;
	vector<int> t_v_cross(v_cross.size(),0);
	for (list<int>::iterator it = temp_car.path_cross.begin(); it != temp_car.path_cross.end(); it++)
		t_v_cross[*it]=1;
	queue<int> que_cross;
	que_cross.push(next_cross_id);
	while (!que_cross.empty())
	{
		int temp_cross = que_cross.front();
		que_cross.pop();
		t_v_cross[temp_cross] = 1;
		for (list<int>::iterator it2 = v_neighbour[temp_cross].begin(); it2 != v_neighbour[temp_cross].end(); it2++)
		{
			if (t_v_cross[*it2] == 1) continue;
			if (*it2 == temp_car.end) return true;
			que_cross.push(*it2);
		}
	}
	return false;
}
car_direct_confirm confirm_car_next_direction(int i,int &cross_id, road_in_cross &dealing_road_idANDindex, car_message &temp_car,bool whether_start_cat)//����ó���һ����ֱ�л�����գ����ҹ�
{
	if (t == 13 && cross_id == 30 )
	{
		int hre = 0;
	}
	if (!temp_car.path_cross.empty()&&temp_car.path_cross.back() == temp_car.end)
	{
		car_direct_confirm t_cdc(2, 0, 0, 0, 0, 0, false, false);
		temp_car.next_direction = t_cdc.next_direction;
		temp_car.next_road = t_cdc.road_id;
		temp_car.next_single_road = t_cdc.single_road;

		return t_cdc;
	}
	if (temp_car.next_direction != 0)
	{
		car_direct_confirm temp;
		temp.next_direction = temp_car.next_direction;
		temp.road_id = temp_car.next_road;
		temp.confirmed = true;
		temp.single_road = temp_car.next_single_road;
		return temp;
	}
	priority_queue<car_direct_confirm> que_car_direct_confirm;
	priority_queue<car_direct_confirm> temp_que_car_direct_confirm;
	int search_road_number=3;
	if (whether_start_cat)//������Ϊ��һ����·ʱ������·������������·
	{
		dealing_road_idANDindex.index = -1;
		search_road_number = 4;
	}
	for (int h = dealing_road_idANDindex.index + 1, j = 1; j <= search_road_number; h++, j++)
	{
		h%= 4;
		bool bo_no_select_next_single_road_channel = false;
		int next_road_id = v_cross[cross_id].road[h];
		int repet_road = 0;
		int next_cross = map_road[next_road_id].start + map_road[next_road_id].end - cross_id;


		road_message  next_road_message = map_road[next_road_id];
		// �����Ҳ���ͨ��
		if (next_road_message.end == cross_id&&next_road_message.isDuplex == 0) continue;
		//�жϸ�·���Ƿ��Ѿ��߹�

		if (!temp_car.path_cross.empty())
		{
			list<int>::iterator it = temp_car.path_cross.end();
			it--;

			while (1)
			{
				if (*it == next_cross || it == temp_car.path_cross.begin())
				{
					if (*it == next_cross) repet_road=1;
					break;
				}
				it--;
			}
		}
		if (next_road_id == -1 || repet_road >0) continue;
		
		
		
		car_in_road  next_single_road;//Ҫ�ߵ���һ�������·����Ϣ
		int single_road = 0;
		if (next_road_message.start == cross_id)
		{
			next_single_road = next_road_message.forward_road;
			single_road = 1;
		}
		else
		{
			next_single_road = next_road_message.reverse_road;
			single_road = -1;
		}

		int next_cross_id = next_road_message.start + next_road_message.end - cross_id;
		//��һ�γ����ĳ�������ӵ��·

		double road_occupy = (next_single_road.car_number + 0.0) / (next_road_message.channel*next_road_message.length);//���㳵��ռ���ʣ�
		if (whether_start_cat&&road_occupy >= 0.5&&que_car.size()>1000) continue;

		int select_next_single_road_channel = -1;////Ҫ�ߵĳ������
		bool have_wait_car = false;
		for (int n = 0; n < next_road_message.channel; n++)
		{
			if (next_single_road.lane[0][n] == 0)
			{
				select_next_single_road_channel = n;
				break;
			}
			else
			{
				if (map_car[next_single_road.lane[0][n]].condition != t)
				{
					have_wait_car = true;
					select_next_single_road_channel = n;
					break;
				}
			}
		}
		int block_cost = 0;

		//��һ�γ����ĳ����ܵȴ�
		if (whether_start_cat&&select_next_single_road_channel == -1) continue;
		//��һ�γ����ĳ�������ƫ·
		bool b_extra = false;
		if (v_cross[next_cross_id].x > max(v_cross[cross_id].x, v_cross[temp_car.end].x) || v_cross[next_cross_id].x < min(v_cross[cross_id].x, v_cross[temp_car.end].x)) b_extra = true;
		else if (v_cross[next_cross_id].y > max(v_cross[cross_id].y, v_cross[temp_car.end].y) || v_cross[next_cross_id].y < min(v_cross[cross_id].y, v_cross[temp_car.end].y)) b_extra = true;
		if (b_extra&&whether_start_cat&&(que_car.size()>500||sum_carINroad>500)) continue;

		//�ж��Ƿ�ᷢ������
		bool find_dead_clock = test_dead_clock(i, dealing_road_idANDindex.id, next_road_id, next_single_road, next_road_message, temp_car, whether_start_cat);
		if (find_dead_clock) continue;
		//�ж��Ƿ��ܵ����յ�

		bool can_to_end = test_to_end(next_cross_id, temp_car);
		if (!can_to_end) continue;

		




		if (select_next_single_road_channel == -1)
		{
			bo_no_select_next_single_road_channel = true;
			select_next_single_road_channel = 0;
		}
		double clear_road_time = 0;//�����������������еĳ���ʻ��õ������ʱ��
		int first_blockcar_IN_select_next_single_road_channel = next_road_message.length;//��¼��һ���谭�����ڵĳ�����index
		int clear_car_number = 0;
		for (int k = 0; k < next_road_message.length; k++)
		{
			int car_id = next_single_road.lane[k][select_next_single_road_channel];
			if (car_id != 0)
			{
				if (first_blockcar_IN_select_next_single_road_channel == next_road_message.length) first_blockcar_IN_select_next_single_road_channel = k;
				double time = (next_road_message.length - k + 0.0) / min(next_road_message.limit_speed, map_car[car_id].highspeed);
				clear_road_time = max(clear_road_time, time);
				clear_car_number++;
			}
		}

		double now_car_pass_time = (next_road_message.length + 0.0) / min(next_road_message.limit_speed, temp_car.highspeed);//��������ȳ�ʻ��õ������ʱ��

		double extra_time = 0;//���õ�·�복��ȥ���ص�ķ������ʱ�Ķ��⻨��
		//��next���ĺ�����������겻�ڸ�·�����յ�ĺ�������֮���ʱ����Ϊ���·��
		int cross_connect_road = 0;
		int cross_connect_road_cost = 0;
		for (int h = 0; h<4; h++)
		{
			if (v_cross[next_cross_id].road[h] != -1) cross_connect_road++;
		}
		//cross_id  
		//temp_car.end
		//next_cross_id
		
		if (b_extra) extra_time = 15 * now_car_pass_time;
		clear_road_time *= 2;
		double road_occupy_value = 60;//ӵ����
		if (cross_connect_road == 2) cross_connect_road_cost = now_car_pass_time * 50;
		if (v_cross[cross_id].x == v_cross[temp_car.end].x)
		{
			if (v_cross[next_cross_id].y > max(v_cross[cross_id].y, v_cross[temp_car.end].y) || v_cross[next_cross_id].y < min(v_cross[cross_id].y, v_cross[temp_car.end].y))
			{
				extra_time *= 4;
			}

		}
		if (v_cross[cross_id].y == v_cross[temp_car.end].y)
		{
			if (v_cross[next_cross_id].x > max(v_cross[cross_id].x, v_cross[temp_car.end].x) || v_cross[next_cross_id].x < min(v_cross[cross_id].x, v_cross[temp_car.end].x))
			{
				extra_time *= 4;
			}

		}
		//���õ�ǰһ�������յ����ڣ�������û��������·�����ʱ����ǰһ�����յ����߷�������һ���ͷ�
		int pre_cross_id;
		if (temp_car.path_cross.size() >= 2)
		{
			list<int>::iterator it = temp_car.path_cross.end();
			it--;
			it--;
			pre_cross_id = *it;
			if (abs(pre_cross_id - temp_car.end) == 1)
			{
				if (v_cross[next_cross_id].y > max(v_cross[pre_cross_id].y, v_cross[temp_car.end].y) || v_cross[next_cross_id].y < min(v_cross[pre_cross_id].y, v_cross[temp_car.end].y))
					extra_time *= 4;
			}
			if (abs(pre_cross_id - temp_car.end) == sum_line)
			{
				if (v_cross[next_cross_id].x > max(v_cross[pre_cross_id].x, v_cross[temp_car.end].x) || v_cross[next_cross_id].x < min(v_cross[pre_cross_id].x, v_cross[temp_car.end].x))
					extra_time *= 4;
			}
		}


		int next_road_cost = now_car_pass_time + clear_road_time + extra_time + road_occupy*road_occupy_value + block_cost + cross_connect_road_cost;
		if (temp_car.end == next_cross_id) next_road_cost = -10;
		car_direct_confirm t_cdc(j, next_road_id, next_road_cost, single_road, select_next_single_road_channel, first_blockcar_IN_select_next_single_road_channel,false,have_wait_car);
		que_car_direct_confirm.push(t_cdc);

	}
	//ȡ���ȶ��еĵ�һ������������С����Ϊ��һ������ʻ��·
	if (que_car_direct_confirm.empty() && temp_que_car_direct_confirm.empty())
	{
		car_direct_confirm t_cdc(0, 0, -1, 0, 0, 0, false,false);
		que_car_direct_confirm.push(t_cdc);
		temp_car.next_direction = que_car_direct_confirm.top().next_direction;
		temp_car.next_road = que_car_direct_confirm.top().road_id;
		temp_car.next_single_road = que_car_direct_confirm.top().single_road;

		return que_car_direct_confirm.top();
	}
	else if (!que_car_direct_confirm.empty())
	{
		temp_car.next_direction = que_car_direct_confirm.top().next_direction;
		temp_car.next_road = que_car_direct_confirm.top().road_id;
		temp_car.next_single_road = que_car_direct_confirm.top().single_road;

		return que_car_direct_confirm.top();
	}
	else
	{
		temp_car.next_direction = temp_que_car_direct_confirm.top().next_direction;
		temp_car.next_road = temp_que_car_direct_confirm.top().road_id;
		temp_car.next_single_road = temp_que_car_direct_confirm.top().single_road;

		return temp_que_car_direct_confirm.top();
	}

}
bool car_advance(int i, int j,car_message &temp_car,road_message &dealing_road_message, car_in_road &dealing_single_road, car_direct_confirm &t_cdcnfirm)
{

	//if (t == 7 & dealing_single_road .car_number==23)
		//dealing_single_road.car_number = 99;
	
	int max_drive_distance;
	int drive_distance_in_now_road = dealing_road_message.length-1 - i;
	int drive_distance_in_next_road = min(map_road[t_cdcnfirm.road_id].limit_speed, temp_car.highspeed) - drive_distance_in_now_road;
	if (drive_distance_in_next_road <= 0 || (!t_cdcnfirm.confirmed&&t_cdcnfirm.first_blockcar_IN_select_next_single_road_channel==0))//4)����ڵ�ǰ��·����ʻ����S1�Ѿ����ڵ�����һ����·�ĵ�λʱ�������ʻ����SV2����˳�������ͨ��·�ڣ�ֻ���н�����ǰ��·����ǰ��λ�ã��ȴ���һʱ��ͨ��·�ڡ�
	{
		
		if (drive_distance_in_next_road > 0)
		{
			if (t_cdcnfirm.have_wait_car)
			{
				temp_car.can_not_move = 1;
				dealing_single_road.cannot_move_number++;
				return false;//���ó�����һ�����������谭�������谭��Ϊ�ȴ�����ó�Ҳ�ȴ�
			}
		}
	
		dealing_single_road.lane[dealing_road_message.length - 1][j] = dealing_single_road.lane[i][j];
		if (dealing_road_message.length - 1 != i)dealing_single_road.lane[i][j] = 0;
		temp_car.condition = t;
		//temp_car.next_direction = 0;
		//temp_car.next_road = 0;
		temp_car.can_not_move = 0;
		dealing_single_road.termination_number++;
		temp_mination_number++;
		dealing_single_road.cannot_move_number++;
		//һ����һ���ȴ�����ͨ��·�ڶ���Ϊ��ֹ״̬�����Ըó���C�����г�������һ�ε��ȣ���������õ�·�ó��������ڸó�������ʻ���Ϊ��ֹ״̬�ĳ��������ڵ��Ⱥ���Ȼ�ǵȴ�״̬�ĳ��������е��ȣ�����Ȼ���Ϊ�ȴ�״̬��
		deal_same_channel_after_car(false, dealing_road_message.length - 2, i, j, dealing_road_message, dealing_single_road, temp_car);
		return true;
	}
	else//������·��
	{
		road_message  &next_road_message = map_road[t_cdcnfirm.road_id];
		car_in_road &next_single_road = temp_car.next_single_road == 1 ? next_road_message.forward_road : next_road_message.reverse_road;
	
		//����֮ǰ��ȷ��,��Ҫ����������Ϣsingle_road, select_next_single_road_channel, first_blockcar_IN_select_next_single_road_channel
		if (t_cdcnfirm.confirmed)//����֮ǰ��ȷ��,��Ҫ����������Ϣselect_next_single_road_channel, first_blockcar_IN_select_next_single_road_channel
		{


			int select_next_single_road_channel = -1;////Ҫ�ߵĳ������
			bool have_wait_car = false;//��һ������ڴ��Ƿ��еȴ�����
			for (int n = 0; n < next_road_message.channel; n++)
			{
				if (next_single_road.lane[0][n] == 0)
				{
					select_next_single_road_channel = n;
					break;
				}
				else
				{
					if (map_car[next_single_road.lane[0][n]].condition != t) 
					{
						select_next_single_road_channel = n;
						have_wait_car = true;
						break;
					}
				}
			}
			
			
			if (select_next_single_road_channel == -1 )//������һ��������·��ȫ������ֹ״̬����ռ�ã����ܳ�·��.������Ϊ�÷�����֮ǰ��������ģ�������Ҫ��ֱ�ж��ȴ����ȸó���һ�ο�ʼ����ʱ����һ���п�λ��
			{//

				dealing_single_road.lane[dealing_road_message.length - 1][j] = dealing_single_road.lane[i][j];
				if (dealing_road_message.length - 1 != i)dealing_single_road.lane[i][j] = 0;
				temp_car.condition = t;
				//temp_car.next_direction = 0;
				//temp_car.next_road = 0;
				temp_car.can_not_move = 0;
				dealing_single_road.termination_number++;
				dealing_single_road.cannot_move_number++;
				temp_mination_number++;
				//һ����һ���ȴ�����ͨ��·�ڶ���Ϊ��ֹ״̬�����Ըó���C�����г�������һ�ε��ȣ���������õ�·�ó��������ڸó�������ʻ���Ϊ��ֹ״̬�ĳ��������ڵ��Ⱥ���Ȼ�ǵȴ�״̬�ĳ��������е��ȣ�����Ȼ���Ϊ�ȴ�״̬��
				deal_same_channel_after_car(false, dealing_road_message.length - 2, i, j, dealing_road_message, dealing_single_road, temp_car);
				return true;
			}
			if (select_next_single_road_channel != -1 && have_wait_car)
			{
				temp_car.can_not_move = 1;
				dealing_single_road.cannot_move_number++;
				return false;//������һ��������·��ȫ��������ռ�ã����谭��Ϊ�ȴ�����ó�Ҳ�ȴ�
			}
			int first_blockcar_IN_select_next_single_road_channel = next_road_message.length;//��¼��һ���谭�����ڵĳ�����index
			for (int k = 0; k < next_road_message.length; k++)
			{
				int car_id = next_single_road.lane[k][select_next_single_road_channel];
				if (car_id != 0)
				{
					first_blockcar_IN_select_next_single_road_channel = k;
					break;
				}
			}
			t_cdcnfirm.select_next_single_road_channel = select_next_single_road_channel;
			t_cdcnfirm.first_blockcar_IN_select_next_single_road_channel = first_blockcar_IN_select_next_single_road_channel;
		}

		if (t_cdcnfirm.first_blockcar_IN_select_next_single_road_channel != next_road_message.length)//������谭��
		{
			int	first_blockcar_id_IN_select_channel = next_single_road.lane[t_cdcnfirm.first_blockcar_IN_select_next_single_road_channel][t_cdcnfirm.select_next_single_road_channel];
			if (drive_distance_in_next_road > t_cdcnfirm.first_blockcar_IN_select_next_single_road_channel&&map_car[first_blockcar_id_IN_select_channel].condition != t)
			{
				temp_car.can_not_move = 1;
				dealing_single_road.cannot_move_number++;
				return false;//���ó�����һ�����������谭�������谭��Ϊ�ȴ�����ó�Ҳ�ȴ�
			}
		}
		
		int index_in_next_road = min(t_cdcnfirm.first_blockcar_IN_select_next_single_road_channel, drive_distance_in_next_road) - 1;
		dealing_single_road.car_number--;
		temp_car.condition = t;
		temp_mination_number++;
		temp_car.next_direction = 0;
		temp_car.next_road = 0;

		temp_car.can_not_move = 0;
		temp_car.path_road.push_back(t_cdcnfirm.road_id);
		int next_cross = map_road[t_cdcnfirm.road_id].start + map_road[t_cdcnfirm.road_id].end - temp_car.path_cross.back();
		temp_car.path_cross.push_back(next_cross);


		next_single_road.lane[index_in_next_road][t_cdcnfirm.select_next_single_road_channel] = dealing_single_road.lane[i][j];
		dealing_single_road.lane[i][j] = 0;
		next_single_road.car_number++;
		next_single_road.termination_number++;
		next_single_road.cannot_move_number++;

		//dealing_single_road.cannot_move_number--;
		//һ����һ���ȴ�����ͨ��·�ڶ���Ϊ��ֹ״̬�����Ըó���C�����г�������һ�ε��ȣ���������õ�·�ó��������ڸó�������ʻ���Ϊ��ֹ״̬�ĳ��������ڵ��Ⱥ���Ȼ�ǵȴ�״̬�ĳ��������е��ȣ�����Ȼ���Ϊ�ȴ�״̬��
		deal_same_channel_after_car(true, dealing_road_message.length - 1, i, j, dealing_road_message, dealing_single_road, temp_car);
		return true;
	}
}
bool have_not_conflict_car(int &cross_id, road_in_cross &dealing_road_idANDindex, road_message &dealing_road_message, car_in_road &dealing_single_road, int &next_road_relative_index, int &judge_direction, car_message &dealing_car_message)
{

	int h = (dealing_road_idANDindex.index + next_road_relative_index) %4;
	int wait_judge_road_id = v_cross[cross_id].road[h];
	if (wait_judge_road_id !=- 1)
	{
		road_in_cross wait_judge_road_idANDindex(h, wait_judge_road_id);
		road_message &wait_judge_road_message = map_road[wait_judge_road_id];
		if (wait_judge_road_message.start == cross_id&&wait_judge_road_message.isDuplex == 0) return true;// �����Ҳ���ͨ�У�������ʱ�뷽���һ����·��û��ֱ��
		car_in_road &wait_judge_single_road = wait_judge_road_message.end == cross_id ? wait_judge_road_message.forward_road : wait_judge_road_message.reverse_road;

		for (int u = (wait_judge_road_message.length) - 1; u >= 0; u--)
		{
			if (wait_judge_single_road.car_number == wait_judge_single_road.termination_number) return true;
			for (int v = 0; v < wait_judge_road_message.channel; v++)
			{
				if (wait_judge_single_road.lane[u][v] != 0)
				{
					int wait_judge_car_id = wait_judge_single_road.lane[u][v];
					car_message &wait_judge_car = map_car[wait_judge_car_id];
					if (wait_judge_car.condition == t) continue;
					car_direct_confirm wait_judge_car_next_direction = confirm_car_next_direction(u,cross_id, wait_judge_road_idANDindex, wait_judge_car,false);//����ó���һ����ֱ�л�����գ����ҹ�
					if (wait_judge_car_next_direction.confirmed == false)
					{
						wait_judge_car.next_direction = wait_judge_car_next_direction.next_direction;
						wait_judge_car.next_road = wait_judge_car_next_direction.road_id;
					}
					if (wait_judge_car.next_direction == judge_direction)
					{
						if (wait_judge_car.can_not_move == 1) dealing_car_message.can_not_move = 1;
						return false;
					}
					else
						return true;
				}
			}
		}
	}
	return true;
}
void dispatch_single_road(int &cross_id, road_in_cross &dealing_road_idANDindex, road_message &dealing_road_message, car_in_road &dealing_single_road, queue<road_in_cross> &que_road_id)
{
	

	car_message &temp_55car = map_car[10587];

	for (int i = (dealing_road_message.length) - 1; i >= 0; i--)
	{
		if (dealing_single_road.car_number == dealing_single_road.termination_number) break;
		for (int j = 0; j < dealing_road_message.channel; j++)
		{
			if (dealing_single_road.lane[i][j] == 0)continue;//�����λ���޳�������ѭ��
			car_message &temp_car = map_car[dealing_single_road.lane[i][j]];
			if (temp_car.condition == t)continue;//�����λ�ó�Ϊ��ֹ״̬������ѭ��
			//if (temp_car.next_direction == 0)
			if (1)
			{
				if ( cross_id == 24 && dealing_single_road.lane[i][j] == 13912)
				{
					int a = 0;
				}
				//�����Ƿ񵽴��յ�
				if (temp_car.end == cross_id)
				{
				
					temp_car.actual_end_time = t;
					//temp_car.path.push_back(dealing_road_idANDindex.id);
					dealing_single_road.lane[i][j] = 0;
					dealing_single_road.car_number--;
					//temp_mination_number++;
					arrived_car++;
					sum_carINroad--;
				    //һ����һ���ȴ�����ͨ��·�ڶ���Ϊ��ֹ״̬�����Ըó���C�����г�������һ�ε��ȣ���������õ�·�ó��������ڸó�������ʻ���Ϊ��ֹ״̬�ĳ��������ڵ��Ⱥ���Ȼ�ǵȴ�״̬�ĳ��������е��ȣ�����Ȼ���Ϊ�ȴ�״̬��
					//deal_same_channel_after_car(true, dealing_road_message.length - 1, i, j, dealing_road_message, dealing_single_road, temp_car);
					deal_same_channel_after_car(true, dealing_road_message.length - 1, i, j, dealing_road_message, dealing_single_road, temp_car);
					continue;
				}
				//����ó���һ����ֱ�л�����գ����ҹ�
				car_direct_confirm t_cdcnfirm = confirm_car_next_direction(i,cross_id, dealing_road_idANDindex, temp_car,false);
				if (temp_car.next_direction == 0)//�޴���ȥ
				{
					temp_mination_number += (dealing_single_road.car_number - dealing_single_road.termination_number);
					dealing_single_road.termination_number = dealing_single_road.car_number;
					dealing_single_road.cannot_move_number = dealing_single_road.car_number;
					for (int x = (dealing_road_message.length) - 1; x >= 0; x--)
					{
						for (int y = 0; y < dealing_road_message.channel; y++)
						{
							if (dealing_single_road.lane[x][y] == 0)continue;//�����λ���޳�������ѭ��
							map_car[dealing_single_road.lane[x][y]].condition = t;
						}
					}
					continue;
				}
				if (temp_car.next_direction == 2)//ֱ��
				{
					
					bool temp_car_condition = car_advance(i, j, temp_car, dealing_road_message, dealing_single_road, t_cdcnfirm);//�жϸó�����̽��ʻ������ֹ���ǵȴ�
					if (temp_car_condition) continue;
					else
						return;

				}
				else if (temp_car.next_direction == 1)//���
				{
					//�ж�����ʱ�뷽���һ����·���Ƿ���ֱ��
					
					int next_road_relative_index = 3;
					int judge_direction = 2;
					bool pass = true;
					pass &= have_not_conflict_car(cross_id, dealing_road_idANDindex, dealing_road_message, dealing_single_road, next_road_relative_index, judge_direction, temp_car);

					if (pass)//�������
					{
						bool temp_car_condition = car_advance(i, j, temp_car, dealing_road_message, dealing_single_road, t_cdcnfirm);//�жϸó�����̽��ʻ������ֹ���ǵȴ�
						if (temp_car_condition) continue;
						else
							return;
					}
					else
						return;

				}
				else//�ҹ�
				{
					//�ж���˳ʱ�뷽���һ����·���Ƿ���ֱ�У���˳ʱ�뷽��ڶ�����·���Ƿ������
					int next_road_relative_index = 1;
					int judge_direction = 2;
					bool pass = true;
					pass &= have_not_conflict_car(cross_id, dealing_road_idANDindex, dealing_road_message, dealing_single_road, next_road_relative_index, judge_direction, temp_car);
					next_road_relative_index = 2;
			    	judge_direction = 1;
				
					pass &= have_not_conflict_car(cross_id, dealing_road_idANDindex, dealing_road_message, dealing_single_road, next_road_relative_index, judge_direction, temp_car);
					if (pass)//�����ҹ�
					{
						
						bool temp_car_condition = car_advance(i, j, temp_car, dealing_road_message, dealing_single_road, t_cdcnfirm);//�жϸó�����̽��ʻ������ֹ���ǵȴ�
						if (temp_car_condition) continue;
						else
							return;
					}
					else
						return;

				}

			}
		
		}
	}
	return;
}
int main()
{
	
	read_txt();//��ȡ��������·��·��
	calculate_lineANDrank();//�����·������
	creat_network();//���������ڽӱ�
	//�ȴ���ÿ����·�ϵĳ���������Щ�������б���ɨ��,����Щ�������Ϊ�ȴ���ʻ��������ֹ����
	//����ϵͳ���Ȱ�·��ID������е��ȸ���·�ڣ�·���ڸ���·����·ID������е���
	//ϵͳ�����ȵ�����·����ʻ�ĳ���������ʻ������·�����г���ȫ����������ʻ���ٵ��ȵȴ���·��ʻ�ĳ���
	//���ȵȴ���·��ʻ�ĳ��������ȴ�����ID������е��ȣ������·������Ȼ������С���Ƚ��н���
	sum_car = que_car.size();
	for (; arrived_car < sum_car; t++)
	{
	

		car_message &deali768ng_car_message = map_car[10768];
		int y = 0;
		if (t == 39)
		{
			int r = 0;
		}
		temp_mination_number = 0;
		first_dispatch_car_in_road();//�ȴ���ÿ����·�ϵĳ���������Щ�������б���ɨ��,����Щ�������Ϊ�ȴ���ʻ��������ֹ����
		while (temp_mination_number < sum_carINroad)// ��������·�ڡ���·�д��ڵȴ�״̬�ĳ���,ÿ��·�ڱ�����·ʱ��ֻ���ȸõ�·��·�ڵķ���
		{
			int next_temp_mination_number = temp_mination_number;
			y++;
			for (int i = 1; i < v_cross.size(); i++)//����ϵͳ���Ȱ�·��ID������е��ȸ���·�ڣ�·���ڸ���·����·ID������е���
			{
				priority_queue<road_in_cross> que_road_sort_id;
				queue<road_in_cross> que_road_id;
				for (int j = 0; j < 4; j++)
					if (v_cross[i].road[j] != -1)
					{
						road_in_cross r(j, v_cross[i].road[j]);
						que_road_sort_id.push(r);
					}
				while (!que_road_sort_id.empty())
				{
					que_road_id.push(que_road_sort_id.top());
					que_road_sort_id.pop();
				}
				while (!que_road_id.empty())//·���ڸ���·����·ID������е���
				{
					road_in_cross dealing_road_idANDindex = que_road_id.front();
					que_road_id.pop();//ÿ��·�ڱ�����·ʱ��ֻ���ȸõ�·��·�ڵķ���
					road_message &dealing_road_message = map_road[dealing_road_idANDindex.id];
					if (dealing_road_message.end == i)
					{
						car_in_road &dealing_single_road = dealing_road_message.forward_road;
						dispatch_single_road(i, dealing_road_idANDindex, dealing_road_message, dealing_single_road, que_road_id);
					}
					else if (dealing_road_message.start == i&&dealing_road_message.isDuplex == 1)
					{
						car_in_road &dealing_single_road = dealing_road_message.reverse_road;
						dispatch_single_road(i, dealing_road_idANDindex, dealing_road_message, dealing_single_road, que_road_id);
					}
						
					else
						continue;


					

				}
			
			}

			int no = 0;
			for (unordered_map<int, road_message>::iterator it = map_road.begin(); it != map_road.end(); it++)
			{
				road_message &temp_road = it->second;
				//���������·
				no += temp_road.forward_road.car_number - temp_road.forward_road.termination_number;
				//���������·
				if (temp_road.isDuplex == 0) continue;
				no += temp_road.reverse_road.car_number - temp_road.reverse_road.termination_number;

			}
			int b=5;
			
			if (next_temp_mination_number == temp_mination_number&&temp_mination_number != sum_carINroad)//�����˵�·������Ӧ����������id���·�ڵ���Ϊ��ֹ״̬,,,�Ѿ����ô�
			{
				//deal_dead_lock(1);
				bo_dead_lock = true;
			}
			if (next_temp_mination_number == temp_mination_number&&temp_mination_number != sum_carINroad)//��Ȼ��·������Ӧ����������id��С·�ڵ���Ϊ��ֹ״̬,,,�Ѿ����ô�
			{
				//deal_dead_lock(0);
				bo_dead_lock = true;
			}
			for (unordered_map<int, road_message>::iterator it = map_road.begin(); it != map_road.end(); it++)
			{
				road_message &temp_road = it->second;
				//���������·
				temp_road.forward_road.cannot_move_number = temp_road.forward_road.termination_number;
				//���������·
				if (temp_road.isDuplex == 0) continue;
				temp_road.reverse_road.cannot_move_number = temp_road.reverse_road.termination_number;
			
			}
			int a = 0;
		}

		//���ȵȴ���·��ʻ�ĳ��������ȴ�����ID������е��ȣ������·������Ȼ������С���Ƚ��н���
		queue<car_sort> que_car_cannot_drive;
		while (sum_carINroad<1500&&!que_car.empty() && que_car.top().time <= t)
		{
			car_sort car_timeANDid = que_car.top();
			que_car.pop();
			car_message &dealing_car_message = map_car[car_timeANDid.id];
			//ȷ����һ��Ҫ�����·��·��
			road_in_cross dealing_road_idANDindex;
			car_direct_confirm car_next_direct_confirm = confirm_car_next_direction(0,dealing_car_message.start, dealing_road_idANDindex, dealing_car_message, true);//����ó���һ����ֱ�л�����գ����ҹ�
			if (car_next_direct_confirm.cost == -1) //����ó����·�޿�λ���޷����ȣ��������ض�����
			{
				que_car_cannot_drive.push(car_timeANDid);
				continue;
			}
			road_message  &next_road_message = map_road[car_next_direct_confirm.road_id];
			car_in_road &next_single_road = dealing_car_message.next_single_road == 1 ? next_road_message.forward_road : next_road_message.reverse_road;
			int drive_distance_in_next_road = min(map_road[car_next_direct_confirm.road_id].limit_speed, dealing_car_message.highspeed);
			int index_in_next_road = min(car_next_direct_confirm.first_blockcar_IN_select_next_single_road_channel, drive_distance_in_next_road) - 1;
			dealing_car_message.condition = t;
			dealing_car_message.actual_start_time = t;
			dealing_car_message.next_direction = 0;
			dealing_car_message.next_road = 0;
			dealing_car_message.can_not_move = 0;
			dealing_car_message.path_cross.push_back(dealing_car_message.start);
			dealing_car_message.path_road.push_back(car_next_direct_confirm.road_id);
			int next_cross = map_road[car_next_direct_confirm.road_id].start + map_road[car_next_direct_confirm.road_id].end - dealing_car_message.start;
			dealing_car_message.path_cross.push_back(next_cross);
			//car_in_road  next_single_road;//Ҫ�ߵ���һ�������·����Ϣ
			sum_carINroad++;
			next_single_road.lane[index_in_next_road][car_next_direct_confirm.select_next_single_road_channel] = car_timeANDid.id;
			next_single_road.car_number++;
			next_single_road.termination_number++;
		}
		while (t<=last_time&&!que_car.empty() && que_car.top().time <= t)
		{

			que_car_cannot_drive.push(que_car.top());
			que_car.pop();
		}
		while (!que_car_cannot_drive.empty())
		{
			car_sort car_timeANDid = que_car_cannot_drive.front();
			que_car_cannot_drive.pop();
			if (t <= last_time) car_timeANDid.time = t + 1;
			que_car.push(car_timeANDid);
	
		}
		for (unordered_map<int, road_message>::iterator it = map_road.begin(); it != map_road.end(); it++)
		{
			road_message &temp_road = it->second;
			//���������·
			temp_road.forward_road.cannot_move_number=0;
			temp_road.forward_road.termination_number = 0;
			//���������·
			if (temp_road.isDuplex == 0) continue;
			temp_road.reverse_road.cannot_move_number = 0;
			temp_road.reverse_road.termination_number = 0;
		}
	}

	ofstream outfile_car("F:\\answer.txt", ios::out|ios::trunc);
	for (int d = 0; d < v_car_id.size(); d++)
	{

		string str;
		car_message temp_car_message = map_car[v_car_id[d]];
		outfile_car << "(" << v_car_id[d] << "," << temp_car_message.actual_start_time;
		for (list<int>::iterator it = temp_car_message.path_road.begin(); it != temp_car_message.path_road.end(); it++)
		{
			outfile_car << "," << *it;
		}
		outfile_car << ")" << endl;
	}

	outfile_car.close();
	return 0;
}