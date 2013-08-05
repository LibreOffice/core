#include <rtl/ustring.hxx>

#include "Instance.hxx"
#include "SymbolLoader.hxx"

namespace VLC
{
    namespace
    {
        libvlc_instance_t *(*libvlc_new) (int argc, const char * const *argv);
        void (*libvlc_release) (libvlc_instance_t *p_instance);

        ApiMap VLC_INSTANCE_API[] =
        {
            SYM_MAP( libvlc_new ),
            SYM_MAP( libvlc_release )
        };
    }

    Instance::Instance( int argc, const char * const *argv )
    {
        InitApiMap( VLC_INSTANCE_API );

        mInstance = libvlc_new( argc, argv );
    }

    Instance::~Instance()
    {
        libvlc_release( mInstance );
    }

}
