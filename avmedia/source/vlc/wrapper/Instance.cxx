/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "Instance.hxx"
#include "SymbolLoader.hxx"

namespace VLC
{
    namespace
    {
        libvlc_instance_t* ( *libvlc_new ) ( int argc, const char * const *argv );
        void ( *libvlc_release ) ( libvlc_instance_t *p_instance );
        void ( *libvlc_retain ) ( libvlc_instance_t *p_instance );

        ApiMap VLC_INSTANCE_API[] =
        {
            SYM_MAP( libvlc_new ),
            SYM_MAP( libvlc_release ),
            SYM_MAP( libvlc_retain )
        };
    }

    Instance::Instance( const char * const argv[] )
    {
        InitApiMap( VLC_INSTANCE_API );

        mInstance = libvlc_new( sizeof( argv ) / sizeof( argv[0] ), argv );
    }

    Instance::Instance( const Instance& other )
    {
    }

    const Instance& Instance::operator=( const Instance& other )
    {
        libvlc_release( mInstance );
        mInstance = other.mInstance;
        libvlc_retain( mInstance );
    }

    Instance::~Instance()
    {
        libvlc_release( mInstance );
    }

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
