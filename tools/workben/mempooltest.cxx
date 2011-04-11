#include "tools/mempool.hxx"

struct MempoolTest
{
  int m_int;

  DECL_FIXEDMEMPOOL_NEWDEL(MempoolTest);
};

IMPL_FIXEDMEMPOOL_NEWDEL(MempoolTest, 0, 0);

int main()
{
  MempoolTest * p = new MempoolTest();
  if (p != 0)
    delete p;
  return 1;
}
