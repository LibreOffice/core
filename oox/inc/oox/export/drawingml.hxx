#ifndef _OOX_EXPORT_DRAWINGML_HXX_
#define _OOX_EXPORT_DRAWINGML_HXX_

#include <oox/dllapi.h>
#include <sax/fshelper.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/uno/XReference.hpp>
#include <tools/poly.hxx>
#include <filter/msfilter/escherex.hxx>

class Graphic;
class String;

namespace com { namespace sun { namespace star {
namespace beans {
    class XPropertySet;
    class XPropertyState;
}
namespace drawing {
    class XShape;
}
namespace style {
    struct LineSpacing;
}
namespace text {
    class XTextContent;
    class XTextRange;
}
}}}

namespace oox {
namespace core {
    class XmlFilterBase;
}

namespace drawingml {

class OOX_DLLPUBLIC DrawingML {
public:
    enum DocumentType { DOCUMENT_DOCX, DOCUMENT_PPTX, DOCUMENT_XLSX };

private:
    static int mnImageCounter;

    /// To specify where write eg. the images to (like 'ppt', or 'word' - according to the OPC).
    DocumentType meDocumentType;

protected:
    ::com::sun::star::uno::Any mAny;
    ::sax_fastparser::FSHelperPtr mpFS;
    ::oox::core::XmlFilterBase* mpFB;

    bool GetProperty( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, String aName );
    bool GetPropertyAndState( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet,
                  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState > rXPropState,
                  String aName, ::com::sun::star::beans::PropertyState& eState );
    const char* GetFieldType( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > rRun, sal_Bool& bIsField );

    rtl::OUString WriteImage( const rtl::OUString& rURL );

public:
    DrawingML( ::sax_fastparser::FSHelperPtr pFS, ::oox::core::XmlFilterBase* pFB = NULL, DocumentType eDocumentType = DOCUMENT_PPTX ) : meDocumentType( eDocumentType ), mpFS( pFS ), mpFB( pFB ) {}
    void SetFS( ::sax_fastparser::FSHelperPtr pFS ) { mpFS = pFS; }
    ::sax_fastparser::FSHelperPtr GetFS() { return mpFS; }
    ::oox::core::XmlFilterBase* GetFB() { return mpFB; }

    rtl::OUString WriteImage( const Graphic &rGraphic );

    void WriteColor( sal_uInt32 nColor );
    void WriteGradientStop( sal_uInt16 nStop, sal_uInt32 nColor );
    void WriteLineArrow( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, sal_Bool bLineStart );
    void WriteConnectorConnections( EscherConnectorListEntry& rConnectorEntry, sal_Int32 nStartID, sal_Int32 nEndID );

    void WriteSolidFill( sal_uInt32 nColor );
    void WriteSolidFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet );
    void WriteGradientFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet );
    void WriteBlipFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, String sURLPropName, sal_Int32 nXmlNamespace );
    void WriteBlipFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, String sURLPropName );
    void WriteOutline( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet );
    void WriteStretch();
    void WriteLinespacing( ::com::sun::star::style::LineSpacing& rLineSpacing );

    ::rtl::OUString WriteBlip( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet, ::rtl::OUString& rURL );
    void WriteBlipMode( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet );

    void WriteShapeTransformation( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > rXShape,
                   sal_Bool bFlipH = false, sal_Bool bFlipV = false, sal_Int32 nRotation = 0 );
    void WriteTransformation( const Rectangle& rRectangle,
                  sal_Bool bFlipH = false, sal_Bool bFlipV = false, sal_Int32 nRotation = 0 );

    void WriteText( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > rXShape );
    void WriteParagraph( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > rParagraph );
    void WriteParagraphProperties( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > rParagraph );
    void WriteParagraphNumbering( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet,
                                  sal_Int16 nLevel );
    void WriteRun( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > rRun );
    void WriteRunProperties( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > rRun, sal_Bool bIsField );

    void WritePresetShape( const char* pShape );
    void WritePresetShape( const char* pShape, MSO_SPT eShapeType, sal_Bool bPredefinedHandlesUsed, sal_Int32 nAdjustmentsWhichNeedsToBeConverted, const ::com::sun::star::beans::PropertyValue& rProp );
    void WritePolyPolygon( const PolyPolygon& rPolyPolygon );

    static void ResetCounters();

    void GetUUID( ::rtl::OStringBuffer& rBuffer );

    static sal_Unicode SubstituteBullet( sal_Unicode cBulletId, ::com::sun::star::awt::FontDescriptor& rFontDesc );

    sal_uInt32 ColorWithIntensity( sal_uInt32 nColor, sal_uInt32 nIntensity );

    static const char* GetAlignment( sal_Int32 nAlignment );
};

}
}

#endif
