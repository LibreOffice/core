#ifndef __FRAMEWORK_CLASSES_IMAGESCONFIGURATION_HXX_
#define __FRAMEWORK_CLASSES_IMAGESCONFIGURATION_HXX_

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

namespace framework
{

struct ImageItemDescriptor
{
    ImageItemDescriptor() : nIndex( -1 ) {}

    String  aCommandURL;                // URL command to dispatch
    long    nIndex;                     // index of the bitmap inside the bitmaplist
};

struct ExternalImageItemDescriptor
{
    String  aCommandURL;                // URL command to dispatch
    String  aURL;                       // a URL to an external bitmap
};

typedef ImageItemDescriptor* ImageItemDescriptorPtr;
SV_DECL_PTRARR_DEL( ImageItemListDescriptor, ImageItemDescriptorPtr, 10, 2)

typedef ExternalImageItemDescriptor* ExternalImageItemDescriptorPtr;
SV_DECL_PTRARR_DEL( ExternalImageItemListDescriptor, ExternalImageItemDescriptorPtr, 10, 2)

struct ImageListItemDescriptor
{
    ImageListItemDescriptor() : pImageItemList( 0 ) {}
    ~ImageListItemDescriptor() { delete pImageItemList; }

    String                      aURL;           // a URL to a bitmap with several images inside
    Color                       aMaskColor;     // a color used as transparent
    ImageItemListDescriptor*    pImageItemList; // an array of ImageItemDescriptors that describes every image
};

typedef ImageListItemDescriptor* ImageListItemDescriptorPtr;
SV_DECL_PTRARR_DEL( ImageListDescriptor, ImageListItemDescriptorPtr, 10, 2)

struct ImageListsDescriptor
{
    ImageListsDescriptor() : pImageList( 0 ),
                             pExternalImageList( 0 ) {}
    ~ImageListsDescriptor() { delete pImageList; delete pExternalImageList; }

    ImageListDescriptor*                pImageList;
    ExternalImageItemListDescriptor*    pExternalImageList;
};

class ImagesConfiguration
{
    public:
        static sal_Bool LoadImages( SvStream& rInStream, ImageListsDescriptor& aItems );
        static sal_Bool StoreImages( SvStream& rOutStream, const ImageListsDescriptor& aItems );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_IMAGES
