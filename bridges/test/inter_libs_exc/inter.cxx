
#include <stdio.h>
#include "share.h"

#include <rtl/string.hxx>
#include <osl/module.hxx>


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

extern "C" int main( int argc, char const * argv [] )
{
#ifdef SAL_W32
#define SAL_DLLPREFIX ""
#endif
    Module mod_starter(
        OUSTR(SAL_DLLPREFIX"starter"SAL_DLLEXTENSION),
        SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    Module mod_thrower(
        OUSTR(SAL_DLLPREFIX"thrower"SAL_DLLEXTENSION),
        SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );

    typedef t_throws_exc (SAL_CALL * t_get_thrower)();
    t_get_thrower get_thrower = (t_get_thrower)mod_thrower.getSymbol( OUSTR("get_thrower") );
    t_throws_exc thrower = (*get_thrower)();

    typedef void (SAL_CALL * t_starter)( t_throws_exc );
    t_starter start = (t_starter)mod_starter.getSymbol( OUSTR("start") );

    (*start)( thrower );

    return 0;
}
