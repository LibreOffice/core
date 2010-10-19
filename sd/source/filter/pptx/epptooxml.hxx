#ifndef EPPT_POWERPOINT_EXPORT_HXX
#define EPPT_POWERPOINT_EXPORT_HXX

#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/zipstorage.hxx>

#include "epptbase.hxx"

namespace oox {
    namespace drawingml {
        class ShapeExport;
    }
namespace core {

class PowerPointShapeExport;

struct LayoutInfo
{
    std::vector< sal_Int32 > mnFileIdArray;
};

enum PlaceholderType
{
    None,
    SlideImage,
    Notes,
    Header,
    Footer,
    SlideNumber,
    DateAndTime,
    Outliner,
    Title,
    Subtitle
};

class PowerPointExport : public XmlFilterBase, public PPTWriterBase
{
    friend class PowerPointShapeExport;
public:

    PowerPointExport( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr  );
    ~PowerPointExport();

    // from FilterBase
    virtual bool importDocument() throw();
    virtual bool exportDocument() throw();

    // only needed for import, leave them empty, refactor later XmlFilterBase to export and import base?
    virtual sal_Int32 getSchemeClr( sal_Int32 /* nColorSchemeToken */ ) const { return 0; }
    virtual const oox::vml::DrawingPtr getDrawings() { return oox::vml::DrawingPtr(); }
    virtual const oox::drawingml::Theme* getCurrentTheme() const { return NULL; }
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles() { return oox::drawingml::table::TableStyleListPtr(); }
    virtual oox::drawingml::chart::ChartConverter& getChartConverter();

    static const char* GetSideDirection( sal_uInt8 nDirection );
    static const char* GetCornerDirection( sal_uInt8 nDirection );
    static const char* Get8Direction( sal_uInt8 nDirection );

protected:

    virtual void ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 nMode,
                                 sal_Bool bHasBackground, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );
    virtual void ImplWriteNotes( sal_uInt32 nPageNum );
    virtual void ImplWriteSlideMaster( sal_uInt32 nPageNum, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );
    virtual void ImplWriteLayout( sal_Int32 nOffset, sal_uInt32 nMasterNum );
    void WriteTheme( sal_Int32 nThemeNum );

    virtual sal_Bool ImplCreateDocument();
    virtual sal_Bool ImplCreateMainNotes();

    sal_Bool WriteNotesMaster();

    void WriteTextStyles( ::sax_fastparser::FSHelperPtr pFS );
    void WriteTextStyle( ::sax_fastparser::FSHelperPtr pFS, int nInstance, sal_Int32 xmlToken );
    void WriteTextStyleLevel( ::sax_fastparser::FSHelperPtr pFS, int nInstance, int nLevel );
    void ImplWriteBackground( ::sax_fastparser::FSHelperPtr pFS, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );

    sal_Int32 GetLayoutFileId( sal_Int32 nOffset, sal_uInt32 nMasterNum );

    // shapes
    void WriteShapeTree( ::sax_fastparser::FSHelperPtr pFS, PageType ePageType, sal_Bool bMaster );

    void WritePlaceholderShape( ::sax_fastparser::FSHelperPtr pFS, drawingml::ShapeExport& rDML, PlaceholderType ePlaceholder );
    void WritePageShape( ::sax_fastparser::FSHelperPtr pFS, drawingml::ShapeExport& rDML, PageType ePageType );

    // helper parts
    sal_Bool WritePlaceholder( ::sax_fastparser::FSHelperPtr pFS, drawingml::ShapeExport& rDML, PlaceholderType ePlaceholder, sal_Bool bMaster );

    sal_uInt32 GetNewSlideId() { return mnSlideIdMax ++; }
    sal_uInt32 GetNewSlideMasterId() { return mnSlideMasterIdMax ++; }

private:
    void AddLayoutIdAndRelation( ::sax_fastparser::FSHelperPtr pFS, sal_Int32 nLayoutFileId );

    virtual ::rtl::OUString implGetImplementationName() const;

    ::boost::shared_ptr< ::oox::drawingml::chart::ChartConverter > mxChartConv;

    ::sax_fastparser::FSHelperPtr mPresentationFS;

    LayoutInfo mLayoutInfo[EPP_LAYOUT_SIZE];
    std::vector< ::sax_fastparser::FSHelperPtr > mpSlidesFSArray;
    std::vector< ::sax_fastparser::FSHelperPtr > mpMasterFSArray;
    sal_Int32 mnLayoutFileIdMax;

    sal_uInt32 mnSlideIdMax;
    sal_uInt32 mnSlideMasterIdMax;

    sal_Int32 mnShapeIdMax;
    sal_Int32 mnPictureIdMax;

    sal_Bool mbCreateNotes;

    static sal_Int32 nStyleLevelToken[5];
};

}
}

#endif
