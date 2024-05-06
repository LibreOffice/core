/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_feature_desktop.h>

#include "SeriesPlotterContainer.hxx"

#include <ChartView.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <Diagram.hxx>
#include <ChartType.hxx>
#include <DataSeries.hxx>
#include <NumberFormatterWrapper.hxx>
#include <VDiagram.hxx>
#include "VTitle.hxx"
#include "VButton.hxx"
#include <ShapeFactory.hxx>
#include <BaseCoordinateSystem.hxx>
#include <VCoordinateSystem.hxx>
#include <VSeriesPlotter.hxx>
#include <CommonConverters.hxx>
#include <TitleHelper.hxx>
#include <Legend.hxx>
#include <LegendHelper.hxx>
#include "VLegend.hxx"
#include <PropertyMapper.hxx>
#include <ChartModel.hxx>
#include <ChartTypeHelper.hxx>
#include <ScaleAutomatism.hxx>
#include <ObjectIdentifier.hxx>
#include <DiagramHelper.hxx>
#include <RelativePositionHelper.hxx>
#include <servicenames.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include "AxisUsage.hxx"
#include <AxisIndexDefines.hxx>
#include <BaseGFXHelper.hxx>
#include <DataSeriesHelper.hxx>
#include <DateHelper.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <defines.hxx>
#include <comphelper/dumpxmltostring.hxx>
#include <unonames.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/eeitem.hxx>
#include <tools/globname.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/math.hxx>
#include <unotools/streamwrap.hxx>
#include <svx/svdpage.hxx>
#include <svx/unopage.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svx/unofill.hxx>
#include <drawinglayer/XShapeDumper.hxx>
#include <sfx2/objsh.hxx>

#include <time.h>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/data/XPivotTableDataProvider.hpp>
#include <com/sun/star/chart2/data/PivotTableFieldEntry.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <svl/itempool.hxx>
#include <svl/ctloptions.hxx>
#include <comphelper/classids.hxx>
#include <servicenames_charttypes.hxx>


#include <rtl/ustring.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <tools/stream.hxx>

#include <memory>
#include <libxml/xmlwriter.h>

namespace chart {

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

struct CreateShapeParam2D
{
    css::awt::Rectangle maRemainingSpace;

    std::shared_ptr<SeriesPlotterContainer> mpSeriesPlotterContainer;

    std::shared_ptr<VTitle> mpVTitleX;
    std::shared_ptr<VTitle> mpVTitleY;
    std::shared_ptr<VTitle> mpVTitleZ;

    std::shared_ptr<VTitle> mpVTitleSecondX;
    std::shared_ptr<VTitle> mpVTitleSecondY;

    rtl::Reference<SvxShapeRect> mxMarkHandles;
    rtl::Reference<SvxShapeRect> mxPlotAreaWithAxes;

    rtl::Reference<SvxShapeGroup> mxDiagramWithAxesShapes;

    bool mbAutoPosTitleX;
    bool mbAutoPosTitleY;
    bool mbAutoPosTitleZ;

    bool mbAutoPosSecondTitleX;
    bool mbAutoPosSecondTitleY;

    bool mbUseFixedInnerSize;

    CreateShapeParam2D() :
        mbAutoPosTitleX(true),
        mbAutoPosTitleY(true),
        mbAutoPosTitleZ(true),
        mbAutoPosSecondTitleX(true),
        mbAutoPosSecondTitleY(true),
        mbUseFixedInnerSize(false) {}
};



ChartView::ChartView(
        uno::Reference<uno::XComponentContext> xContext,
        ChartModel& rModel)
    : m_xCC(std::move(xContext))
    , mrChartModel(rModel)
    , m_bViewDirty(true)
    , m_bInViewUpdate(false)
    , m_bViewUpdatePending(false)
    , m_bRefreshAddIn(true)
    , m_aPageResolution(1000,1000)
    , m_bPointsWereSkipped(false)
    , m_nScaleXNumerator(1)
    , m_nScaleXDenominator(1)
    , m_nScaleYNumerator(1)
    , m_nScaleYDenominator(1)
    , m_bSdrViewIsInEditMode(false)
    , m_aResultingDiagramRectangleExcludingAxes(0,0,0,0)
{
    init();
}

void ChartView::init()
{
    if( !m_pDrawModelWrapper )
    {
        SolarMutexGuard aSolarGuard;
        m_pDrawModelWrapper = std::make_shared< DrawModelWrapper >();
        m_xShapeFactory = m_pDrawModelWrapper->getShapeFactory();
        m_xDrawPage = m_pDrawModelWrapper->getMainDrawPage();
        StartListening( m_pDrawModelWrapper->getSdrModel() );
    }
}

void SAL_CALL ChartView::initialize( const uno::Sequence< uno::Any >& )
{
    init();
}

ChartView::~ChartView()
{
    maTimeBased.maTimer.Stop();
    // #i120831#. In ChartView::initialize(), m_xShapeFactory is created from SdrModel::getUnoModel() and indirectly
    //   from SfxBaseModel, it needs call dispose() to make sure SfxBaseModel object is freed correctly.
    uno::Reference< lang::XComponent > xComp( m_xShapeFactory, uno::UNO_QUERY);
    if ( xComp.is() )
        xComp->dispose();

    if( m_pDrawModelWrapper )
    {
        SolarMutexGuard aSolarGuard;
        EndListening( m_pDrawModelWrapper->getSdrModel() );
        m_pDrawModelWrapper.reset();
    }
    m_xDrawPage = nullptr;
    impl_deleteCoordinateSystems();
}

void ChartView::impl_deleteCoordinateSystems()
{
    //delete all coordinate systems
    m_aVCooSysList.clear();
}

// datatransfer::XTransferable
namespace
{
constexpr OUString lcl_aGDIMetaFileMIMEType(
    u"application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\""_ustr );
constexpr OUString lcl_aGDIMetaFileMIMETypeHighContrast(
    u"application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\""_ustr );
} // anonymous namespace

void ChartView::getMetaFile( const uno::Reference< io::XOutputStream >& xOutStream
                           , bool bUseHighContrast )
{
    if( !m_xDrawPage.is() )
        return;

    // creating the graphic exporter
    uno::Reference< drawing::XGraphicExportFilter > xExporter = drawing::GraphicExportFilter::create( m_xCC );

    uno::Sequence< beans::PropertyValue > aFilterData{
        comphelper::makePropertyValue(u"ExportOnlyBackground"_ustr, false),
        comphelper::makePropertyValue(u"HighContrast"_ustr, bUseHighContrast),
        comphelper::makePropertyValue(u"Version"_ustr, sal_Int32(SOFFICE_FILEFORMAT_50)),
        comphelper::makePropertyValue(u"CurrentPage"_ustr, uno::Reference< uno::XInterface >( static_cast<cppu::OWeakObject*>(m_xDrawPage.get()), uno::UNO_QUERY )),
        //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
        comphelper::makePropertyValue(u"ScaleXNumerator"_ustr, m_nScaleXNumerator),
        comphelper::makePropertyValue(u"ScaleXDenominator"_ustr, m_nScaleXDenominator),
        comphelper::makePropertyValue(u"ScaleYNumerator"_ustr, m_nScaleYNumerator),
        comphelper::makePropertyValue(u"ScaleYDenominator"_ustr, m_nScaleYDenominator)
    };

    uno::Sequence< beans::PropertyValue > aProps{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"SVM"_ustr),
        comphelper::makePropertyValue(u"OutputStream"_ustr, xOutStream),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData)
    };

    xExporter->setSourceDocument( m_xDrawPage );
    if( xExporter->filter( aProps ) )
    {
        xOutStream->flush();
        xOutStream->closeOutput();
        uno::Reference< io::XSeekable > xSeekable( xOutStream, uno::UNO_QUERY );
        if( xSeekable.is() )
            xSeekable->seek(0);
    }
}

uno::Any SAL_CALL ChartView::getTransferData( const datatransfer::DataFlavor& aFlavor )
{
    bool bHighContrastMetaFile( aFlavor.MimeType == lcl_aGDIMetaFileMIMETypeHighContrast);
    uno::Any aRet;
    if( ! (bHighContrastMetaFile || aFlavor.MimeType == lcl_aGDIMetaFileMIMEType) )
        return aRet;

    update();

    SvMemoryStream aStream( 1024, 1024 );
    rtl::Reference<utl::OStreamWrapper> pStreamWrapper = new utl::OStreamWrapper( aStream );

    this->getMetaFile( pStreamWrapper, bHighContrastMetaFile );

    pStreamWrapper->seek(0);
    sal_Int32 nBytesToRead = pStreamWrapper->available();
    uno::Sequence< sal_Int8 > aSeq( nBytesToRead );
    pStreamWrapper->readBytes( aSeq, nBytesToRead);
    aRet <<= aSeq;
    pStreamWrapper->closeInput();

    return aRet;
}
uno::Sequence< datatransfer::DataFlavor > SAL_CALL ChartView::getTransferDataFlavors()
{
    return
    {
        { lcl_aGDIMetaFileMIMEType, u"GDIMetaFile"_ustr, cppu::UnoType<uno::Sequence< sal_Int8 >>::get() },
        { lcl_aGDIMetaFileMIMETypeHighContrast, u"GDIMetaFile"_ustr, cppu::UnoType<uno::Sequence< sal_Int8 >>::get() }
    };
}
sal_Bool SAL_CALL ChartView::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
{
    return ( aFlavor.MimeType == lcl_aGDIMetaFileMIMEType ||
             aFlavor.MimeType == lcl_aGDIMetaFileMIMETypeHighContrast );
}

// lang::XServiceInfo

OUString SAL_CALL ChartView::getImplementationName()
{
    return CHART_VIEW_SERVICE_IMPLEMENTATION_NAME;
}

sal_Bool SAL_CALL ChartView::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ChartView::getSupportedServiceNames()
{
    return { CHART_VIEW_SERVICE_NAME };
}

static ::basegfx::B3DHomMatrix createTransformationSceneToScreen(
    const ::basegfx::B2IRectangle& rDiagramRectangleWithoutAxes )
{
    ::basegfx::B3DHomMatrix aM;
    aM.scale(double(rDiagramRectangleWithoutAxes.getWidth())/FIXED_SIZE_FOR_3D_CHART_VOLUME
            , -double(rDiagramRectangleWithoutAxes.getHeight())/FIXED_SIZE_FOR_3D_CHART_VOLUME, 1.0 );
    aM.translate(double(rDiagramRectangleWithoutAxes.getMinX())
        , double(rDiagramRectangleWithoutAxes.getMinY()+rDiagramRectangleWithoutAxes.getHeight()-1), 0);
    return aM;
}

namespace
{

bool lcl_IsPieOrDonut( const rtl::Reference< Diagram >& xDiagram )
{
    //special treatment for pie charts
    //the size is checked after complete creation to get the datalabels into the given space

    //todo: this is just a workaround at the moment for pie and donut labels
    return xDiagram->isPieOrDonutChart();
}

void lcl_setDefaultWritingMode( const std::shared_ptr< DrawModelWrapper >& pDrawModelWrapper, ChartModel& rModel)
{
    //get writing mode from parent document:
    if( !SvtCTLOptions::IsCTLFontEnabled() )
        return;

    try
    {
        sal_Int16 nWritingMode=-1;
        uno::Reference< beans::XPropertySet > xParentProps( rModel.getParent(), uno::UNO_QUERY );
        uno::Reference< style::XStyleFamiliesSupplier > xStyleFamiliesSupplier( xParentProps, uno::UNO_QUERY );
        if( xStyleFamiliesSupplier.is() )
        {
            uno::Reference< container::XNameAccess > xStylesFamilies( xStyleFamiliesSupplier->getStyleFamilies() );
            if( xStylesFamilies.is() )
            {
                if( !xStylesFamilies->hasByName( u"PageStyles"_ustr ) )
                {
                    //draw/impress is parent document
                    uno::Reference< lang::XMultiServiceFactory > xFatcory( xParentProps, uno::UNO_QUERY );
                    if( xFatcory.is() )
                    {
                        uno::Reference< beans::XPropertySet > xDrawDefaults( xFatcory->createInstance( u"com.sun.star.drawing.Defaults"_ustr ), uno::UNO_QUERY );
                        if( xDrawDefaults.is() )
                            xDrawDefaults->getPropertyValue( u"WritingMode"_ustr ) >>= nWritingMode;
                    }
                }
                else
                {
                    uno::Reference< container::XNameAccess > xPageStyles( xStylesFamilies->getByName( u"PageStyles"_ustr ), uno::UNO_QUERY );
                    if( xPageStyles.is() )
                    {
                        OUString aPageStyle;

                        uno::Reference< text::XTextDocument > xTextDocument( xParentProps, uno::UNO_QUERY );
                        if( xTextDocument.is() )
                        {
                            //writer is parent document
                            //retrieve the current page style from the text cursor property PageStyleName

                            uno::Reference< text::XTextEmbeddedObjectsSupplier > xTextEmbeddedObjectsSupplier( xTextDocument, uno::UNO_QUERY );
                            if( xTextEmbeddedObjectsSupplier.is() )
                            {
                                uno::Reference< container::XNameAccess > xEmbeddedObjects( xTextEmbeddedObjectsSupplier->getEmbeddedObjects() );
                                if( xEmbeddedObjects.is() )
                                {
                                    uno::Sequence< OUString > aNames( xEmbeddedObjects->getElementNames() );

                                    sal_Int32 nCount = aNames.getLength();
                                    for( sal_Int32 nN=0; nN<nCount; nN++ )
                                    {
                                        uno::Reference< beans::XPropertySet > xEmbeddedProps( xEmbeddedObjects->getByName( aNames[nN] ), uno::UNO_QUERY );
                                        if( xEmbeddedProps.is() )
                                        {
                                            static OUString aChartCLSID = SvGlobalName( SO3_SCH_CLASSID ).GetHexName();
                                            OUString aCLSID;
                                            xEmbeddedProps->getPropertyValue( u"CLSID"_ustr ) >>= aCLSID;
                                            if( aCLSID == aChartCLSID )
                                            {
                                                uno::Reference< text::XTextContent > xEmbeddedObject( xEmbeddedProps, uno::UNO_QUERY );
                                                if( xEmbeddedObject.is() )
                                                {
                                                    uno::Reference< text::XTextRange > xAnchor( xEmbeddedObject->getAnchor() );
                                                    if( xAnchor.is() )
                                                    {
                                                        uno::Reference< beans::XPropertySet > xAnchorProps( xAnchor, uno::UNO_QUERY );
                                                        if( xAnchorProps.is() )
                                                        {
                                                            xAnchorProps->getPropertyValue( u"WritingMode"_ustr ) >>= nWritingMode;
                                                        }
                                                        uno::Reference< text::XText > xText( xAnchor->getText() );
                                                        if( xText.is() )
                                                        {
                                                            uno::Reference< beans::XPropertySet > xTextCursorProps( xText->createTextCursor(), uno::UNO_QUERY );
                                                            if( xTextCursorProps.is() )
                                                                xTextCursorProps->getPropertyValue( u"PageStyleName"_ustr ) >>= aPageStyle;
                                                        }
                                                    }
                                                }
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            if( aPageStyle.isEmpty() )
                            {
                                uno::Reference< text::XText > xText( xTextDocument->getText() );
                                if( xText.is() )
                                {
                                    uno::Reference< beans::XPropertySet > xTextCursorProps( xText->createTextCursor(), uno::UNO_QUERY );
                                    if( xTextCursorProps.is() )
                                        xTextCursorProps->getPropertyValue( u"PageStyleName"_ustr ) >>= aPageStyle;
                                }
                            }
                            if(aPageStyle.isEmpty())
                                aPageStyle = "Standard";
                        }
                        else
                        {
                            //Calc is parent document
                            Reference< com::sun::star::beans::XPropertySetInfo > xInfo = xParentProps->getPropertySetInfo();
                            if (xInfo->hasPropertyByName(u"PageStyle"_ustr))
                            {
                                xParentProps->getPropertyValue( u"PageStyle"_ustr ) >>= aPageStyle;
                            }
                            if(aPageStyle.isEmpty())
                                aPageStyle = "Default";
                        }
                        if( nWritingMode == -1 || nWritingMode == text::WritingMode2::PAGE )
                        {
                            uno::Reference< beans::XPropertySet > xPageStyle( xPageStyles->getByName( aPageStyle ), uno::UNO_QUERY );
                            Reference< com::sun::star::beans::XPropertySetInfo > xInfo = xPageStyle->getPropertySetInfo();
                            if (xInfo->hasPropertyByName(u"WritingMode"_ustr))
                            {
                                if( xPageStyle.is() )
                                    xPageStyle->getPropertyValue( u"WritingMode"_ustr ) >>= nWritingMode;
                            }
                        }
                    }
                }
            }
        }
        if( nWritingMode != -1 && nWritingMode != text::WritingMode2::PAGE )
        {
            if( pDrawModelWrapper )
                pDrawModelWrapper->GetItemPool().SetUserDefaultItem(SvxFrameDirectionItem(static_cast<SvxFrameDirection>(nWritingMode), EE_PARA_WRITINGDIR) );
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2" );
    }
}

sal_Int16 lcl_getDefaultWritingModeFromPool( const std::shared_ptr<DrawModelWrapper>& pDrawModelWrapper )
{
    sal_Int16 nWritingMode = text::WritingMode2::LR_TB;
    if(!pDrawModelWrapper)
        return nWritingMode;

    const SfxPoolItem& rItem = pDrawModelWrapper->GetItemPool().GetUserOrPoolDefaultItem(EE_PARA_WRITINGDIR);
    nWritingMode
        = static_cast<sal_Int16>(static_cast<const SvxFrameDirectionItem&>(rItem).GetValue());
    return nWritingMode;
}

} //end anonymous namespace

awt::Rectangle ChartView::impl_createDiagramAndContent( const CreateShapeParam2D& rParam, const awt::Size& rPageSize )
{
    //return the used rectangle
    awt::Rectangle aUsedOuterRect(rParam.maRemainingSpace.X, rParam.maRemainingSpace.Y, 0, 0);

    rtl::Reference< Diagram > xDiagram( mrChartModel.getFirstChartDiagram() );
    if( !xDiagram.is())
        return aUsedOuterRect;

    sal_Int32 nDimensionCount = xDiagram->getDimension();
    if(!nDimensionCount)
    {
        //@todo handle mixed dimension
        nDimensionCount = 2;
    }

    basegfx::B2IRectangle aAvailableOuterRect = BaseGFXHelper::makeRectangle(rParam.maRemainingSpace);

    const std::vector< std::unique_ptr<VCoordinateSystem> >& rVCooSysList( rParam.mpSeriesPlotterContainer->getCooSysList() );
    auto& rSeriesPlotterList = rParam.mpSeriesPlotterContainer->getSeriesPlotterList();

    //create VAxis, so they can give necessary information for automatic scaling
    uno::Reference<util::XNumberFormatsSupplier> const xNumberFormatsSupplier(
            mrChartModel.getNumberFormatsSupplier());

    for (auto& rpVCooSys : rVCooSysList)
    {
        if (nDimensionCount == 3)
        {
            CuboidPlanePosition eLeftWallPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardLeftWall( xDiagram ) );
            CuboidPlanePosition eBackWallPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBackWall( xDiagram ) );
            CuboidPlanePosition eBottomPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBottom( xDiagram ) );
            rpVCooSys->set3DWallPositions( eLeftWallPos, eBackWallPos, eBottomPos );
        }
        rpVCooSys->createVAxisList(&mrChartModel, rPageSize, rParam.maRemainingSpace,
            rParam.mbUseFixedInnerSize, rSeriesPlotterList, getComponentContext());
    }

    // - prepare list of all axis and how they are used
    Date aNullDate = NumberFormatterWrapper( xNumberFormatsSupplier ).getNullDate();
    rParam.mpSeriesPlotterContainer->initAxisUsageList(aNullDate);
    rParam.mpSeriesPlotterContainer->doAutoScaling( mrChartModel );
    rParam.mpSeriesPlotterContainer->setScalesFromCooSysToPlotter();
    rParam.mpSeriesPlotterContainer->setNumberFormatsFromAxes();

    //create shapes

    //aspect ratio
    drawing::Direction3D aPreferredAspectRatio =
        rParam.mpSeriesPlotterContainer->getPreferredAspectRatio();

    rtl::Reference<SvxShapeGroupAnyD> xSeriesTargetInFrontOfAxis;
    rtl::Reference<SvxShapeGroupAnyD> xSeriesTargetBehindAxis;
    VDiagram aVDiagram(xDiagram, aPreferredAspectRatio, nDimensionCount);
    {//create diagram
        aVDiagram.init(rParam.mxDiagramWithAxesShapes);
        aVDiagram.createShapes(
            awt::Point(rParam.maRemainingSpace.X, rParam.maRemainingSpace.Y),
            awt::Size(rParam.maRemainingSpace.Width, rParam.maRemainingSpace.Height));

        xSeriesTargetInFrontOfAxis = aVDiagram.getCoordinateRegion();
        // It is preferable to use full size than minimum for pie charts
        if (!rParam.mbUseFixedInnerSize)
            aVDiagram.reduceToMinimumSize();
    }

    rtl::Reference<SvxShapeGroup> xTextTargetShapes =
        ShapeFactory::createGroup2D(rParam.mxDiagramWithAxesShapes);

    // - create axis and grids for all coordinate systems

    //init all coordinate systems
    for (auto& rpVCooSys : rVCooSysList)
    {
        rpVCooSys->initPlottingTargets(xSeriesTargetInFrontOfAxis, xTextTargetShapes, xSeriesTargetBehindAxis);

        rpVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));

        rpVCooSys->initVAxisInList();
    }

    //calculate resulting size respecting axis label layout and fontscaling

    rtl::Reference<SvxShapeGroup> xBoundingShape(rParam.mxDiagramWithAxesShapes);
    ::basegfx::B2IRectangle aConsumedOuterRect;

    //use first coosys only so far; todo: calculate for more than one coosys if we have more in future
    //todo: this is just a workaround at the moment for pie and donut labels
    bool bIsPieOrDonut = lcl_IsPieOrDonut(xDiagram);
    if( !bIsPieOrDonut && (!rVCooSysList.empty()) )
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[0].get();
        pVCooSys->createMaximumAxesLabels();

        aConsumedOuterRect = ShapeFactory::getRectangleOfShape(*xBoundingShape);
        ::basegfx::B2IRectangle aNewInnerRect( aVDiagram.getCurrentRectangle() );
        if (!rParam.mbUseFixedInnerSize)
            aNewInnerRect = aVDiagram.adjustInnerSize( aConsumedOuterRect );

        pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aNewInnerRect ) ));

        //redo autoscaling to get size and text dependent automatic main increment count
        rParam.mpSeriesPlotterContainer->doAutoScaling( mrChartModel );
        rParam.mpSeriesPlotterContainer->updateScalesAndIncrementsOnAxes();
        rParam.mpSeriesPlotterContainer->setScalesFromCooSysToPlotter();

        pVCooSys->createAxesLabels();

        bool bLessSpaceConsumedThanExpected = false;
        {
            aConsumedOuterRect = ShapeFactory::getRectangleOfShape(*xBoundingShape);
            if( aConsumedOuterRect.getMinX() > aAvailableOuterRect.getMinX()
                || aConsumedOuterRect.getMaxX() < aAvailableOuterRect.getMaxX()
                || aConsumedOuterRect.getMinY() > aAvailableOuterRect.getMinY()
                || aConsumedOuterRect.getMinY() < aAvailableOuterRect.getMaxY() )
            {
                bLessSpaceConsumedThanExpected = true;
            }
        }

        if (bLessSpaceConsumedThanExpected && !rParam.mbUseFixedInnerSize)
        {
            aVDiagram.adjustInnerSize( aConsumedOuterRect );
            pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
                createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));

            // Need to re-adjust again if the labels have changed height because of
            // text can break. Ideally this shouldn't be needed, but the chart height
            // isn't readjusted otherwise.
            pVCooSys->createAxesLabels();
            aConsumedOuterRect = ShapeFactory::getRectangleOfShape(*xBoundingShape);
            aVDiagram.adjustInnerSize(aConsumedOuterRect);
            pVCooSys->setTransformationSceneToScreen(B3DHomMatrixToHomogenMatrix(
                createTransformationSceneToScreen(aVDiagram.getCurrentRectangle())));

        }
        pVCooSys->updatePositions();//todo: logically this belongs to the condition above, but it seems also to be necessary to give the axes group shapes the right bounding rects for hit test -  probably caused by bug i106183 -> check again if fixed
    }

    //create axes and grids for the final size
    for (auto& rpVCooSys : rVCooSysList)
    {
        rpVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));

        rpVCooSys->createAxesShapes();
        rpVCooSys->createGridShapes();
    }

    // - create data series for all charttypes
    m_bPointsWereSkipped = false;
    for( const std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
    {
        VSeriesPlotter* pSeriesPlotter = aPlotter.get();
        rtl::Reference<SvxShapeGroupAnyD> xSeriesTarget;
        if( pSeriesPlotter->WantToPlotInFrontOfAxisLine() )
            xSeriesTarget = xSeriesTargetInFrontOfAxis;
        else
        {
            xSeriesTarget = xSeriesTargetBehindAxis;
            OSL_ENSURE( !bIsPieOrDonut, "not implemented yet! - during a complete recreation this shape is destroyed so no series can be created anymore" );
        }
        pSeriesPlotter->initPlotter( xSeriesTarget,xTextTargetShapes,OUString() );
        pSeriesPlotter->setPageReferenceSize( rPageSize );
        VCoordinateSystem* pVCooSys = SeriesPlotterContainer::getCooSysForPlotter( rVCooSysList, pSeriesPlotter );
        if(nDimensionCount==2)
            pSeriesPlotter->setTransformationSceneToScreen( pVCooSys->getTransformationSceneToScreen() );
        //better performance for big data
        {
            //calculate resolution for coordinate system
            Sequence<sal_Int32> aCoordinateSystemResolution = pVCooSys->getCoordinateSystemResolution( rPageSize, m_aPageResolution );
            pSeriesPlotter->setCoordinateSystemResolution( aCoordinateSystemResolution );
        }
        // Do not allow to move data labels in case of pie or donut chart, yet!
        pSeriesPlotter->setPieLabelsAllowToMove(!bIsPieOrDonut);
        // use the pagesize as remaining space if we have a fixed inner size
        if( rParam.mbUseFixedInnerSize )
            aAvailableOuterRect = BaseGFXHelper::makeRectangle(awt::Rectangle(0, 0, rPageSize.Width, rPageSize.Height));
        // set the available space for data labels to avoid moving out from chart area
        pSeriesPlotter->setAvailableOuterRect(aAvailableOuterRect);
        pSeriesPlotter->createShapes();
        m_bPointsWereSkipped = m_bPointsWereSkipped || pSeriesPlotter->PointsWereSkipped();
    }

    //recreate all with corrected sizes if requested
    if( bIsPieOrDonut )
    {
        m_bPointsWereSkipped = false;

        aConsumedOuterRect = ShapeFactory::getRectangleOfShape(*xBoundingShape);
        ::basegfx::B2IRectangle aNewInnerRect( aVDiagram.getCurrentRectangle() );
        if (!rParam.mbUseFixedInnerSize)
            aNewInnerRect = aVDiagram.adjustInnerSize( aConsumedOuterRect );

        for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
        {
            aPlotter->releaseShapes();
        }

        //clear and recreate
        ShapeFactory::removeSubShapes( xSeriesTargetInFrontOfAxis ); //xSeriesTargetBehindAxis is a sub shape of xSeriesTargetInFrontOfAxis and will be removed here
        xSeriesTargetBehindAxis.clear();
        ShapeFactory::removeSubShapes( xTextTargetShapes );

        //set new transformation
        for (auto& rpVCooSys : rVCooSysList)
        {
            auto aMatrix = createTransformationSceneToScreen(aNewInnerRect);
            rpVCooSys->setTransformationSceneToScreen(B3DHomMatrixToHomogenMatrix(aMatrix));
        }

        // - create data series for all charttypes
        for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
        {
            VCoordinateSystem* pVCooSys = SeriesPlotterContainer::getCooSysForPlotter( rVCooSysList, aPlotter.get() );
            if(nDimensionCount==2)
                aPlotter->setTransformationSceneToScreen( pVCooSys->getTransformationSceneToScreen() );
            // Now we can move data labels in case of pie or donut chart!
            aPlotter->setPieLabelsAllowToMove(bIsPieOrDonut);
            aPlotter->createShapes();
            m_bPointsWereSkipped = m_bPointsWereSkipped || aPlotter->PointsWereSkipped();
        }

        for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
            aPlotter->rearrangeLabelToAvoidOverlapIfRequested(rPageSize);
    }

    if (rParam.mbUseFixedInnerSize)
    {
        aUsedOuterRect = awt::Rectangle( aConsumedOuterRect.getMinX(), aConsumedOuterRect.getMinY(), aConsumedOuterRect.getWidth(), aConsumedOuterRect.getHeight() );
    }
    else
        aUsedOuterRect = rParam.maRemainingSpace;

    bool bSnapRectToUsedArea = false;
    for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
    {
        bSnapRectToUsedArea = aPlotter->shouldSnapRectToUsedArea();
        if(bSnapRectToUsedArea)
            break;
    }
    if(bSnapRectToUsedArea)
    {
        if (rParam.mbUseFixedInnerSize)
            m_aResultingDiagramRectangleExcludingAxes = getRectangleOfObject( u"PlotAreaExcludingAxes"_ustr );
        else
        {
            ::basegfx::B2IRectangle aConsumedInnerRect = aVDiagram.getCurrentRectangle();
            m_aResultingDiagramRectangleExcludingAxes = BaseGFXHelper::toAwtRectangle(aConsumedInnerRect);
        }
    }
    else
    {
        if (rParam.mbUseFixedInnerSize)
            m_aResultingDiagramRectangleExcludingAxes = rParam.maRemainingSpace;
        else
        {
            ::basegfx::B2IRectangle aConsumedInnerRect = aVDiagram.getCurrentRectangle();
            m_aResultingDiagramRectangleExcludingAxes = BaseGFXHelper::toAwtRectangle(aConsumedInnerRect);
        }
    }

    if (rParam.mxMarkHandles.is())
    {
        awt::Point aPos(rParam.maRemainingSpace.X, rParam.maRemainingSpace.Y);
        awt::Size  aSize(rParam.maRemainingSpace.Width, rParam.maRemainingSpace.Height);

        bool bPosSizeExcludeAxesProperty = true;
        xDiagram->getPropertyValue(u"PosSizeExcludeAxes"_ustr) >>= bPosSizeExcludeAxesProperty;
        if (rParam.mbUseFixedInnerSize || bPosSizeExcludeAxesProperty)
        {
            aPos = awt::Point( m_aResultingDiagramRectangleExcludingAxes.X, m_aResultingDiagramRectangleExcludingAxes.Y );
            aSize = awt::Size( m_aResultingDiagramRectangleExcludingAxes.Width, m_aResultingDiagramRectangleExcludingAxes.Height );
        }
        rParam.mxMarkHandles->setPosition(aPos);
        rParam.mxMarkHandles->setSize(aSize);
    }

    return aUsedOuterRect;
}

bool ChartView::getExplicitValuesForAxis(
                     rtl::Reference< Axis > xAxis
                     , ExplicitScaleData&  rExplicitScale
                     , ExplicitIncrementData& rExplicitIncrement )
{
    SolarMutexGuard aSolarGuard;

    impl_updateView();

    if(!xAxis.is())
        return false;

    rtl::Reference< BaseCoordinateSystem > xCooSys = AxisHelper::getCoordinateSystemOfAxis(xAxis, mrChartModel.getFirstChartDiagram() );
    const VCoordinateSystem* pVCooSys = SeriesPlotterContainer::findInCooSysList(m_aVCooSysList, xCooSys);
    if(!pVCooSys)
        return false;

    sal_Int32 nDimensionIndex=-1;
    sal_Int32 nAxisIndex=-1;
    if( !AxisHelper::getIndicesForAxis( xAxis, xCooSys, nDimensionIndex, nAxisIndex ) )
        return false;

    rExplicitScale = pVCooSys->getExplicitScale(nDimensionIndex,nAxisIndex);
    rExplicitIncrement = pVCooSys->getExplicitIncrement(nDimensionIndex,nAxisIndex);
    if( !rExplicitScale.m_bShiftedCategoryPosition )
        return true;

    //remove 'one' from max
    if( rExplicitScale.AxisType == css::chart2::AxisType::DATE )
    {
        Date aMaxDate(rExplicitScale.NullDate); aMaxDate.AddDays(::rtl::math::approxFloor(rExplicitScale.Maximum));
        //for explicit scales with shifted categories we need one interval more
        switch( rExplicitScale.TimeResolution )
        {
        case css::chart::TimeUnit::DAY:
            --aMaxDate;
            break;
        case css::chart::TimeUnit::MONTH:
            aMaxDate = DateHelper::GetDateSomeMonthsAway(aMaxDate,-1);
            break;
        case css::chart::TimeUnit::YEAR:
            aMaxDate = DateHelper::GetDateSomeYearsAway(aMaxDate,-1);
            break;
        }
        rExplicitScale.Maximum = aMaxDate - rExplicitScale.NullDate;
    }
    else if( rExplicitScale.AxisType == css::chart2::AxisType::CATEGORY )
        rExplicitScale.Maximum -= 1.0;
    else if( rExplicitScale.AxisType == css::chart2::AxisType::SERIES )
        rExplicitScale.Maximum -= 1.0;
    return true;
}

SdrPage* ChartView::getSdrPage()
{
    if(m_xDrawPage)
        return m_xDrawPage->GetSdrPage();

    return nullptr;
}

rtl::Reference< SvxShape > ChartView::getShapeForCID( const OUString& rObjectCID )
{
    SolarMutexGuard aSolarGuard;
    SdrObject* pObj = DrawModelWrapper::getNamedSdrObject( rObjectCID, this->getSdrPage() );
    if( !pObj )
        return nullptr;

    uno::Reference< drawing::XShape > xShape = pObj->getUnoShape();
    rtl::Reference<SvxShape> xShape2 = dynamic_cast<SvxShape*>(xShape.get());
    assert(xShape2 || !xShape);
    return xShape2;
}

awt::Rectangle ChartView::getDiagramRectangleExcludingAxes()
{
    impl_updateView();
    return m_aResultingDiagramRectangleExcludingAxes;
}

awt::Rectangle ChartView::getRectangleOfObject( const OUString& rObjectCID, bool bSnapRect )
{
    impl_updateView();

    awt::Rectangle aRet;
    rtl::Reference< SvxShape > xShape = getShapeForCID(rObjectCID);
    if(xShape.is())
    {
        //special handling for axis for old api:
        //same special handling for diagram
        ObjectType eObjectType( ObjectIdentifier::getObjectType( rObjectCID ) );
        if( eObjectType == OBJECTTYPE_AXIS || eObjectType == OBJECTTYPE_DIAGRAM )
        {
            SolarMutexGuard aSolarGuard;
            SdrObject* pRootSdrObject = xShape->GetSdrObject();
            if( pRootSdrObject )
            {
                SdrObjList* pRootList = pRootSdrObject->GetSubList();
                if( pRootList )
                {
                    OUString aShapeName = u"MarkHandles"_ustr;
                    if( eObjectType == OBJECTTYPE_DIAGRAM )
                        aShapeName = "PlotAreaIncludingAxes";
                    SdrObject* pShape = DrawModelWrapper::getNamedSdrObject( aShapeName, pRootList );
                    if( pShape )
                    {
                        xShape = dynamic_cast<SvxShape*>(pShape->getUnoShape().get());
                        assert(xShape);
                    }
                }
            }
        }

        awt::Size aSize( xShape->getSize() );
        awt::Point aPoint( xShape->getPosition() );
        aRet = awt::Rectangle( aPoint.X, aPoint.Y, aSize.Width, aSize.Height );
        if( bSnapRect )
        {
            //for rotated objects the shape size and position differs from the visible rectangle
            SdrObject* pSdrObject = xShape->GetSdrObject();
            if( pSdrObject )
            {
                tools::Rectangle aSnapRect( pSdrObject->GetSnapRect() );
                aRet = awt::Rectangle(aSnapRect.Left(),aSnapRect.Top(),aSnapRect.GetWidth(),aSnapRect.GetHeight());
            }
        }
    }
    return aRet;
}

std::shared_ptr< DrawModelWrapper > ChartView::getDrawModelWrapper()
{
    return m_pDrawModelWrapper;
}

namespace
{

constexpr double constPageLayoutDistancePercentage = 0.02;
constexpr sal_Int32 constPageLayoutFixedDistance = 350;

bool getAvailablePosAndSizeForDiagram(
    CreateShapeParam2D& rParam, const awt::Size & rPageSize, rtl::Reference<Diagram> const& xDiagram)
{
    uno::Reference<beans::XPropertySet> const& xProp(xDiagram);
    rParam.mbUseFixedInnerSize = false;

    //@todo: we need a size dependent on the axis labels
    rtl::Reference<ChartType> xChartType;
    if (xDiagram)
        xChartType = xDiagram->getChartTypeByIndex(0);

    sal_Int32 nXDistance = sal_Int32(rPageSize.Width * constPageLayoutDistancePercentage);
    sal_Int32 nYDistance = sal_Int32(rPageSize.Height * constPageLayoutDistancePercentage);

    // Only pie chart uses fixed size margins
    if (xChartType.is() && xChartType->getChartType() == CHART2_SERVICE_NAME_CHARTTYPE_PIE)
    {
        nXDistance = constPageLayoutFixedDistance;
        nYDistance = constPageLayoutFixedDistance;
    }

    rParam.maRemainingSpace.X += nXDistance;
    rParam.maRemainingSpace.Width -= 2*nXDistance;
    rParam.maRemainingSpace.Y += nYDistance;
    rParam.maRemainingSpace.Height -= 2*nYDistance;

    bool bPosSizeExcludeAxes = false;
    if( xProp.is() )
        xProp->getPropertyValue( u"PosSizeExcludeAxes"_ustr ) >>= bPosSizeExcludeAxes;

    //size:
    css::chart2::RelativeSize aRelativeSize;
    if( xProp.is() && (xProp->getPropertyValue( u"RelativeSize"_ustr )>>=aRelativeSize) )
    {
        rParam.maRemainingSpace.Height = static_cast<sal_Int32>(aRelativeSize.Secondary*rPageSize.Height);
        rParam.maRemainingSpace.Width = static_cast<sal_Int32>(aRelativeSize.Primary*rPageSize.Width);
        rParam.mbUseFixedInnerSize = bPosSizeExcludeAxes;
    }

    if (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0)
        return false;

    //position:
    chart2::RelativePosition aRelativePosition;
    if( xProp.is() && (xProp->getPropertyValue( u"RelativePosition"_ustr )>>=aRelativePosition) )
    {
        //@todo decide whether x is primary or secondary

        //the coordinates re relative to the page
        double fX = aRelativePosition.Primary*rPageSize.Width;
        double fY = aRelativePosition.Secondary*rPageSize.Height;

        awt::Point aPos = RelativePositionHelper::getUpperLeftCornerOfAnchoredObject(
            awt::Point(static_cast<sal_Int32>(fX),static_cast<sal_Int32>(fY)),
            awt::Size(rParam.maRemainingSpace.Width, rParam.maRemainingSpace.Height),
            aRelativePosition.Anchor);

        rParam.maRemainingSpace.X = aPos.X;
        rParam.maRemainingSpace.Y = aPos.Y;

        rParam.mbUseFixedInnerSize = bPosSizeExcludeAxes;
    }

    //ensure that the diagram does not lap out right side or out of bottom
    if (rParam.maRemainingSpace.Y + rParam.maRemainingSpace.Height > rPageSize.Height)
        rParam.maRemainingSpace.Height = rPageSize.Height - rParam.maRemainingSpace.Y;

    if (rParam.maRemainingSpace.X + rParam.maRemainingSpace.Width > rPageSize.Width)
        rParam.maRemainingSpace.Width = rPageSize.Width - rParam.maRemainingSpace.X;

    return true;
}

enum class TitleAlignment { ALIGN_LEFT, ALIGN_TOP, ALIGN_RIGHT, ALIGN_BOTTOM, ALIGN_Z };

void changePositionOfAxisTitle( VTitle* pVTitle, TitleAlignment eAlignment
                               , awt::Rectangle const & rDiagramPlusAxesRect, const awt::Size & rPageSize )
{
    if(!pVTitle)
        return;

    awt::Point aNewPosition(0,0);
    awt::Size aTitleSize = pVTitle->getFinalSize();
    sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height * constPageLayoutDistancePercentage);
    sal_Int32 nXDistance = static_cast<sal_Int32>(rPageSize.Width * constPageLayoutDistancePercentage);
    switch (eAlignment)
    {
    case TitleAlignment::ALIGN_TOP:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width/2
                                    , rDiagramPlusAxesRect.Y - aTitleSize.Height/2  - nYDistance );
        break;
    case TitleAlignment::ALIGN_BOTTOM:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width/2
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height + aTitleSize.Height/2  + nYDistance );
        break;
    case TitleAlignment::ALIGN_LEFT:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X - aTitleSize.Width/2 - nXDistance
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height/2 );
        break;
    case TitleAlignment::ALIGN_RIGHT:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width + aTitleSize.Width/2 + nXDistance
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height/2 );
        break;
    case TitleAlignment::ALIGN_Z:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width + aTitleSize.Width/2 + nXDistance
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height - aTitleSize.Height/2 );
       break;
    }

    sal_Int32 nMaxY = rPageSize.Height - aTitleSize.Height/2;
    sal_Int32 nMaxX = rPageSize.Width - aTitleSize.Width/2;
    sal_Int32 nMinX = aTitleSize.Width/2;
    sal_Int32 nMinY = aTitleSize.Height/2;
    if( aNewPosition.Y > nMaxY )
        aNewPosition.Y = nMaxY;
    if( aNewPosition.X > nMaxX )
        aNewPosition.X = nMaxX;
    if( aNewPosition.Y < nMinY )
        aNewPosition.Y = nMinY;
    if( aNewPosition.X < nMinX )
        aNewPosition.X = nMinX;

    pVTitle->changePosition( aNewPosition );
}

std::shared_ptr<VTitle> lcl_createTitle( TitleHelper::eTitleType eType
                , const rtl::Reference<SvxShapeGroupAnyD>& xPageShapes
                , ChartModel& rModel
                , awt::Rectangle& rRemainingSpace
                , const awt::Size & rPageSize
                , TitleAlignment eAlignment
                , bool& rbAutoPosition )
{
    std::shared_ptr<VTitle> apVTitle;

    // #i109336# Improve auto positioning in chart
    double fPercentage = constPageLayoutDistancePercentage;
    sal_Int32 nXDistance = static_cast< sal_Int32 >( rPageSize.Width * fPercentage );
    sal_Int32 nYDistance = static_cast< sal_Int32 >( rPageSize.Height * fPercentage );
    if ( eType == TitleHelper::MAIN_TITLE )
    {
        nYDistance += 135; // 1/100 mm
    }
    else if ( eType == TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION )
    {
        nYDistance = 420; // 1/100 mm
    }
    else if ( eType == TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION )
    {
        nXDistance = 450; // 1/100 mm
    }

    rtl::Reference< Title > xTitle( TitleHelper::getTitle( eType, rModel ) );
    OUString aCompleteString = TitleHelper::getCompleteString(xTitle);
    if (aCompleteString.isEmpty() || !VTitle::isVisible(xTitle))
        return apVTitle;

    //create title
    awt::Size aTextMaxWidth(rPageSize.Width, rPageSize.Height);
    bool bYAxisTitle = false;
    if (eType == TitleHelper::MAIN_TITLE || eType == TitleHelper::SUB_TITLE)
    {
        aTextMaxWidth.Width = static_cast<sal_Int32>(rPageSize.Width * 0.8);
        aTextMaxWidth.Height = static_cast<sal_Int32>(rPageSize.Height * 0.5);
    }
    else if (eType == TitleHelper::X_AXIS_TITLE || eType == TitleHelper::SECONDARY_X_AXIS_TITLE
             || eType == TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION)
    {
        aTextMaxWidth.Width = static_cast<sal_Int32>(rPageSize.Width * 0.8);
        aTextMaxWidth.Height = static_cast<sal_Int32>(rPageSize.Height * 0.2);
    }
    else if (eType == TitleHelper::Y_AXIS_TITLE || eType == TitleHelper::SECONDARY_Y_AXIS_TITLE
             || eType == TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION)
    {
        aTextMaxWidth.Width = static_cast<sal_Int32>(rPageSize.Width * 0.2);
        aTextMaxWidth.Height = static_cast<sal_Int32>(rPageSize.Height * 0.8);
        bYAxisTitle = true;
    }
    apVTitle = std::make_shared<VTitle>(xTitle);
    OUString aCID = ObjectIdentifier::createClassifiedIdentifierForObject(xTitle, &rModel);
    apVTitle->init(xPageShapes, aCID);
    apVTitle->createShapes(awt::Point(0, 0), rPageSize, aTextMaxWidth, bYAxisTitle);
    awt::Size aTitleUnrotatedSize = apVTitle->getUnrotatedSize();
    awt::Size aTitleSize = apVTitle->getFinalSize();

    //position
    rbAutoPosition = true;
    awt::Point aNewPosition(0,0);
    chart2::RelativePosition aRelativePosition;
    if (xTitle.is() && (xTitle->getPropertyValue(u"RelativePosition"_ustr) >>= aRelativePosition))
    {
        rbAutoPosition = false;

        //@todo decide whether x is primary or secondary
        double fX = aRelativePosition.Primary*rPageSize.Width;
        double fY = aRelativePosition.Secondary*rPageSize.Height;

        double fAnglePi = apVTitle->getRotationAnglePi();
        aNewPosition = RelativePositionHelper::getCenterOfAnchoredObject(
                awt::Point(static_cast<sal_Int32>(fX),static_cast<sal_Int32>(fY))
                , aTitleUnrotatedSize, aRelativePosition.Anchor, fAnglePi );
    }
    else //auto position
    {
        switch( eAlignment )
        {
        case TitleAlignment::ALIGN_TOP:
            aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width/2
                                     , rRemainingSpace.Y + aTitleSize.Height/2 + nYDistance );
            break;
        case TitleAlignment::ALIGN_BOTTOM:
            aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width/2
                                     , rRemainingSpace.Y + rRemainingSpace.Height - aTitleSize.Height/2 - nYDistance );
            break;
        case TitleAlignment::ALIGN_LEFT:
            aNewPosition = awt::Point( rRemainingSpace.X + aTitleSize.Width/2 + nXDistance
                                     , rRemainingSpace.Y + rRemainingSpace.Height/2 );
            break;
        case TitleAlignment::ALIGN_RIGHT:
            aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width - aTitleSize.Width/2 - nXDistance
                                     , rRemainingSpace.Y + rRemainingSpace.Height/2 );
            break;
        case TitleAlignment::ALIGN_Z:
            break;

        }
    }
    apVTitle->changePosition( aNewPosition );

    //remaining space
    switch( eAlignment )
    {
        case TitleAlignment::ALIGN_TOP:
            // Push the remaining space down from top.
            rRemainingSpace.Y += ( aTitleSize.Height + nYDistance );
            rRemainingSpace.Height -= ( aTitleSize.Height + nYDistance );
            break;
        case TitleAlignment::ALIGN_BOTTOM:
            // Push the remaining space up from bottom.
            rRemainingSpace.Height -= ( aTitleSize.Height + nYDistance );
            break;
        case TitleAlignment::ALIGN_LEFT:
            // Push the remaining space to the right from left edge.
            rRemainingSpace.X += ( aTitleSize.Width + nXDistance );
            rRemainingSpace.Width -= ( aTitleSize.Width + nXDistance );
            break;
        case TitleAlignment::ALIGN_RIGHT:
            // Push the remaining space to the left from right edge.
            rRemainingSpace.Width -= ( aTitleSize.Width + nXDistance );
            break;
        case TitleAlignment::ALIGN_Z:
            break;
    }

    return apVTitle;
}

bool lcl_createLegend( const rtl::Reference< Legend > & xLegend
                   , const rtl::Reference<SvxShapeGroupAnyD>& xPageShapes
                   , const uno::Reference< uno::XComponentContext > & xContext
                   , awt::Rectangle & rRemainingSpace
                   , const awt::Size & rPageSize
                   , ChartModel& rModel
                   , std::vector< LegendEntryProvider* >&& rLegendEntryProviderList
                   , sal_Int16 nDefaultWritingMode )
{
    if (!VLegend::isVisible(xLegend))
        return false;

    awt::Size rDefaultLegendSize;
    VLegend aVLegend( xLegend, xContext, std::move(rLegendEntryProviderList),
            xPageShapes, rModel);
    aVLegend.setDefaultWritingMode( nDefaultWritingMode );
    aVLegend.createShapes( awt::Size( rRemainingSpace.Width, rRemainingSpace.Height ),
                           rPageSize, rDefaultLegendSize );
    aVLegend.changePosition( rRemainingSpace, rPageSize, rDefaultLegendSize );
    return true;
}

void lcl_createButtons(const rtl::Reference<SvxShapeGroupAnyD>& xPageShapes,
                       ChartModel& rModel,
                       awt::Rectangle& rRemainingSpace)
{
    uno::Reference<chart2::data::XPivotTableDataProvider> xPivotTableDataProvider(rModel.getDataProvider(), uno::UNO_QUERY);
    if (!xPivotTableDataProvider.is())
        return;

    uno::Reference<beans::XPropertySet> xModelPage(rModel.getPageBackground());

    awt::Size aSize(4000, 700); // size of the button

    tools::Long x = 0;

    if (xPivotTableDataProvider->getPageFields().hasElements())
    {
        x = 0;

        const css::uno::Sequence<chart2::data::PivotTableFieldEntry> aPivotFieldEntries = xPivotTableDataProvider->getPageFields();
        for (css::chart2::data::PivotTableFieldEntry const & rPageFieldEntry : aPivotFieldEntries)
        {
            VButton aButton;
            aButton.init(xPageShapes);
            awt::Point aNewPosition(rRemainingSpace.X + x + 100, rRemainingSpace.Y + 100);
            sal_Int32 nDimensionIndex = rPageFieldEntry.DimensionIndex;
            OUString aFieldOutputDescription = xPivotTableDataProvider->getFieldOutputDescription(nDimensionIndex);
            aButton.setLabel(rPageFieldEntry.Name + " | " + aFieldOutputDescription);
            aButton.setCID("FieldButton.Page." + OUString::number(nDimensionIndex));
            aButton.setPosition(aNewPosition);
            aButton.setSize(aSize);
            if (rPageFieldEntry.HasHiddenMembers)
                aButton.setArrowColor(Color(0x0000FF));

            aButton.createShapes(xModelPage);
            x += aSize.Width + 100;
        }
        rRemainingSpace.Y += (aSize.Height + 100 + 100);
        rRemainingSpace.Height -= (aSize.Height + 100 + 100);
    }

    aSize = awt::Size(3000, 700); // size of the button

    if (!xPivotTableDataProvider->getRowFields().hasElements())
        return;

    x = 200;
    const css::uno::Sequence<chart2::data::PivotTableFieldEntry> aPivotFieldEntries = xPivotTableDataProvider->getRowFields();
    for (css::chart2::data::PivotTableFieldEntry const & rRowFieldEntry : aPivotFieldEntries)
    {
        VButton aButton;
        aButton.init(xPageShapes);
        awt::Point aNewPosition(rRemainingSpace.X + x + 100,
                                rRemainingSpace.Y + rRemainingSpace.Height - aSize.Height - 100);
        aButton.setLabel(rRowFieldEntry.Name);
        aButton.setCID("FieldButton.Row." + OUString::number(rRowFieldEntry.DimensionIndex));
        aButton.setPosition(aNewPosition);
        aButton.setSize(aSize);
        if ( rRowFieldEntry.Name == "Data" )
        {
            aButton.setBGColor( Color(0x00F6F6F6) );
            aButton.showArrow( false );
        }
        else if (rRowFieldEntry.HasHiddenMembers)
            aButton.setArrowColor(Color(0x0000FF));
        aButton.createShapes(xModelPage);
        x += aSize.Width + 100;
    }
    rRemainingSpace.Height -= (aSize.Height + 100 + 100);
}

void formatPage(
      ChartModel& rChartModel
    , const awt::Size& rPageSize
    , const rtl::Reference<SvxShapeGroupAnyD>& xTarget
    )
{
    try
    {
        uno::Reference< beans::XPropertySet > xModelPage( rChartModel.getPageBackground());
        if( ! xModelPage.is())
            return;

        //format page
        tPropertyNameValueMap aNameValueMap;
        PropertyMapper::getValueMap( aNameValueMap, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xModelPage );

        OUString aCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, u"" ) );
        aNameValueMap.emplace( "Name", uno::Any( aCID ) ); //CID OUString

        tNameSequence aNames;
        tAnySequence aValues;
        PropertyMapper::getMultiPropertyListsFromValueMap( aNames, aValues, aNameValueMap );

        ShapeFactory::createRectangle(
            xTarget, rPageSize, awt::Point(0, 0), aNames, aValues);
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2" );
    }
}

void lcl_removeEmptyGroupShapes( const SdrObject& rParent )
{
    SdrObjList* pObjList = rParent.getChildrenOfSdrObject();
    if (!pObjList || pObjList->GetObjCount() == 0)
        return;

    //iterate from back!
    for(auto nIdx = static_cast<sal_Int32>(pObjList->GetObjCount() - 1); nIdx >= 0; --nIdx)
    {
        SdrObject* pChildSdrObject = pObjList->GetObj(nIdx);
        SdrObjList* pChildObjList = pChildSdrObject->getChildrenOfSdrObject();
        if (!pChildObjList)
            continue;
        if (pChildObjList->GetObjCount() == 0)
        {
            //remove empty group shape
            pObjList->NbcRemoveObject(nIdx);
        }
        else
            lcl_removeEmptyGroupShapes(*pChildSdrObject);
    }
}

}

void ChartView::impl_refreshAddIn()
{
    if( !m_bRefreshAddIn )
        return;

    uno::Reference< beans::XPropertySet > xProp( static_cast< ::cppu::OWeakObject* >( &mrChartModel ), uno::UNO_QUERY );
    if( !xProp.is())
        return;

    try
    {
        uno::Reference< util::XRefreshable > xAddIn;
        xProp->getPropertyValue( u"AddIn"_ustr ) >>= xAddIn;
        if( xAddIn.is() )
        {
            bool bRefreshAddInAllowed = true;
            xProp->getPropertyValue( u"RefreshAddInAllowed"_ustr ) >>= bRefreshAddInAllowed;
            if( bRefreshAddInAllowed )
                xAddIn->refresh();
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartView::createShapes()
{
    SolarMutexGuard aSolarGuard;

    std::unique_lock aTimedGuard(maTimeMutex);
    if(mrChartModel.isTimeBased())
    {
        maTimeBased.bTimeBased = true;
    }

    //make sure add-in is refreshed after creating the shapes
    const ::comphelper::ScopeGuard aGuard( [this]() { this->impl_refreshAddIn(); } );

    m_aResultingDiagramRectangleExcludingAxes = awt::Rectangle(0,0,0,0);
    impl_deleteCoordinateSystems();
    if( m_pDrawModelWrapper )
    {
        // #i12587# support for shapes in chart
        m_pDrawModelWrapper->getSdrModel().EnableUndo( false );
        m_pDrawModelWrapper->clearMainDrawPage();
    }

    lcl_setDefaultWritingMode( m_pDrawModelWrapper, mrChartModel );

    awt::Size aPageSize = mrChartModel.getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );

    if(!mxRootShape.is())
        mxRootShape = ShapeFactory::getOrCreateChartRootShape( m_xDrawPage );

    SdrPage* pPage = getSdrPage();

    if (pPage) //it is necessary to use the implementation here as the uno page does not provide a propertyset
    {
        pPage->SetSize(Size(aPageSize.Width,aPageSize.Height));
    }
    else
    {
        OSL_FAIL("could not set page size correctly");
    }
    ShapeFactory::setPageSize(mxRootShape, aPageSize);

    createShapes2D(aPageSize);

    // #i12587# support for shapes in chart
    if ( m_pDrawModelWrapper )
    {
        m_pDrawModelWrapper->getSdrModel().EnableUndo( true );
    }

    if(maTimeBased.bTimeBased)
    {
        maTimeBased.nFrame++;
    }
}

// util::XEventListener (base of XCloseListener)
void SAL_CALL ChartView::disposing( const lang::EventObject& /* rSource */ )
{
}

namespace
{
// Disables setting the chart's modified state, as well as its parent's (if exists).
// Painting a chart must not set these states.
struct ChartModelDisableSetModified
{
    ChartModel& mrChartModel;
    SfxObjectShell* mpParentShell;
    bool mbWasUnmodified;
    ChartModelDisableSetModified(ChartModel& rChartModel)
        : mrChartModel(rChartModel)
        , mpParentShell(SfxObjectShell::GetShellFromComponent(rChartModel.getParent()))
        , mbWasUnmodified(!rChartModel.isModified())
    {
        if (mpParentShell && mpParentShell->IsEnableSetModified())
            mpParentShell->EnableSetModified(false);
        else
            mpParentShell = nullptr;
    }
    ~ChartModelDisableSetModified()
    {
        if (mbWasUnmodified && mrChartModel.isModified())
            mrChartModel.setModified(false);
        if (mpParentShell)
            mpParentShell->EnableSetModified(true);
    }
};
}

void ChartView::impl_updateView( bool bCheckLockedCtrler )
{
    if( !m_pDrawModelWrapper )
        return;

    // #i12587# support for shapes in chart
    if ( m_bSdrViewIsInEditMode )
    {
        return;
    }

    if (bCheckLockedCtrler && mrChartModel.hasControllersLocked())
        return;

    if( !m_bViewDirty || m_bInViewUpdate )
        return;

    m_bInViewUpdate = true;
    //bool bOldRefreshAddIn = m_bRefreshAddIn;
    //m_bRefreshAddIn = false;
    try
    {
        impl_notifyModeChangeListener(u"invalid"_ustr);

        //prepare draw model
        {
            SolarMutexGuard aSolarGuard;
            m_pDrawModelWrapper->lockControllers();
        }

        // Rendering the chart must not set its (or its parent) modified status
        ChartModelDisableSetModified dontSetModified(mrChartModel);

        //create chart view
        {
            m_bViewDirty = false;
            m_bViewUpdatePending = false;
            createShapes();

            if( m_bViewDirty )
            {
                //avoid recursions due to add-in
                m_bRefreshAddIn = false;
                m_bViewDirty = false;
                m_bViewUpdatePending = false;
                //delete old chart view
                createShapes();
                m_bRefreshAddIn = true;
            }
        }

        m_bViewDirty = m_bViewUpdatePending;
        m_bViewUpdatePending = false;
        m_bInViewUpdate = false;
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2" );
        m_bViewDirty = m_bViewUpdatePending;
        m_bViewUpdatePending = false;
        m_bInViewUpdate = false;
    }

    {
        SolarMutexGuard aSolarGuard;
        m_pDrawModelWrapper->unlockControllers();
    }

    impl_notifyModeChangeListener(u"valid"_ustr);

    //m_bRefreshAddIn = bOldRefreshAddIn;
}

// ____ XModifyListener ____
void SAL_CALL ChartView::modified( const lang::EventObject& /* aEvent */ )
{
    m_bViewDirty = true;
    if( m_bInViewUpdate )
        m_bViewUpdatePending = true;

    impl_notifyModeChangeListener(u"dirty"_ustr);
}

//SfxListener
void ChartView::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    //#i77362 change notification for changes on additional shapes are missing
    if( m_bInViewUpdate )
        return;

    // #i12587# support for shapes in chart
    if ( m_bSdrViewIsInEditMode )
    {
        uno::Reference< view::XSelectionSupplier > xSelectionSupplier( mrChartModel.getCurrentController(), uno::UNO_QUERY );
        if ( xSelectionSupplier.is() )
        {
            OUString aSelObjCID;
            uno::Any aSelObj( xSelectionSupplier->getSelection() );
            aSelObj >>= aSelObjCID;
            if ( !aSelObjCID.isEmpty() )
            {
                return;
            }
        }
    }

    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;
    const SdrHint* pSdrHint = static_cast< const SdrHint* >(&rHint);

    bool bShapeChanged = false;
    switch( pSdrHint->GetKind() )
    {
         case SdrHintKind::ObjectChange:
            bShapeChanged = true;
            break;
        case SdrHintKind::ObjectInserted:
            bShapeChanged = true;
            break;
        case SdrHintKind::ObjectRemoved:
            bShapeChanged = true;
            break;
        case SdrHintKind::ModelCleared:
            bShapeChanged = true;
            break;
        case SdrHintKind::EndEdit:
            bShapeChanged = true;
            break;
        default:
            break;
    }

    if(bShapeChanged)
    {
        //#i76053# do not send view modified notifications for changes on the hidden page which contains e.g. the symbols for the dialogs
        if( ChartView::getSdrPage() != pSdrHint->GetPage() )
            bShapeChanged=false;
    }

    if(!bShapeChanged)
        return;

    mrChartModel.setModified(true);
}

void ChartView::impl_notifyModeChangeListener( const OUString& rNewMode )
{
    try
    {
        std::unique_lock g(m_aMutex);
        if( m_aModeChangeListeners.getLength(g) )
        {
            util::ModeChangeEvent aEvent( static_cast< uno::XWeak* >( this ), rNewMode );
            m_aModeChangeListeners.notifyEach( g, &css::util::XModeChangeListener::modeChanged, aEvent);
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XModeChangeBroadcaster ____

void SAL_CALL ChartView::addModeChangeListener( const uno::Reference< util::XModeChangeListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aModeChangeListeners.addInterface(g, xListener );
}
void SAL_CALL ChartView::removeModeChangeListener( const uno::Reference< util::XModeChangeListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aModeChangeListeners.removeInterface(g, xListener );
}
void SAL_CALL ChartView::addModeChangeApproveListener( const uno::Reference< util::XModeChangeApproveListener >& /* _rxListener */ )
{

}
void SAL_CALL ChartView::removeModeChangeApproveListener( const uno::Reference< util::XModeChangeApproveListener >& /* _rxListener */ )
{

}

// ____ XUpdatable ____
void SAL_CALL ChartView::update()
{
    impl_updateView();

    //#i100778# migrate all imported or old documents to a plot area sizing exclusive axes (in case the save settings allow for this):
    //Although in general it is a bad idea to change the model from within the view this is exceptionally the best place to do this special conversion.
    //When a view update is requested (what happens for creating the metafile or displaying
    //the chart in edit mode or printing) it is most likely that all necessary information is available - like the underlying spreadsheet data for example.
    //Those data are important for the correct axis label sizes which are needed during conversion.
    if( DiagramHelper::switchDiagramPositioningToExcludingPositioning( mrChartModel, true, false ) )
        impl_updateView();
}

void SAL_CALL ChartView::updateSoft()
{
    update();
}

void SAL_CALL ChartView::updateHard()
{
    impl_updateView(false);
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL ChartView::getPropertySetInfo()
{
    OSL_FAIL("not implemented");
    return nullptr;
}

void SAL_CALL ChartView::setPropertyValue( const OUString& rPropertyName
                                                     , const Any& rValue )
{
    if( rPropertyName == "Resolution" )
    {
        awt::Size aNewResolution;
        if( ! (rValue >>= aNewResolution) )
            throw lang::IllegalArgumentException( u"Property 'Resolution' requires value of type awt::Size"_ustr, nullptr, 0 );

        if( m_aPageResolution.Width!=aNewResolution.Width || m_aPageResolution.Height!=aNewResolution.Height )
        {
            //set modified only when the new resolution is higher and points were skipped before
            bool bSetModified = m_bPointsWereSkipped && (m_aPageResolution.Width<aNewResolution.Width || m_aPageResolution.Height<aNewResolution.Height);

            m_aPageResolution = aNewResolution;

            if( bSetModified )
                this->modified( lang::EventObject(  static_cast< uno::XWeak* >( this )  ) );
        }
    }
    else if( rPropertyName == "ZoomFactors" )
    {
        //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
        uno::Sequence< beans::PropertyValue > aZoomFactors;
        if( ! (rValue >>= aZoomFactors) )
            throw lang::IllegalArgumentException( u"Property 'ZoomFactors' requires value of type Sequence< PropertyValue >"_ustr, nullptr, 0 );

        for (auto& propval : aZoomFactors)
        {
            if (propval.Name == "ScaleXNumerator")
                propval.Value >>= m_nScaleXNumerator;
            else if (propval.Name == "ScaleXDenominator")
                propval.Value >>= m_nScaleXDenominator;
            else if (propval.Name == "ScaleYNumerator")
                propval.Value >>= m_nScaleYNumerator;
            else if (propval.Name == "ScaleYDenominator")
                propval.Value >>= m_nScaleYDenominator;
        }
    }
    else if( rPropertyName == "SdrViewIsInEditMode" )
    {
        //#i77362 change notification for changes on additional shapes are missing
        if( ! (rValue >>= m_bSdrViewIsInEditMode) )
            throw lang::IllegalArgumentException( u"Property 'SdrViewIsInEditMode' requires value of type sal_Bool"_ustr, nullptr, 0 );
    }
    else
        throw beans::UnknownPropertyException( "unknown property was tried to set to chart wizard " + rPropertyName, nullptr );
}

Any SAL_CALL ChartView::getPropertyValue( const OUString& rPropertyName )
{
    if( rPropertyName != "Resolution" )
        throw beans::UnknownPropertyException( "unknown property was tried to get from chart wizard " + rPropertyName, nullptr );

    return Any(m_aPageResolution);
}

void SAL_CALL ChartView::addPropertyChangeListener(
    const OUString& /* aPropertyName */, const Reference< beans::XPropertyChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL ChartView::removePropertyChangeListener(
    const OUString& /* aPropertyName */, const Reference< beans::XPropertyChangeListener >& /* aListener */ )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ChartView::addVetoableChangeListener( const OUString& /* PropertyName */, const Reference< beans::XVetoableChangeListener >& /* aListener */ )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ChartView::removeVetoableChangeListener( const OUString& /* PropertyName */, const Reference< beans::XVetoableChangeListener >& /* aListener */ )
{
    OSL_FAIL("not implemented");
}

// ____ XMultiServiceFactory ____

Reference< uno::XInterface > ChartView::createInstance( const OUString& aServiceSpecifier )
{
    SolarMutexGuard aSolarGuard;

    SdrModel* pModel = ( m_pDrawModelWrapper ? &m_pDrawModelWrapper->getSdrModel() : nullptr );
    if ( pModel )
    {
        if ( aServiceSpecifier == "com.sun.star.drawing.DashTable" )
        {
            if ( !m_xDashTable.is() )
            {
                m_xDashTable = SvxUnoDashTable_createInstance( pModel );
            }
            return m_xDashTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.GradientTable" )
        {
            if ( !m_xGradientTable.is() )
            {
                m_xGradientTable = SvxUnoGradientTable_createInstance( pModel );
            }
            return m_xGradientTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.HatchTable" )
        {
            if ( !m_xHatchTable.is() )
            {
                m_xHatchTable = SvxUnoHatchTable_createInstance( pModel );
            }
            return m_xHatchTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.BitmapTable" )
        {
            if ( !m_xBitmapTable.is() )
            {
                m_xBitmapTable = SvxUnoBitmapTable_createInstance( pModel );
            }
            return m_xBitmapTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.TransparencyGradientTable" )
        {
            if ( !m_xTransGradientTable.is() )
            {
                m_xTransGradientTable = SvxUnoTransGradientTable_createInstance( pModel );
            }
            return m_xTransGradientTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.MarkerTable" )
        {
            if ( !m_xMarkerTable.is() )
            {
                m_xMarkerTable = SvxUnoMarkerTable_createInstance( pModel );
            }
            return m_xMarkerTable;
        }
    }

    return nullptr;
}

Reference< uno::XInterface > ChartView::createInstanceWithArguments( const OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
{
    OSL_ENSURE( Arguments.hasElements(), "ChartView::createInstanceWithArguments: arguments are ignored" );
    return createInstance( ServiceSpecifier );
}

uno::Sequence< OUString > ChartView::getAvailableServiceNames()
{
    uno::Sequence< OUString > aServiceNames{ u"com.sun.star.drawing.DashTable"_ustr,
                                             u"com.sun.star.drawing.GradientTable"_ustr,
                                             u"com.sun.star.drawing.HatchTable"_ustr,
                                             u"com.sun.star.drawing.BitmapTable"_ustr,
                                             u"com.sun.star.drawing.TransparencyGradientTable"_ustr,
                                             u"com.sun.star.drawing.MarkerTable"_ustr };

    return aServiceNames;
}

OUString ChartView::dump(OUString const & kind)
{
    if (kind.isEmpty()) {
        return comphelper::dumpXmlToString([this](auto writer) { return dumpAsXml(writer); });
    }

    // kind == "shapes":
#if HAVE_FEATURE_DESKTOP
    // Used for unit tests and in chartcontroller only, no need to drag in this when cross-compiling
    // for non-desktop
    impl_updateView();
    sal_Int32 n = m_xDrawPage->getCount();
    OUStringBuffer aBuffer;
    for(sal_Int32 i = 0; i < n; ++i)
    {
        uno::Reference< drawing::XShapes > xShape(m_xDrawPage->getByIndex(i), uno::UNO_QUERY);
        if(xShape.is())
        {
            OUString aString = XShapeDumper::dump(uno::Reference<drawing::XShapes>(mxRootShape));
            aBuffer.append(aString);
        }
        else
        {
            uno::Reference< drawing::XShape > xSingleShape(m_xDrawPage->getByIndex(i), uno::UNO_QUERY);
            if(!xSingleShape.is())
                continue;
            OUString aString = XShapeDumper::dump(xSingleShape);
            aBuffer.append(aString);
        }
        aBuffer.append("\n\n");
    }

    return aBuffer.makeStringAndClear();
#else
    return OUString();
#endif
}

void ChartView::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ChartView"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    if (m_pDrawModelWrapper)
    {
        m_pDrawModelWrapper->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

void ChartView::setViewDirty()
{
    std::unique_lock aGuard(maTimeMutex);
    m_bViewDirty = true;
}

IMPL_LINK_NOARG(ChartView, UpdateTimeBased, Timer *, void)
{
    setViewDirty();
    update();
}

void ChartView::createShapes2D( const awt::Size& rPageSize )
{
    // todo: it would be nicer to just pass the page m_xDrawPage and format it,
    // but the draw page does not support XPropertySet
    formatPage( mrChartModel, rPageSize, mxRootShape );

    CreateShapeParam2D aParam;
    aParam.maRemainingSpace.X = 0;
    aParam.maRemainingSpace.Y = 0;
    aParam.maRemainingSpace.Width = rPageSize.Width;
    aParam.maRemainingSpace.Height = rPageSize.Height;

    //create the group shape for diagram and axes first to have title and legends on top of it
    rtl::Reference< Diagram > xDiagram( mrChartModel.getFirstChartDiagram() );
    bool bHasRelativeSize = false;
    if( xDiagram.is() && xDiagram->getPropertyValue(u"RelativeSize"_ustr).hasValue() )
        bHasRelativeSize = true;

    OUString aDiagramCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM, OUString::number( 0 ) ) );//todo: other index if more than one diagram is possible
    rtl::Reference<SvxShapeGroup> xDiagramPlusAxesPlusMarkHandlesGroup_Shapes =
            ShapeFactory::createGroup2D(mxRootShape,aDiagramCID);

    aParam.mxMarkHandles = ShapeFactory::createInvisibleRectangle(
        xDiagramPlusAxesPlusMarkHandlesGroup_Shapes, awt::Size(0,0));
    ShapeFactory::setShapeName(aParam.mxMarkHandles, u"MarkHandles"_ustr);

    aParam.mxPlotAreaWithAxes = ShapeFactory::createInvisibleRectangle(
        xDiagramPlusAxesPlusMarkHandlesGroup_Shapes, awt::Size(0, 0));
    ShapeFactory::setShapeName(aParam.mxPlotAreaWithAxes, u"PlotAreaIncludingAxes"_ustr);

    aParam.mxDiagramWithAxesShapes = ShapeFactory::createGroup2D(xDiagramPlusAxesPlusMarkHandlesGroup_Shapes);

    bool bAutoPositionDummy = true;

    // create buttons
    lcl_createButtons(mxRootShape, mrChartModel, aParam.maRemainingSpace);

    lcl_createTitle(
        TitleHelper::MAIN_TITLE, mxRootShape, mrChartModel,
        aParam.maRemainingSpace, rPageSize, TitleAlignment::ALIGN_TOP, bAutoPositionDummy);
    if (!bHasRelativeSize && (aParam.maRemainingSpace.Width <= 0 || aParam.maRemainingSpace.Height <= 0))
        return;

    lcl_createTitle(
        TitleHelper::SUB_TITLE, mxRootShape, mrChartModel,
        aParam.maRemainingSpace, rPageSize, TitleAlignment::ALIGN_TOP, bAutoPositionDummy );
    if (!bHasRelativeSize && (aParam.maRemainingSpace.Width <= 0 || aParam.maRemainingSpace.Height <= 0))
        return;

    aParam.mpSeriesPlotterContainer = std::make_shared<SeriesPlotterContainer>(m_aVCooSysList);
    aParam.mpSeriesPlotterContainer->initializeCooSysAndSeriesPlotter( mrChartModel );
    if(maTimeBased.bTimeBased && maTimeBased.nFrame != 0)
    {
        auto& rSeriesPlotter = aParam.mpSeriesPlotterContainer->getSeriesPlotterList();
        size_t n = rSeriesPlotter.size();
        for(size_t i = 0; i < n; ++i)
        {
            std::vector<VDataSeries*> aAllNewDataSeries = rSeriesPlotter[i]->getAllSeries();
            std::vector< VDataSeries* >& rAllOldDataSeries =
                maTimeBased.m_aDataSeriesList[i];
            size_t m = std::min(aAllNewDataSeries.size(), rAllOldDataSeries.size());
            for(size_t j = 0; j < m; ++j)
            {
                aAllNewDataSeries[j]->setOldTimeBased(
                        rAllOldDataSeries[j], (maTimeBased.nFrame % 60)/60.0);
            }
        }
    }

    lcl_createLegend(
        LegendHelper::getLegend( mrChartModel ), mxRootShape, getComponentContext(),
        aParam.maRemainingSpace, rPageSize, mrChartModel, aParam.mpSeriesPlotterContainer->getLegendEntryProviderList(),
        lcl_getDefaultWritingModeFromPool( m_pDrawModelWrapper ) );

    if (!bHasRelativeSize && (aParam.maRemainingSpace.Width <= 0 || aParam.maRemainingSpace.Height <= 0))
        return;

    if (!createAxisTitleShapes2D(aParam, rPageSize, bHasRelativeSize))
        return;

    bool bDummy = false;
    bool bIsVertical = xDiagram && xDiagram->getVertical(bDummy, bDummy);

    if (getAvailablePosAndSizeForDiagram(aParam, rPageSize, xDiagram))
    {
        awt::Rectangle aUsedOuterRect = impl_createDiagramAndContent(aParam, rPageSize);

        if (aParam.mxPlotAreaWithAxes.is())
        {
            aParam.mxPlotAreaWithAxes->setPosition(awt::Point(aUsedOuterRect.X, aUsedOuterRect.Y));
            aParam.mxPlotAreaWithAxes->setSize(awt::Size(aUsedOuterRect.Width, aUsedOuterRect.Height));
        }

        //correct axis title position
        awt::Rectangle aDiagramPlusAxesRect( aUsedOuterRect );
        if (aParam.mbAutoPosTitleX)
            changePositionOfAxisTitle(aParam.mpVTitleX.get(), TitleAlignment::ALIGN_BOTTOM, aDiagramPlusAxesRect, rPageSize);
        if (aParam.mbAutoPosTitleY)
            changePositionOfAxisTitle(aParam.mpVTitleY.get(), TitleAlignment::ALIGN_LEFT, aDiagramPlusAxesRect, rPageSize);
        if (aParam.mbAutoPosTitleZ)
            changePositionOfAxisTitle(aParam.mpVTitleZ.get(), TitleAlignment::ALIGN_Z, aDiagramPlusAxesRect, rPageSize);
        if (aParam.mbAutoPosSecondTitleX)
            changePositionOfAxisTitle(aParam.mpVTitleSecondX.get(), bIsVertical? TitleAlignment::ALIGN_RIGHT : TitleAlignment::ALIGN_TOP, aDiagramPlusAxesRect, rPageSize);
        if (aParam.mbAutoPosSecondTitleY)
            changePositionOfAxisTitle(aParam.mpVTitleSecondY.get(), bIsVertical? TitleAlignment::ALIGN_TOP : TitleAlignment::ALIGN_RIGHT, aDiagramPlusAxesRect, rPageSize);
    }

    //cleanup: remove all empty group shapes to avoid grey border lines:
    lcl_removeEmptyGroupShapes( *mxRootShape->GetSdrObject() );

    if(maTimeBased.bTimeBased && maTimeBased.nFrame % 60 == 0)
    {
        // create copy of the data for next frame
        auto& rSeriesPlotter = aParam.mpSeriesPlotterContainer->getSeriesPlotterList();
        size_t n = rSeriesPlotter.size();
        maTimeBased.m_aDataSeriesList.clear();
        maTimeBased.m_aDataSeriesList.resize(n);
        for(size_t i = 0; i < n; ++i)
        {
            std::vector<VDataSeries*> aAllNewDataSeries = rSeriesPlotter[i]->getAllSeries();
            std::vector<VDataSeries*>& rAllOldDataSeries = maTimeBased.m_aDataSeriesList[i];
            size_t m = aAllNewDataSeries.size();
            for(size_t j = 0; j < m; ++j)
            {
                rAllOldDataSeries.push_back( aAllNewDataSeries[j]->
                        createCopyForTimeBased() );
            }
        }

        maTimeBased.maTimer.Stop();
    }

    if(maTimeBased.bTimeBased && !maTimeBased.maTimer.IsActive())
    {
        maTimeBased.maTimer.SetTimeout(15);
        maTimeBased.maTimer.SetInvokeHandler(LINK(this, ChartView, UpdateTimeBased));
        maTimeBased.maTimer.Start();
    }
}

bool ChartView::createAxisTitleShapes2D( CreateShapeParam2D& rParam, const css::awt::Size& rPageSize, bool bHasRelativeSize )
{
    rtl::Reference<Diagram> xDiagram = mrChartModel.getFirstChartDiagram();

    rtl::Reference< ChartType > xChartType;
    sal_Int32 nDimension = 0;
    if (xDiagram)
    {
        xChartType = xDiagram->getChartTypeByIndex( 0 );
        nDimension = xDiagram->getDimension();
    }

    if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 0 ) )
        rParam.mpVTitleX = lcl_createTitle( TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, mxRootShape, mrChartModel
                , rParam.maRemainingSpace, rPageSize, TitleAlignment::ALIGN_BOTTOM, rParam.mbAutoPosTitleX );
    if (!bHasRelativeSize && (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0))
        return false;

    if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 1 ) )
        rParam.mpVTitleY = lcl_createTitle( TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, mxRootShape, mrChartModel
                , rParam.maRemainingSpace, rPageSize, TitleAlignment::ALIGN_LEFT, rParam.mbAutoPosTitleY );
    if (!bHasRelativeSize && (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0))
        return false;

    if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 2 ) )
        rParam.mpVTitleZ = lcl_createTitle( TitleHelper::Z_AXIS_TITLE, mxRootShape, mrChartModel
                , rParam.maRemainingSpace, rPageSize, TitleAlignment::ALIGN_RIGHT, rParam.mbAutoPosTitleZ );
    if (!bHasRelativeSize && (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0))
        return false;

    bool bDummy = false;
    bool bIsVertical = xDiagram && xDiagram->getVertical( bDummy, bDummy );

    if( ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimension ) )
        rParam.mpVTitleSecondX = lcl_createTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, mxRootShape, mrChartModel
                , rParam.maRemainingSpace, rPageSize, bIsVertical? TitleAlignment::ALIGN_RIGHT : TitleAlignment::ALIGN_TOP, rParam.mbAutoPosSecondTitleX );
    if (!bHasRelativeSize && (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0))
        return false;

    if( ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimension ) )
        rParam.mpVTitleSecondY = lcl_createTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, mxRootShape, mrChartModel
                , rParam.maRemainingSpace, rPageSize, bIsVertical? TitleAlignment::ALIGN_TOP : TitleAlignment::ALIGN_RIGHT, rParam.mbAutoPosSecondTitleY );
    if (!bHasRelativeSize && (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0))
        return false;

    return true;
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_ChartView_get_implementation(css::uno::XComponentContext *context,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    rtl::Reference<::chart::ChartModel> pChartModel = new ::chart::ChartModel(context);
    return cppu::acquire(new ::chart::ChartView(context, *pChartModel));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
