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
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif


namespace framework
{

struct ToolBoxItemDescriptor
{
    Bitmap* pBmp;                       // Bitmap ptr not used by xml configuration
    String  aBitmapName;                // bitmap name => use to find correct bmp file
    String  aItemText;                  // label for this toolbox item
    String  aURL;                       // URL command to dispatch
    USHORT  nId;                        // internal id not used by xml configuration
    USHORT  nItemBits;                  // properties for this toolbox item (WinBits)
    USHORT  nItemType;                  // toolbox item type (BUTTON, SPACE, BREAK, SEPARATOR)
    USHORT  nVisible;                   // toolbox item visible?
    USHORT  nWidth;                     // width of a toolbox window (edit field, etc.)
    USHORT  nUserDef;                   // user defined toolbox item (1=yes/0=no)

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

struct ToolBoxLayoutItemDescriptor
{
    String                      aName;          // Name der Toolbox ( Objectbar, Toolbar etc. )
    Point                       aFloatingPos;   // Position im nicht angedockten Zustand
    USHORT                      nFloatingLines; // Anzahl der Zeilen im nicht angedockten Zustand
    USHORT                      nLines;         // Anzahl der Zeilen im angedockten Zustand
    ToolBoxAlign                eAlign;         // Alignment im angedockten Zustand
    BOOL                        bVisible;       // ein - oder ausgeschaltet
    BOOL                        bFloating;      // angedockt oder nicht

    ToolBoxLayoutItemDescriptor() : nFloatingLines( 0 )
                                    ,nLines( 1 )
                                    ,eAlign( BOXALIGN_LEFT )
                                    ,bVisible( sal_False )
                                    ,bFloating( sal_False ) {}
};

typedef ToolBoxLayoutItemDescriptor* ToolBoxLayoutItemDescriptorPtr;
SV_DECL_PTRARR_DEL( ToolBoxLayoutDescriptor, ToolBoxLayoutItemDescriptorPtr, 10, 2)

class ToolBoxConfiguration
{
    public:
        static sal_Bool LoadToolBox( SvStream& rInStream, ToolBoxDescriptor& aItems );
        static sal_Bool StoreToolBox( SvStream& rOutStream, const ToolBoxDescriptor& aItems );
        static sal_Bool LoadToolBoxLayout( SvStream& rInStream, ToolBoxLayoutDescriptor& aItems );
        static sal_Bool StoreToolBoxLayout( SvStream& rOutStream, ToolBoxLayoutDescriptor& aItems );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_TOOLBOXCONFIGURATION_HXX_
