#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#define _UNTOOLS_UCBSTREAMHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#include <tools/stream.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace task
            {
                class XInteractionHandler;
            }
        }
    }
};

#define NS_UNO ::com::sun::star::uno
#define NS_IO ::com::sun::star::io
#define NS_TASK ::com::sun::star::task

class String;
namespace utl
{
    class UcbLockBytesHandler;

    class UcbStreamHelper : public SvStream
    {
    public:
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                NS_UNO::Reference < NS_TASK::XInteractionHandler >,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XInputStream > xStream );
    };
};

#endif
