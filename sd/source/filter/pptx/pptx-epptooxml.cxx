#include <hash_map>
#include <stdio.h>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <oox/core/tokens.hxx>
#include <epptooxml.hxx>
#include <epptdef.hxx>
#include <oox/export/shapes.hxx>

#include <cppuhelper/factory.hxx>
#include <sax/fshelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <filter/msfilter/escherex.hxx>
#include <tools/poly.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>

#include <oox/export/utils.hxx>

// presentation namespaces
#define PNMSS         FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main", \
                      FSNS( XML_xmlns, XML_p ), "http://schemas.openxmlformats.org/presentationml/2006/main", \
                      FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships"

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::task::XStatusIndicator;
using ::com::sun::star::text::XSimpleText;
using ::sax_fastparser::FastSerializerHelper;
using ::sax_fastparser::FSHelperPtr;

void dump_pset(Reference< XPropertySet > rXPropSet);

#define IDS(x) (OString(#x " ") + OString::valueOf( mnShapeIdMax++ )).getStr()

namespace oox {
    using namespace drawingml;
    namespace core {

class PowerPointShapeExport : public ShapeExport
{
    PowerPointExport&   mrExport;
    PageType            mePageType;
    sal_Bool            mbMaster;
public:
                        PowerPointShapeExport( FSHelperPtr pFS, PowerPointExport* pFB );
    void                SetMaster( sal_Bool bMaster );
    void                SetPageType( PageType ePageType );
    ShapeExport&        WriteNonVisualProperties( Reference< XShape > xShape );
    ShapeExport&        WriteTextShape( Reference< XShape > xShape );
    ShapeExport&        WriteUnknownShape( Reference< XShape > xShape );
};

PowerPointShapeExport::PowerPointShapeExport( FSHelperPtr pFS, PowerPointExport* pFB )
    : ShapeExport( XML_p, pFS, pFB )
    , mrExport( *pFB )
{
}

void PowerPointShapeExport::SetMaster( sal_Bool bMaster )
{
    mbMaster = bMaster;
}

void PowerPointShapeExport::SetPageType( PageType ePageType )
{
    mePageType = ePageType;
}

ShapeExport& PowerPointShapeExport::WriteNonVisualProperties( Reference< XShape > )
{
    GetFS()->singleElementNS( XML_p, XML_nvPr, FSEND );

    return *this;
}

ShapeExport& PowerPointShapeExport::WriteTextShape( Reference< XShape > xShape )
{
    OUString sShapeType = xShape->getShapeType();

    DBG(printf( "shape(text): %s\n", USS(sShapeType) ));

    if( sShapeType.equalsAscii( "com.sun.star.drawing.TextShape" ) )
    {
    ShapeExport::WriteTextShape( xShape );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.presentation.DateTimeShape" ) )
    {
        if( !mrExport.WritePlaceholder( GetFS(), *this, DateAndTime, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.presentation.FooterShape" ) )
    {
        if( !mrExport.WritePlaceholder( GetFS(), *this, Footer, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.presentation.HeaderShape" ) )
    {
        if( !mrExport.WritePlaceholder( GetFS(), *this, Header, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.presentation.NotesShape" ) )
    {
        if( mePageType == NOTICE && mrExport.GetPresObj() )
            mrExport.WritePlaceholderShape( GetFS(), *this, Notes );
        else
            ShapeExport::WriteTextShape( xShape );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.presentation.OutlinerShape" ) )
    {
        if( !mrExport.WritePlaceholder( GetFS(), *this, Outliner, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.presentation.SlideNumberShape" ) )
    {
        if( !mrExport.WritePlaceholder( GetFS(), *this, SlideNumber, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.presentation.TitleTextShape" ) )
    {
        if( !mrExport.WritePlaceholder( GetFS(), *this, Title, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }

    return *this;
}

ShapeExport& PowerPointShapeExport::WriteUnknownShape( Reference< XShape > xShape )
{
    OUString sShapeType = xShape->getShapeType();

    DBG(printf( "shape(unknown): %s\n", USS(sShapeType) ));

    if( sShapeType.equalsAscii( "com.sun.star.drawing.Group" ) )
    {
        Reference< XIndexAccess > rXIndexAccess( xShape, UNO_QUERY );

        mrExport.EnterGroup( rXIndexAccess );
        DBG(printf( "enter group\n" ));
    }
    else if( sShapeType.equalsAscii( "com.sun.star.drawing.Group" ) )
    {
        mrExport.WritePageShape( GetFS(), *this, mePageType );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.drawing.PageShape" ) )
    {
        mrExport.WritePageShape( GetFS(), *this, mePageType );
    }
    else if( sShapeType.equalsAscii( "com.sun.star.presentation.SubtitleShape" ) )
    {
        if( !mrExport.WritePlaceholder( GetFS(), *this, Subtitle, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }

    return *this;
}

PowerPointExport::PowerPointExport( const Reference< XMultiServiceFactory > & rSMgr  )
    : XmlFilterBase( rSMgr ),
      PPTWriterBase(),
      mxChartConv( new ::oox::drawingml::chart::ChartConverter ),
      mnLayoutFileIdMax( 1 ),
      mnSlideIdMax( 1 << 8 ),
      mnSlideMasterIdMax( 1 << 31 ),
      mnShapeIdMax( 1 ),
      mnPictureIdMax( 1 )
{
    memset( mLayoutInfo, 0, sizeof(mLayoutInfo) );
}

PowerPointExport::~PowerPointExport()
{
}

bool PowerPointExport::importDocument() throw()
{
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool PowerPointExport::exportDocument() throw()
{
    DrawingML::ResetCounters();

    addRelation( US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" ), S( "ppt/presentation.xml" ) );

    mPresentationFS = openOutputStreamWithSerializer( US( "ppt/presentation.xml" ),
                                                      US( "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml" ) );

    addRelation( mPresentationFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" ),
                 US( "theme/theme1.xml" ) );

    mPresentationFS->startElementNS( XML_p, XML_presentation, PNMSS, FSEND );

    mXModel.set( getModel(), UNO_QUERY );
    mXStatusIndicator.set( getStatusIndicator(), UNO_QUERY );

    exportPPT();

    mPresentationFS->singleElementNS( XML_p, XML_sldSz,
                                      XML_cx, IS( PPTtoEMU( maDestPageSize.Width ) ),
                                      XML_cy, IS( PPTtoEMU( maDestPageSize.Height ) ),
                                      FSEND );
    // for some reason if added before slides list it will not load the slides (alas with error reports) in mso
    mPresentationFS->singleElementNS( XML_p, XML_notesSz,
                                      XML_cx, IS( PPTtoEMU( maNotesPageSize.Width ) ),
                                      XML_cy, IS( PPTtoEMU( maNotesPageSize.Height ) ),
                                      FSEND );

    mPresentationFS->endElementNS( XML_p, XML_presentation );
    mPresentationFS.reset();

    commit();

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

void PowerPointExport::ImplWriteBackground( FSHelperPtr pFS, Reference< XPropertySet > rXPropSet )
{
    FillStyle aFillStyle( FillStyle_NONE );
    if ( ImplGetPropertyValue( rXPropSet, S( "FillStyle" ) ) )
        mAny >>= aFillStyle;

    if( aFillStyle == FillStyle_BITMAP ) {
        //DBG(printf ("FillStyle_BITMAP properties\n"));
        //DBG(dump_pset(rXPropSet));
    }

    if( aFillStyle == FillStyle_NONE ||
        aFillStyle == FillStyle_GRADIENT ||
        aFillStyle == FillStyle_HATCH )
        return;

    pFS->startElementNS( XML_p, XML_bg, FSEND );
    pFS->startElementNS( XML_p, XML_bgPr, FSEND );

    PowerPointShapeExport( pFS, this ).WriteFill( rXPropSet );

    pFS->endElementNS( XML_p, XML_bgPr );
    pFS->endElementNS( XML_p, XML_bg );
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define MINIMAL_SPTREE "<p:spTree>\
      <p:nvGrpSpPr>\
        <p:cNvPr id=\"1\" name=\"\"/>\
        <p:cNvGrpSpPr/>\
        <p:nvPr/>\
      </p:nvGrpSpPr>\
      <p:grpSpPr>\
        <a:xfrm>\
          <a:off x=\"0\" y=\"0\"/>\
          <a:ext cx=\"0\" cy=\"0\"/>\
          <a:chOff x=\"0\" y=\"0\"/>\
          <a:chExt cx=\"0\" cy=\"0\"/>\
        </a:xfrm>\
      </p:grpSpPr>\
    </p:spTree>"

#define MAIN_GROUP \
     "<p:nvGrpSpPr>\
        <p:cNvPr id=\"1\" name=\"\"/>\
        <p:cNvGrpSpPr/>\
        <p:nvPr/>\
      </p:nvGrpSpPr>\
      <p:grpSpPr>\
        <a:xfrm>\
          <a:off x=\"0\" y=\"0\"/>\
          <a:ext cx=\"0\" cy=\"0\"/>\
          <a:chOff x=\"0\" y=\"0\"/>\
          <a:chExt cx=\"0\" cy=\"0\"/>\
        </a:xfrm>\
      </p:grpSpPr>"


#define GETA(propName) \
    ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( #propName ) ) )

#define GET(variable, propName) \
    if ( GETA(propName) ) \
        mAny >>= variable;

const char* PowerPointExport::GetSideDirection( sal_uInt8 nDirection )
{
    const char* pDirection = NULL;

    switch( nDirection ) {
    case 0:
        pDirection = "r";
        break;
    case 1:
        pDirection = "d";
        break;
    case 2:
        pDirection = "l";
        break;
    case 3:
        pDirection = "u";
        break;
    }

    return pDirection;
}

const char* PowerPointExport::GetCornerDirection( sal_uInt8 nDirection )
{
    const char* pDirection = NULL;

    switch( nDirection ) {
    case 4:
        pDirection = "rd";
        break;
    case 5:
        pDirection = "ld";
        break;
    case 6:
        pDirection = "ru";
        break;
    case 7:
        pDirection = "lu";
        break;
    }

    return pDirection;
}

const char* PowerPointExport::Get8Direction( sal_uInt8 nDirection )
{
    const char* pDirection = GetSideDirection( nDirection );

    if( !pDirection )
    pDirection = GetCornerDirection( nDirection );

    return pDirection;
}

void PowerPointExport::ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 /* nMode */,
                                       sal_Bool bHasBackground, Reference< XPropertySet > aXBackgroundPropSet )
{
    DBG(printf("write slide: %d\n----------------\n", nPageNum));

    // slides list
    if( nPageNum == 0 )
        mPresentationFS->startElementNS( XML_p, XML_sldIdLst, FSEND );

    // add explicit relation of presentation to this slide
    OUString sRelId = addRelation( mPresentationFS->getOutputStream(),
                                   US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" ),
                                   OUStringBuffer()
                                   .appendAscii( "slides/slide" )
                                   .append( (sal_Int32) nPageNum + 1 )
                                   .appendAscii( ".xml" )
                                   .makeStringAndClear() );

    mPresentationFS->singleElementNS( XML_p, XML_sldId,
                                      XML_id, I32S( GetNewSlideId() ),
                                      FSNS( XML_r, XML_id ), USS( sRelId ),
                                      FSEND );

    if( nPageNum == mnPages - 1 )
        mPresentationFS->endElementNS( XML_p, XML_sldIdLst );

    FSHelperPtr pFS = openOutputStreamWithSerializer( OUStringBuffer()
                                                      .appendAscii( "ppt/slides/slide" )
                                                      .append( (sal_Int32) nPageNum + 1 )
                                                      .appendAscii( ".xml" )
                                                      .makeStringAndClear(),
                                                      US( "application/vnd.openxmlformats-officedocument.presentationml.slide+xml" ) );

    if( mpSlidesFSArray.size() < mnPages )
    mpSlidesFSArray.resize( mnPages );
    mpSlidesFSArray[ nPageNum ] = pFS;

    pFS->startElementNS( XML_p, XML_sld, PNMSS, FSEND );

    pFS->startElementNS( XML_p, XML_cSld, FSEND );

    // background
    if( bHasBackground ) {
        ImplWriteBackground( pFS, aXBackgroundPropSet );
    }

    WriteShapeTree( pFS, NORMAL, FALSE );

    pFS->endElementNS( XML_p, XML_cSld );

    FadeEffect eFadeEffect = FadeEffect_NONE;
    GET( eFadeEffect, Effect );

    OSL_TRACE("fade effect %d", eFadeEffect);

    sal_Int16 nTransitionType = 0, nTransitionSubtype = 0;
    sal_Int8 nPPTTransitionType = 0;
    sal_uInt8 nDirection = 0;

    if( GETA( TransitionType ) && ( mAny >>= nTransitionType ) &&
    GETA( TransitionSubtype ) && ( mAny >>= nTransitionSubtype ) )
    nPPTTransitionType = GetTransition( nTransitionType, nTransitionSubtype, eFadeEffect, nDirection );

    if( !nPPTTransitionType && eFadeEffect != FadeEffect_NONE )
    nPPTTransitionType = GetTransition( eFadeEffect, nDirection );

    if( nPPTTransitionType ) {
    AnimationSpeed animationSpeed = AnimationSpeed_MEDIUM;
    const char* speed = NULL;
    sal_Int32 advanceTiming = -1;
    sal_Int32 changeType = 0;

    if( GETA( Speed ) ) {
        mAny >>= animationSpeed;

        switch( animationSpeed ) {
        default:
        case AnimationSpeed_MEDIUM:
            speed = "med";
            break;
        case AnimationSpeed_SLOW:
            speed = "slow";
            break;
        case AnimationSpeed_FAST:
            break;
        }
    }

    if( GETA( Change ) )
        mAny >>= changeType;

    // 1 means automatic, 2 half automatic - not sure what it means - at least I don't see it in UI
    if( changeType == 1 && GETA( Duration ) )
        mAny >>= advanceTiming;

    pFS->startElementNS( XML_p, XML_transition,
                 XML_spd, speed,
                 XML_advTm, advanceTiming != -1 ? I32S( advanceTiming*1000 ) : NULL,
                 FSEND );

    sal_Int32 nTransition = 0;
    const char* pDirection = NULL;
    const char* pOrientation = NULL;
    const char* pThruBlk = NULL;
    const char* pSpokes = NULL;
    char pSpokesTmp[2] = "0";

    switch( nPPTTransitionType ) {
        case PPT_TRANSITION_TYPE_BLINDS:
        nTransition = XML_blinds;
        pDirection = ( nDirection == 0) ? "vert" : "horz";
        break;
        case PPT_TRANSITION_TYPE_CHECKER:
        nTransition = XML_checker;
        pDirection = ( nDirection == 1) ? "vert" : "horz";
        break;
        case PPT_TRANSITION_TYPE_CIRCLE:
        nTransition = XML_circle;
        break;
        case PPT_TRANSITION_TYPE_COMB:
        nTransition = XML_comb;
        pDirection = ( nDirection == 1) ? "vert" : "horz";
        break;
        case PPT_TRANSITION_TYPE_COVER:
        nTransition = XML_cover;
        pDirection = Get8Direction( nDirection );
        break;
// we don't have cut transition AFAIK
//      case PPT_TRANSITION_TYPE_CUT:
//      nTransition = XML_cut;
//      break;
        case PPT_TRANSITION_TYPE_DIAMOND:
        nTransition = XML_diamond;
        break;
        case PPT_TRANSITION_TYPE_DISSOLVE:
        nTransition = XML_dissolve;
        break;
        case PPT_TRANSITION_TYPE_FADE:
        nTransition = XML_fade;
        pThruBlk = "true";
        break;
        case PPT_TRANSITION_TYPE_SMOOTHFADE:
        nTransition = XML_fade;
        break;
        case PPT_TRANSITION_TYPE_NEWSFLASH:
        nTransition = XML_newsflash;
        break;
        case PPT_TRANSITION_TYPE_PLUS:
        nTransition = XML_plus;
        break;
        case PPT_TRANSITION_TYPE_PULL:
        nTransition = XML_pull;
        pDirection = Get8Direction( nDirection );
        break;
        case PPT_TRANSITION_TYPE_PUSH:
        nTransition = XML_push;
        pDirection = GetSideDirection( nDirection );
        break;
        case PPT_TRANSITION_TYPE_RANDOM:
        nTransition = XML_random;
        break;
        case PPT_TRANSITION_TYPE_RANDOM_BARS:
        nTransition = XML_randomBar;
        pDirection = ( nDirection == 1) ? "vert" : "horz";
        break;
        case PPT_TRANSITION_TYPE_SPLIT:
        nTransition = XML_split;
        pDirection = ( nDirection & 1) ? "in" : "out";
        pOrientation = ( nDirection < 2) ? "horz" : "vert";
        break;
        case PPT_TRANSITION_TYPE_STRIPS:
        nTransition = XML_strips;
        pDirection = GetCornerDirection( nDirection );
        break;
        case PPT_TRANSITION_TYPE_WEDGE:
        nTransition = XML_wedge;
        break;
        case PPT_TRANSITION_TYPE_WHEEL:
        nTransition = XML_wheel;
        if( nDirection != 4 && nDirection <= 9 ) {
            pSpokesTmp[0] = '0' + nDirection;
            pSpokes = pSpokesTmp;
        }
        break;
        case PPT_TRANSITION_TYPE_WIPE:
        nTransition = XML_wipe;
        pDirection = GetSideDirection( nDirection );
        break;
        case PPT_TRANSITION_TYPE_ZOOM:
        nTransition = XML_zoom;
        pDirection = ( nDirection == 1) ? "in" : "out";
        break;
        case PPT_TRANSITION_TYPE_NONE:
        default:
        nTransition = 0;
    }

    if( nTransition )
        pFS->singleElementNS( XML_p, nTransition,
                  XML_dir, pDirection,
                  XML_orient, pOrientation,
                  XML_spokes, pSpokes,
                  XML_thruBlk, pThruBlk,
                  FSEND );

    pFS->endElementNS( XML_p, XML_transition );
    }

    pFS->endElementNS( XML_p, XML_sld );

    // add implicit relation to slide layout
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" ),
                 OUStringBuffer()
                 .appendAscii( "../slideLayouts/slideLayout" )
                 .append( GetLayoutFileId( GetLayoutOffset( mXPagePropSet ), nMasterNum ) )
                 .appendAscii( ".xml" )
                 .makeStringAndClear() );

    DBG(printf("----------------\n"));
}

void PowerPointExport::ImplWriteNotes( sal_uInt32 nPageNum )
{
    if( !mbCreateNotes || !ContainsOtherShapeThanPlaceholders( TRUE ) )
    return;

    DBG(printf("write Notes %d\n----------------\n", nPageNum));

    FSHelperPtr pFS = openOutputStreamWithSerializer( OUStringBuffer()
                                                      .appendAscii( "ppt/notesSlides/notesSlide" )
                                                      .append( (sal_Int32) nPageNum + 1 )
                                                      .appendAscii( ".xml" )
                                                      .makeStringAndClear(),
                                                      US( "application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml" ) );

    pFS->startElementNS( XML_p, XML_notes, PNMSS, FSEND );

    pFS->startElementNS( XML_p, XML_cSld, FSEND );

    // background
//     if( bHasBackground ) {
//         ImplWriteBackground( pFS, aXBackgroundPropSet );
//     }

    WriteShapeTree( pFS, NOTICE, FALSE );

    pFS->endElementNS( XML_p, XML_cSld );

    pFS->endElementNS( XML_p, XML_notes );

    // add implicit relation to slide
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" ),
                 OUStringBuffer()
                 .appendAscii( "../slides/slide" )
                 .append( (sal_Int32) nPageNum + 1 )
                 .appendAscii( ".xml" )
                 .makeStringAndClear() );

    // add slide implicit relation to notes
    if( mpSlidesFSArray.size() >= nPageNum )
    addRelation( mpSlidesFSArray[ nPageNum ]->getOutputStream(),
             US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/notesSlide" ),
             OUStringBuffer()
             .appendAscii( "../notesSlides/notesSlide" )
             .append( (sal_Int32) nPageNum + 1 )
             .appendAscii( ".xml" )
             .makeStringAndClear() );

    // add implicit relation to notes master
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/notesMaster" ),
         US( "../notesMasters/notesMaster1.xml" ) );

    DBG(printf("----------------\n"));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

void PowerPointExport::AddLayoutIdAndRelation( FSHelperPtr pFS, sal_Int32 nLayoutFileId )
{
    // add implicit relation of slide master to slide layout
    OUString sRelId = addRelation( pFS->getOutputStream(),
                   US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" ),
                   OUStringBuffer()
                   .appendAscii( "../slideLayouts/slideLayout" )
                   .append( nLayoutFileId )
                   .appendAscii( ".xml" )
                   .makeStringAndClear() );

    pFS->singleElementNS( XML_p, XML_sldLayoutId,
              XML_id, I64S( GetNewSlideMasterId() ),
              FSNS( XML_r, XML_id ), USS( sRelId ),
              FSEND );
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

sal_Int32 PowerPointExport::nStyleLevelToken[5] =
{
    XML_lvl1pPr,
    XML_lvl2pPr,
    XML_lvl3pPr,
    XML_lvl4pPr,
    XML_lvl5pPr
};

void PowerPointExport::WriteTextStyleLevel( FSHelperPtr pFS, int nInstance, int nLevel )
{
    OSL_ASSERT( nLevel >= 0 && nLevel < 5 );
    OSL_ASSERT( nInstance >= 0 && nInstance < 9 );

    PPTExCharLevel rCharLevel = mpStyleSheet->GetCharSheet( nInstance ).maCharLevel[ nLevel ];
    PPTExParaLevel rParaLevel = mpStyleSheet->GetParaSheet( nInstance ).maParaLevel[ nLevel ];

    pFS->startElementNS( XML_a, PowerPointExport::nStyleLevelToken[ nLevel ],
             XML_algn, DrawingML::GetAlignment( rParaLevel.mnOOAdjust ),
             FSEND );

    pFS->endElementNS( XML_a, PowerPointExport::nStyleLevelToken[ nLevel ] );
}

void PowerPointExport::WriteTextStyle( FSHelperPtr pFS, int nInstance, sal_Int32 xmlToken )
{
    pFS->startElementNS( XML_p, xmlToken, FSEND );

    for( int nLevel = 0; nLevel < 5; nLevel ++ )
    WriteTextStyleLevel( pFS, nInstance, nLevel );

    pFS->endElementNS( XML_p, xmlToken );
}

void PowerPointExport::WriteTextStyles( FSHelperPtr pFS )
{
    pFS->startElementNS( XML_p, XML_txBody, FSEND );

    WriteTextStyle( pFS, EPP_TEXTTYPE_Title, XML_titleStyle );
    WriteTextStyle( pFS, EPP_TEXTTYPE_Body, XML_bodyStyle );
    WriteTextStyle( pFS, EPP_TEXTTYPE_Other, XML_otherStyle );

    pFS->endElementNS( XML_p, XML_txBody );
}

void PowerPointExport::ImplWriteSlideMaster( sal_uInt32 nPageNum, Reference< XPropertySet > aXBackgroundPropSet )
{
    DBG(printf("write slide master: %d\n----------------\n", nPageNum));

    // slides list
    if( nPageNum == 0 )
        mPresentationFS->startElementNS( XML_p, XML_sldMasterIdLst, FSEND );

    OUString sRelId = addRelation( mPresentationFS->getOutputStream(),
                                   US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" ),
                                   OUStringBuffer()
                                   .appendAscii( "slideMasters/slideMaster" )
                                   .append( (sal_Int32) nPageNum + 1 )
                                   .appendAscii( ".xml" )
                                   .makeStringAndClear() );

    mPresentationFS->singleElementNS( XML_p, XML_sldMasterId,
                                      XML_id, OString::valueOf( (sal_Int64) GetNewSlideMasterId() ).getStr(),
                                      FSNS( XML_r, XML_id ), USS( sRelId ),
                                      FSEND );

    if( nPageNum == mnMasterPages - 1 )
        mPresentationFS->endElementNS( XML_p, XML_sldMasterIdLst );

    FSHelperPtr pFS =
    openOutputStreamWithSerializer( OUStringBuffer()
                    .appendAscii( "ppt/slideMasters/slideMaster" )
                    .append( (sal_Int32) nPageNum + 1 )
                    .appendAscii( ".xml" )
                    .makeStringAndClear(),
                    US( "application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml" ) );
    if( mpMasterFSArray.size() < mnMasterPages )
    mpMasterFSArray.resize( mnMasterPages );
    mpMasterFSArray[ nPageNum ] = pFS;

    // write theme per master
    WriteTheme( nPageNum );

    // add implicit relation to the presentation theme
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" ),
         OUStringBuffer()
         .appendAscii( "../theme/theme" )
         .append( (sal_Int32) nPageNum + 1 )
         .appendAscii( ".xml" )
         .makeStringAndClear() );

    pFS->startElementNS( XML_p, XML_sldMaster, PNMSS, FSEND );

    pFS->startElementNS( XML_p, XML_cSld, FSEND );

    ImplWriteBackground( pFS, aXBackgroundPropSet );
    WriteShapeTree( pFS, MASTER, TRUE );

    pFS->endElementNS( XML_p, XML_cSld );

    // color map - now it uses colors from hardcoded theme, once we eventually generate theme, this might need update
    pFS->singleElementNS( XML_p, XML_clrMap,
                          XML_bg1, "lt1",
                          XML_bg2, "lt2",
                          XML_tx1, "dk1",
                          XML_tx2, "dk2",
                          XML_accent1, "accent1",
                          XML_accent2, "accent2",
                          XML_accent3, "accent3",
                          XML_accent4, "accent4",
                          XML_accent5, "accent5",
                          XML_accent6, "accent6",
                          XML_hlink, "hlink",
                          XML_folHlink, "folHlink",
                          FSEND );

    // use master's id type as they have same range, mso does that as well
    pFS->startElementNS( XML_p, XML_sldLayoutIdLst, FSEND );

    int nCount = 0;
    for( int i = 0; i < EPP_LAYOUT_SIZE; i++) {

    sal_Int32 nLayoutFileId = GetLayoutFileId( i, nPageNum );
    if( nLayoutFileId > 0 ) {
        AddLayoutIdAndRelation( pFS, nLayoutFileId );
        nCount++;
    }
    }

    if( nCount == 0 ) {
    // add at least empty layout, so that we don't have master page
    // without layout, such master cannot be used in ppt
    ImplWriteLayout( 0, nPageNum );
    AddLayoutIdAndRelation( pFS, GetLayoutFileId( 0, nPageNum ) );
    }

    pFS->endElementNS( XML_p, XML_sldLayoutIdLst );

    // WriteTextStyles( pFS );

    pFS->endElementNS( XML_p, XML_sldMaster );

    DBG(printf("----------------\n"));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

sal_Int32 PowerPointExport::GetLayoutFileId( sal_Int32 nOffset, sal_uInt32 nMasterNum )
{
    if( mLayoutInfo[ nOffset ].mnFileIdArray.size() <= nMasterNum )
    return 0;

    return mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ];
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

void PowerPointExport::ImplWriteLayout( sal_Int32 nOffset, sal_uInt32 nMasterNum )
{
    if( mLayoutInfo[ nOffset ].mnFileIdArray.size() < mnMasterPages ) {
    mLayoutInfo[ nOffset ].mnFileIdArray.resize( mnMasterPages );
    }

    if( mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ] != 0 )
    return;

    FSHelperPtr pFS
        = openOutputStreamWithSerializer( OUStringBuffer()
                                          .appendAscii( "ppt/slideLayouts/slideLayout" )
                                          .append( (sal_Int32) mnLayoutFileIdMax )
                                          .appendAscii( ".xml" )
                                          .makeStringAndClear(),
                                          US( "application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml" ) );

//     LayoutInfo& rLayoutInfo = GetLayoutInfo( mXPagePropSet );

    // add implicit relation of slide layout to slide master
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" ),
                 OUStringBuffer()
                 .appendAscii( "../slideMasters/slideMaster" )
                 .append( (sal_Int32) nMasterNum + 1 )
                 .appendAscii( ".xml" )
                 .makeStringAndClear() );

    pFS->startElementNS( XML_p, XML_sldLayout,
                         PNMSS,
                         XML_type, "title",
                         XML_preserve, "1",
                         FSEND );

    pFS->startElementNS( XML_p, XML_cSld,
                         XML_name, "Title Slide",
                         FSEND );
    pFS->write( MINIMAL_SPTREE ); // TODO: write actual shape tree
    pFS->endElementNS( XML_p, XML_cSld );

    pFS->endElementNS( XML_p, XML_sldLayout );

    mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ] = mnLayoutFileIdMax;

    mnLayoutFileIdMax ++;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

void PowerPointExport::WriteShapeTree( FSHelperPtr pFS, PageType ePageType, sal_Bool bMaster )
{
    PowerPointShapeExport aDML( pFS, this );
    aDML.SetMaster( bMaster );
    aDML.SetPageType( ePageType );
    sal_uInt32 nShapes;

    pFS->startElementNS( XML_p, XML_spTree, FSEND );
    pFS->write( MAIN_GROUP );

    ResetGroupTable( nShapes = mXShapes->getCount() );

    while( GetNextGroupEntry() ) {

        sal_uInt32 nGroups = GetGroupsClosed();
        for ( sal_uInt32 i = 0; i < nGroups; i++ ) {
            DBG(printf( "leave group\n" ));
        }

        if ( GetShapeByIndex( GetCurrentGroupIndex(), TRUE ) ) {
            DBG(printf( "mType: \"%s\"\n", mType.GetBuffer() ));
            aDML.WriteShape( mXShape );
        }
    }

    pFS->endElementNS( XML_p, XML_spTree );
}

#define BEGIN_SHAPE pFS->startElementNS( XML_p, XML_sp, FSEND )
#define END_SHAPE pFS->endElementNS( XML_p, XML_sp )

void PowerPointExport::WritePageShape( FSHelperPtr pFS, ShapeExport& rDML, PageType ePageType )
{
    if( ePageType == NOTICE && mbPresObj )
    WritePlaceholderShape( pFS, rDML, SlideImage );
    else
    rDML.WriteTextShape( mXShape );
}

sal_Bool PowerPointExport::WritePlaceholder( FSHelperPtr pFS, ShapeExport& rDML, PlaceholderType ePlaceholder, sal_Bool bMaster )
{
    if( bMaster && ShapeExport::NonEmptyText( mXShape ) ) {
    WritePlaceholderShape( pFS, rDML, ePlaceholder );

    return TRUE;
    }

    return FALSE;
}

void PowerPointExport::WritePlaceholderShape( FSHelperPtr pFS, ShapeExport& rDML, PlaceholderType ePlaceholder )
{
    BEGIN_SHAPE;

    // non visual shape properties
    pFS->startElementNS( XML_p, XML_nvSpPr, FSEND );
    rDML.WriteNonVisualDrawingProperties( mXShape, IDS( PlaceHolder ) );
    pFS->startElementNS( XML_p, XML_cNvSpPr, FSEND );
    pFS->singleElementNS( XML_a, XML_spLocks, XML_noGrp, "1", FSEND );
    pFS->endElementNS( XML_p, XML_cNvSpPr );
    pFS->startElementNS( XML_p, XML_nvPr, FSEND );

    const char* pType = NULL;
    switch( ePlaceholder ) {
    case SlideImage:
        pType = "sldImg";
        break;
    case Notes:
        pType = "body";
        break;
    case Header:
        pType = "hdr";
        break;
    case Footer:
        pType = "ftr";
        break;
    case SlideNumber:
        pType = "sldNum";
        break;
    case DateAndTime:
        pType = "dt";
        break;
    case Outliner:
        pType = "body";
        break;
    case Title:
        pType = "title";
        break;
    case Subtitle:
        pType = "subTitle";
        break;
    default:
        DBG(printf("warning: unhandled placeholder type: %d\n", ePlaceholder));
    }
    pFS->singleElementNS( XML_p, XML_ph, XML_type, pType, FSEND );
    pFS->endElementNS( XML_p, XML_nvPr );
    pFS->endElementNS( XML_p, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( XML_p, XML_spPr, FSEND );
    rDML.WriteShapeTransformation( mXShape );
    rDML.WritePresetShape( "rect" );
    rDML.WriteBlipFill( mXPropSet, S( "GraphicURL" ) );
    pFS->endElementNS( XML_p, XML_spPr );

    rDML.WriteTextBox( mXShape );

    END_SHAPE;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define MINIMAL_THEME "  <a:themeElements>\
    <a:clrScheme name=\"Office\">\
      <a:dk1>\
        <a:sysClr val=\"windowText\" lastClr=\"000000\"/>\
      </a:dk1>\
      <a:lt1>\
        <a:sysClr val=\"window\" lastClr=\"FFFFFF\"/>\
      </a:lt1>\
      <a:dk2>\
        <a:srgbClr val=\"1F497D\"/>\
      </a:dk2>\
      <a:lt2>\
        <a:srgbClr val=\"EEECE1\"/>\
      </a:lt2>\
      <a:accent1>\
        <a:srgbClr val=\"4F81BD\"/>\
      </a:accent1>\
      <a:accent2>\
        <a:srgbClr val=\"C0504D\"/>\
      </a:accent2>\
      <a:accent3>\
        <a:srgbClr val=\"9BBB59\"/>\
      </a:accent3>\
      <a:accent4>\
        <a:srgbClr val=\"8064A2\"/>\
      </a:accent4>\
      <a:accent5>\
        <a:srgbClr val=\"4BACC6\"/>\
      </a:accent5>\
      <a:accent6>\
        <a:srgbClr val=\"F79646\"/>\
      </a:accent6>\
      <a:hlink>\
        <a:srgbClr val=\"0000FF\"/>\
      </a:hlink>\
      <a:folHlink>\
        <a:srgbClr val=\"800080\"/>\
      </a:folHlink>\
    </a:clrScheme>\
    <a:fontScheme name=\"Office\">\
      <a:majorFont>\
        <a:latin typeface=\"Arial\"/>\
        <a:ea typeface=\"DejaVu Sans\"/>\
        <a:cs typeface=\"DejaVu Sans\"/>\
      </a:majorFont>\
      <a:minorFont>\
        <a:latin typeface=\"Arial\"/>\
        <a:ea typeface=\"DejaVu Sans\"/>\
        <a:cs typeface=\"DejaVu Sans\"/>\
      </a:minorFont>\
    </a:fontScheme>\
    <a:fmtScheme name=\"Office\">\
      <a:fillStyleLst>\
        <a:solidFill>\
          <a:schemeClr val=\"phClr\"/>\
        </a:solidFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"50000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"35000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"37000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"15000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:lin ang=\"16200000\" scaled=\"1\"/>\
        </a:gradFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"51000\"/>\
                <a:satMod val=\"130000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"80000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"93000\"/>\
                <a:satMod val=\"130000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"94000\"/>\
                <a:satMod val=\"135000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:lin ang=\"16200000\" scaled=\"0\"/>\
        </a:gradFill>\
      </a:fillStyleLst>\
      <a:lnStyleLst>\
        <a:ln w=\"9525\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\">\
              <a:shade val=\"95000\"/>\
              <a:satMod val=\"105000\"/>\
            </a:schemeClr>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
        <a:ln w=\"25400\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\"/>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
        <a:ln w=\"38100\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\"/>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
      </a:lnStyleLst>\
      <a:effectStyleLst>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"20000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"38000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
        </a:effectStyle>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"35000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
        </a:effectStyle>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"35000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
          <a:scene3d>\
            <a:camera prst=\"orthographicFront\">\
              <a:rot lat=\"0\" lon=\"0\" rev=\"0\"/>\
            </a:camera>\
            <a:lightRig rig=\"threePt\" dir=\"t\">\
              <a:rot lat=\"0\" lon=\"0\" rev=\"1200000\"/>\
            </a:lightRig>\
          </a:scene3d>\
          <a:sp3d>\
            <a:bevelT w=\"63500\" h=\"25400\"/>\
          </a:sp3d>\
        </a:effectStyle>\
      </a:effectStyleLst>\
      <a:bgFillStyleLst>\
        <a:solidFill>\
          <a:schemeClr val=\"phClr\"/>\
        </a:solidFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"40000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"40000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"45000\"/>\
                <a:shade val=\"99000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"20000\"/>\
                <a:satMod val=\"255000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:path path=\"circle\">\
            <a:fillToRect l=\"50000\" t=\"-80000\" r=\"50000\" b=\"180000\"/>\
          </a:path>\
        </a:gradFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"80000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"30000\"/>\
                <a:satMod val=\"200000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:path path=\"circle\">\
            <a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/>\
          </a:path>\
        </a:gradFill>\
      </a:bgFillStyleLst>\
    </a:fmtScheme>\
  </a:themeElements>"

void PowerPointExport::WriteTheme( sal_Int32 nThemeNum )
{
    FSHelperPtr pFS = openOutputStreamWithSerializer( OUStringBuffer()
                              .appendAscii( "ppt/theme/theme" )
                              .append( (sal_Int32) nThemeNum + 1 )
                              .appendAscii( ".xml" )
                              .makeStringAndClear(),
                                                      US( "application/vnd.openxmlformats-officedocument.theme+xml" ) );

    pFS->startElementNS( XML_a, XML_theme,
                         FSNS( XML_xmlns, XML_a), "http://schemas.openxmlformats.org/drawingml/2006/main",
                         XML_name, "Office Theme",
                         FSEND );

    pFS->write( MINIMAL_THEME );
    pFS->endElementNS( XML_a, XML_theme );
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

sal_Bool PowerPointExport::ImplCreateDocument()
{
    mbCreateNotes = FALSE;

    for( sal_uInt32 i = 0; i < mnPages; i++ )
    {
        if ( !GetPageByIndex( i, NOTICE ) )
            return FALSE;

    if( ContainsOtherShapeThanPlaceholders( TRUE ) ) {
        mbCreateNotes = TRUE;
        break;
    }
    }

    return TRUE;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

sal_Bool PowerPointExport::WriteNotesMaster()
{
    DBG(printf("write Notes master\n----------------\n"));

    mPresentationFS->startElementNS( XML_p, XML_notesMasterIdLst, FSEND );

    OUString sRelId = addRelation( mPresentationFS->getOutputStream(),
                                   US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/notesMaster" ),
                                   US( "notesMasters/notesMaster1.xml" ) );

    mPresentationFS->singleElementNS( XML_p, XML_notesMasterId,
                                      FSNS( XML_r, XML_id ), USS( sRelId ),
                                      FSEND );

    mPresentationFS->endElementNS( XML_p, XML_notesMasterIdLst );

    FSHelperPtr pFS =
    openOutputStreamWithSerializer( US( "ppt/notesMasters/notesMaster1.xml" ),
                    US( "application/vnd.openxmlformats-officedocument.presentationml.notesMaster+xml" ) );
    // write theme per master
    WriteTheme( mnMasterPages );

    // add implicit relation to the presentation theme
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" ),
         OUStringBuffer()
         .appendAscii( "../theme/theme" )
         .append( (sal_Int32) mnMasterPages + 1 )
         .appendAscii( ".xml" )
         .makeStringAndClear() );

    pFS->startElementNS( XML_p, XML_notesMaster, PNMSS, FSEND );

    pFS->startElementNS( XML_p, XML_cSld, FSEND );

    Reference< XPropertySet > aXBackgroundPropSet;
    if( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Background" ) ) ) &&
    ( mAny >>= aXBackgroundPropSet ) )
    ImplWriteBackground( pFS, aXBackgroundPropSet );

    WriteShapeTree( pFS, NOTICE, TRUE );

    pFS->endElementNS( XML_p, XML_cSld );

    // color map - now it uses colors from hardcoded theme, once we eventually generate theme, this might need update
    pFS->singleElementNS( XML_p, XML_clrMap,
                          XML_bg1, "lt1",
                          XML_bg2, "lt2",
                          XML_tx1, "dk1",
                          XML_tx2, "dk2",
                          XML_accent1, "accent1",
                          XML_accent2, "accent2",
                          XML_accent3, "accent3",
                          XML_accent4, "accent4",
                          XML_accent5, "accent5",
                          XML_accent6, "accent6",
                          XML_hlink, "hlink",
                          XML_folHlink, "folHlink",
                          FSEND );

    pFS->endElementNS( XML_p, XML_notesMaster );

    DBG(printf("----------------\n"));

    return TRUE;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

sal_Bool PowerPointExport::ImplCreateMainNotes()
{
    if( mbCreateNotes )
    return WriteNotesMaster();

    return TRUE;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

drawingml::chart::ChartConverter& PowerPointExport::getChartConverter()
{
    return *mxChartConv;
}

#define IMPL_NAME "com.sun.star.comp.Impress.oox.PowerPointExport"

OUString SAL_CALL PowerPointExport_getImplementationName() throw()
{
    return CREATE_OUSTRING( IMPL_NAME );
}

uno::Sequence< OUString > SAL_CALL PowerPointExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName = CREATE_OUSTRING( "com.sun.star.comp.ooxpptx" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL PowerPointExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new PowerPointExport( rSMgr );
}

OUString PowerPointExport::implGetImplementationName() const
{
    return PowerPointExport_getImplementationName();
}
}
}

// UNO component
// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// -----------------------
// - component_writeInfo -
// -----------------------

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo( void* /* pServiceManager */, void* pRegistryKey )
{
    sal_Bool bRet = sal_False;

    if( pRegistryKey )
    {
        try
        {
            uno::Reference< registry::XRegistryKey > xNewKey1(
                static_cast< registry::XRegistryKey* >( pRegistryKey )->createKey(
                ::rtl::OUString::createFromAscii( IMPL_NAME "/UNO/SERVICES/" ) ) );
            xNewKey1->createKey( oox::core::PowerPointExport_getSupportedServiceNames().getConstArray()[0] );

            bRet = sal_True;
        }
        catch( registry::InvalidRegistryException& )
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return bRet;
}

// ------------------------
// - component_getFactory -
// ------------------------

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /* pRegistryKey */ )
{
    uno::Reference< lang::XSingleServiceFactory > xFactory;
    void*                                   pRet = 0;

    if( rtl_str_compare( pImplName, IMPL_NAME ) == 0 )
    {
        const ::rtl::OUString aServiceName( ::rtl::OUString::createFromAscii( IMPL_NAME ) );

        xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
                        reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                        oox::core::PowerPointExport_getImplementationName(),
                        oox::core::PowerPointExport_createInstance,
                        oox::core::PowerPointExport_getSupportedServiceNames() ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

#ifdef __cplusplus
}
#endif

DBG(
void dump_pset(Reference< XPropertySet > rXPropSet)
{
    Reference< XPropertySetInfo > info = rXPropSet->getPropertySetInfo ();
    Sequence< beans::Property > props = info->getProperties ();

    for (int i=0; i < props.getLength (); i++) {
        OString name = OUStringToOString( props [i].Name, RTL_TEXTENCODING_UTF8);
        printf ("%30s = ", name.getStr() );

        Any value = rXPropSet->getPropertyValue( props [i].Name );

        OUString strValue;
        sal_Int32 intValue;
        bool boolValue;
        RectanglePoint pointValue;

        if( value >>= strValue )
            printf ("\"%s\"\n", USS( strValue ) );
        else if( value >>= intValue )
            printf ("%d            (hex: %x)\n", intValue, intValue);
        else if( value >>= boolValue )
            printf ("%d            (bool)\n", boolValue);
        else if( value >>= pointValue )
            printf ("%d            (RectanglePoint)\n", pointValue);
        else
            printf ("???           <unhandled type>\n");
    }
}
);
