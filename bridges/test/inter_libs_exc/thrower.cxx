
#include "share.h"


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static void SAL_CALL throws_exc()
{
    throw lang::IllegalArgumentException(
        OUSTR("bla"), Reference< XInterface >(), 0 );
}

extern "C" t_throws_exc SAL_CALL get_thrower()
{
    return throws_exc;
}
