#ifndef __FRAMEWORK_CLASSES_TOOLBOXCONFIGURATION_HXX_
#define __FRAMEWORK_CLASSES_TOOLBOXCONFIGURATION_HXX_

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif


namespace framework
{

struct ToolBoxItemDescriptor
{
    Bitmap* pBmp;
    String  aBitmapName;
    String  aItemText;
    String  aURL;
    USHORT  nId;
    USHORT  nItemBits;
    USHORT  nItemType;
    USHORT  nVisible;
    USHORT  nWidth;
    USHORT  nUserDef;

    public:

        ToolBoxItemDescriptor() : pBmp( 0 )
                                 ,nId( 0 )
                                 ,nItemBits( 0 )
                                 ,nItemType( (USHORT)TOOLBOXITEM_SPACE )
                                 ,nVisible( sal_True )
                                 ,nWidth( 0 )
                                 ,nUserDef( sal_False ) {}
};

typedef ToolBoxItemDescriptor* ToolBoxItemDescriptorPtr;
SV_DECL_PTRARR_DEL( ToolBoxDescriptor, ToolBoxItemDescriptorPtr, 10, 2)

class ToolBoxConfiguration
{
    public:
        static sal_Bool LoadToolBox( SvStream& rInStream, ToolBoxDescriptor& aItems );
        static sal_Bool StoreToolBox( SvStream& rOutStream, const ToolBoxDescriptor& aItems );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_TOOLBOXCONFIGURATION_HXX_
