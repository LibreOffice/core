#include <rtl/ustring.h>
#include "Media.hxx"
#include "SymbolLoader.hxx"
#include "Instance.hxx"

struct libvlc_instance_t;
namespace VLC
{
    namespace
    {
        libvlc_media_t* ( *libvlc_media_new_path ) ( libvlc_instance_t *p_instance, const char *path );
        void ( *libvlc_media_release )( libvlc_media_t *p_md );

        ApiMap VLC_MEDIA_API[] =
        {
            SYM_MAP( libvlc_media_new_path ),
            SYM_MAP( libvlc_media_release )
        };

        libvlc_media_t* InitMedia( const rtl::OUString& url, VLC::Instance& instance )
        {
            rtl::OString dest;
            url.convertToString(&dest, RTL_TEXTENCODING_UTF8, 0);

            return libvlc_media_new_path(instance, dest.getStr());
        }

    }


Media::Media( const rtl::OUString& url, Instance& instance )
{
    InitApiMap(VLC_MEDIA_API);
    mMedia = InitMedia( url, instance );
}

Media::~Media()
{
    libvlc_media_release( mMedia );
}

}
