/*************************************************************************
 *
 *  $RCSfile: ChartController_Properties.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ChartController.hxx"

#include "DrawModelWrapper.hxx"
#include "chartview/ChartView.hxx"
#include "chartview/ObjectIdentifier.hxx"

#include "SchSlotIds.hxx"

#include "dlg_ObjectProperties.hxx"
#include "ViewElementListProvider.hxx"
#include "DataPointItemConverter.hxx"
#include "AxisItemConverter.hxx"
#include "MultipleItemConverter.hxx"
#include "TitleItemConverter.hxx"
#include "ChartModelHelper.hxx"
#include "MeterHelper.hxx"
#include "macros.hxx"
#include "TitleHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXISCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XAxisContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XGRIDCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XGridContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <drafts/com/sun/star/chart2/XIdentifiable.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <drafts/com/sun/star/chart2/XTitled.hpp>
#endif

//for auto_ptr
#include <memory>

// for test in executeDispatch_ObjectToDefault
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

namespace
{

uno::Reference< beans::XPropertySet > getObjectPropertySet( const rtl::OUString& rObjectCID
                , const uno::Reference< frame::XModel >& xModel )
{
    if(!rObjectCID.getLength())
        return NULL;
    if(!xModel.is())
        return NULL;

    uno::Reference< beans::XPropertySet > xObjectProperties = NULL;
    try
    {
        ObjectType eObjectType = ObjectIdentifier::getObjectType( rObjectCID );
        rtl::OUString aParticleID = ObjectIdentifier::getParticleID( rObjectCID );
        switch(eObjectType)
        {
            case OBJECTTYPE_PAGE:
                    break;
            case OBJECTTYPE_TITLE:
                {
                    if( TitleHelper::getIdentifierForTitle(TitleHelper::MAIN_TITLE).equals( aParticleID ) )
                        xObjectProperties.set( TitleHelper::getTitle(TitleHelper::MAIN_TITLE,xModel), uno::UNO_QUERY );
                }
                break;
            case OBJECTTYPE_LEGEND:
                    break;
            case OBJECTTYPE_LEGEND_ENTRY:
                    break;
            case OBJECTTYPE_DIAGRAM:
                     break;
            case OBJECTTYPE_DIAGRAM_WALL:
                {
                    uno::Reference< XDiagram > xDia( ChartModelHelper::findDiagram( xModel ) );
                    if( xDia.is())
                        xObjectProperties.set( xDia->getWall() );
                }
                break;
            case OBJECTTYPE_DIAGRAM_FLOOR:
                    break;
            case OBJECTTYPE_AXIS:
                {
                    uno::Reference< XAxisContainer > xAxisCnt( ChartModelHelper::findDiagram(xModel), uno::UNO_QUERY );
                    if( xAxisCnt.is())
                        xObjectProperties.set(
                            xAxisCnt->getAxisByIdentifier( aParticleID ),
                            uno::UNO_QUERY );
                }
                break;
            case OBJECTTYPE_AXIS_UNITLABEL:
                    break;
            case OBJECTTYPE_GRID:
                {
                    uno::Reference< XGridContainer > xGridCnt( ChartModelHelper::findDiagram(xModel), uno::UNO_QUERY );
                    if( xGridCnt.is())
                        xObjectProperties.set(
                            xGridCnt->getGridByIdentifier( aParticleID ),
                            uno::UNO_QUERY );
                }
                break;
            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_SERIES:
                {
                    uno::Reference< XDataSeries > xSeries =
                        ChartModelHelper::getSeriesByIdentifier( aParticleID, xModel );
                    if(xSeries.is())
                        xObjectProperties = uno::Reference< beans::XPropertySet >(
                                                xSeries, uno::UNO_QUERY );
                    break;
                }
            case OBJECTTYPE_DATA_LABEL:
            case OBJECTTYPE_DATA_POINT:
                {
                    rtl::OUString aSeriesID = ObjectIdentifier::getParentParticleID( rObjectCID );

                    uno::Reference< XDataSeries > xSeries =
                        ChartModelHelper::getSeriesByIdentifier( aSeriesID, xModel );
                    if(xSeries.is())
                    {
                        sal_Int32 nIndex = aParticleID.toInt32();
                        xObjectProperties = xSeries->getDataPointByIndex( nIndex );
                    }
                    break;
                }
            case OBJECTTYPE_DATA_ERRORS:
                    break;
            case OBJECTTYPE_DATA_ERRORS_X:
                    break;
            case OBJECTTYPE_DATA_ERRORS_Y:
                    break;
            case OBJECTTYPE_DATA_ERRORS_Z:
                    break;
            case OBJECTTYPE_DATA_FUNCTION:
                    break;
            case OBJECTTYPE_DATA_STOCK_RANGE:
                    break;
            case OBJECTTYPE_DATA_STOCK_LOSS:
                    break;
            case OBJECTTYPE_DATA_STOCK_GAIN:
                    break;
            default: //OBJECTTYPE_UNKNOWN
                    break;
        }
    }
    catch( uno::Exception& ex)
    {
        ex;
    }
    return xObjectProperties;
}

wrapper::ItemConverter* createItemConverter(
    const ::rtl::OUString & aObjectCID
    , const uno::Reference< frame::XModel > & xModel
    , SdrModel & rDrawModel
    , NumberFormatterWrapper * pNumberFormatterWrapper = NULL
    , ExplicitValueProvider * pExplicitValueProvider = NULL
    )
{
    wrapper::ItemConverter* pItemConverter=NULL;

    //-------------------------------------------------------------
    //get type of selected object
    ObjectType eObjectType = ObjectIdentifier::getObjectType( aObjectCID );
    if( OBJECTTYPE_UNKNOWN==eObjectType )
    {
        DBG_ERROR("unknown ObjectType");
        return NULL;
    }
    //--
    rtl::OUString aParticleID = ObjectIdentifier::getParticleID( aObjectCID );
    bool bAffectsMultipleObjects = aParticleID.equals(C2U("ALLELEMENTS"));
    //-------------------------------------------------------------
    if( !bAffectsMultipleObjects )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties= getObjectPropertySet( aObjectCID, xModel );
        if(!xObjectProperties.is())
            return NULL;
        //create itemconverter for a single object
        switch(eObjectType)
        {
            case OBJECTTYPE_PAGE:
                    break;
            case OBJECTTYPE_TITLE:
                pItemConverter = new wrapper::TitleItemConverter( xObjectProperties,
                                                                  rDrawModel.GetItemPool(),
                                                                  rDrawModel );
                    break;
            case OBJECTTYPE_LEGEND:
                    break;
            case OBJECTTYPE_LEGEND_ENTRY:
                    break;
            case OBJECTTYPE_DIAGRAM:
                    break;
            case OBJECTTYPE_DIAGRAM_WALL:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel,
                                        wrapper::GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES );
                    break;
            case OBJECTTYPE_DIAGRAM_FLOOR:
                    break;
            case OBJECTTYPE_AXIS:
            {
                uno::Reference< beans::XPropertySet > xDiaProp;
                xDiaProp.set( ChartModelHelper::findDiagram( xModel ), uno::UNO_QUERY );

                // the second property set contains the property CoordinateOrigin
                // nOriginIndex is the index of the corresponding index of the
                // origin (x=0, y=1, z=2)

                ExplicitScaleData aExplicitScale;
                ExplicitIncrementData aExplicitIncrement;
                double fExplicitOrigin;
                if( pExplicitValueProvider )
                    pExplicitValueProvider->getExplicitValuesForMeter(
                        uno::Reference< XMeter >( xObjectProperties, uno::UNO_QUERY ),
                        aExplicitScale, aExplicitIncrement, fExplicitOrigin );

                pItemConverter =  new wrapper::AxisItemConverter(
                    xObjectProperties, rDrawModel.GetItemPool(),
                    rDrawModel,
                    pNumberFormatterWrapper,
                    &aExplicitScale, &aExplicitIncrement, &fExplicitOrigin );
            }
            break;
            case OBJECTTYPE_AXIS_UNITLABEL:
                    break;
            case OBJECTTYPE_GRID:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel,
                                        wrapper::GraphicPropertyItemConverter::LINE_PROPERTIES );
                    break;
            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_SERIES:
                pItemConverter =  new wrapper::DataPointItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(), rDrawModel,
                                        pNumberFormatterWrapper );
                    break;
            case OBJECTTYPE_DATA_LABEL:
            case OBJECTTYPE_DATA_POINT:
                pItemConverter =  new wrapper::DataPointItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(), rDrawModel,
                                        pNumberFormatterWrapper );
                    break;
            case OBJECTTYPE_DATA_ERRORS:
                    break;
            case OBJECTTYPE_DATA_ERRORS_X:
                    break;
            case OBJECTTYPE_DATA_ERRORS_Y:
                    break;
            case OBJECTTYPE_DATA_ERRORS_Z:
                    break;
            case OBJECTTYPE_DATA_FUNCTION:
                    break;
            case OBJECTTYPE_DATA_STOCK_RANGE:
                    break;
            case OBJECTTYPE_DATA_STOCK_LOSS:
                    break;
            case OBJECTTYPE_DATA_STOCK_GAIN:
                    break;
            default: //OBJECTTYPE_UNKNOWN
                    break;
        }
    }
    else
    {
        //create itemconverter for a all objects of given type
        switch(eObjectType)
        {
            case OBJECTTYPE_TITLE:
                break;
            case OBJECTTYPE_AXIS:
                pItemConverter =  new wrapper::AllAxisItemConverter( xModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel );
                break;
            case OBJECTTYPE_GRID:
                pItemConverter =  new wrapper::AllGridItemConverter( xModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel );
                break;
            default: //for this type it is not supported to change all elements at once
                break;
        }

    }
    return pItemConverter;
}

rtl::OUString getTitleIDForSlotId( sal_Int32 nSlotID, const uno::Reference< XChartDocument > & xChartDocument )
{
    rtl::OUString aRet;
    switch(nSlotID)
    {
        //@todo get id for special objects
        case SID_DIAGRAM_TITLE_MAIN:
            aRet = TitleHelper::getIdentifierForTitle(TitleHelper::MAIN_TITLE);
            break;
        case SID_DIAGRAM_TITLE_SUB:
            aRet = TitleHelper::getIdentifierForTitle(TitleHelper::SUB_TITLE);
            break;
        case SID_DIAGRAM_TITLE_X:
            aRet = TitleHelper::getIdentifierForTitle(TitleHelper::X_AXIS_TITLE);
            break;
        case SID_DIAGRAM_TITLE_Y:
            aRet = TitleHelper::getIdentifierForTitle(TitleHelper::Y_AXIS_TITLE);
            break;
        case SID_DIAGRAM_TITLE_Z:
            aRet = TitleHelper::getIdentifierForTitle(TitleHelper::Z_AXIS_TITLE);
            break;
        case SID_DIAGRAM_TITLE_ALL:
            break;
        default:
            break;
    }
    return aRet;
}

rtl::OUString getAxisIDForSlotId( sal_Int32 nSlotID, const uno::Reference< XDiagram >& xDiagram )
{
    if( (sal_Int32)SID_DIAGRAM_AXIS_ALL == nSlotID )
        return rtl::OUString();

    sal_Int32   nDimensionIndex=0;
    bool        bMainAxis=true;
    switch( nSlotID )
    {
        case (sal_Int32)SID_DIAGRAM_AXIS_X:
            nDimensionIndex=0; bMainAxis=true; break;
        case (sal_Int32)SID_DIAGRAM_AXIS_Y:
            nDimensionIndex=1; bMainAxis=true; break;
        case (sal_Int32)SID_DIAGRAM_AXIS_Z:
            nDimensionIndex=2; bMainAxis=true; break;
        case (sal_Int32)SID_DIAGRAM_AXIS_A:
            nDimensionIndex=0; bMainAxis=false; break;
        case (sal_Int32)SID_DIAGRAM_AXIS_B:
            nDimensionIndex=1; bMainAxis=false; break;
        default:
            break;
    }
    uno::Reference< XAxis > xAxis = MeterHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram );

    uno::Reference< XIdentifiable > xIdent( xAxis, uno::UNO_QUERY );
    if( xIdent.is() )
        return xIdent->getIdentifier();
    return rtl::OUString();
}

rtl::OUString getGridIDForSlotId( sal_Int32 nSlotID, const uno::Reference< XDiagram >& xDiagram )
{
    if( (sal_Int32)SID_DIAGRAM_GRID_ALL == nSlotID )
        return rtl::OUString();

    sal_Int32   nDimensionIndex=0;
    bool        bMainGrid=true;
    switch( nSlotID )
    {
        case (sal_Int32)SID_DIAGRAM_GRID_X_MAIN:
            nDimensionIndex=0; bMainGrid=true; break;
        case (sal_Int32)SID_DIAGRAM_GRID_X_HELP:
            nDimensionIndex=0; bMainGrid=false; break;
        case (sal_Int32)SID_DIAGRAM_GRID_Y_MAIN:
            nDimensionIndex=1; bMainGrid=true; break;
        case (sal_Int32)SID_DIAGRAM_GRID_Y_HELP:
            nDimensionIndex=1; bMainGrid=false; break;
        case (sal_Int32)SID_DIAGRAM_GRID_Z_MAIN:
            nDimensionIndex=2; bMainGrid=true; break;
        case (sal_Int32)SID_DIAGRAM_GRID_Z_HELP:
            nDimensionIndex=2; bMainGrid=false; break;
        default:
            break;
    }
    uno::Reference< XGrid > xGrid = MeterHelper::getGrid( nDimensionIndex, bMainGrid, xDiagram );

    uno::Reference< XIdentifiable > xIdent( xGrid, uno::UNO_QUERY );
    if( xIdent.is() )
        return xIdent->getIdentifier();
    return rtl::OUString();
}

rtl::OUString getObjectCIDForSlotId( sal_Int32 nSlotID, const uno::Reference< XChartDocument > & xChartDocument )
{
    ObjectType eObjectType;
    rtl::OUString aParticleID;

    if(        (sal_Int32)SID_DIAGRAM_TITLE_MAIN == nSlotID
            || (sal_Int32)SID_DIAGRAM_TITLE_SUB == nSlotID
            || (sal_Int32)SID_DIAGRAM_TITLE_X == nSlotID
            || (sal_Int32)SID_DIAGRAM_TITLE_Y == nSlotID
            || (sal_Int32)SID_DIAGRAM_TITLE_Z == nSlotID
            || (sal_Int32)SID_DIAGRAM_TITLE_ALL == nSlotID )
    {
        eObjectType = OBJECTTYPE_TITLE;
        aParticleID = getTitleIDForSlotId( nSlotID, xChartDocument );
    }
    else if(   (sal_Int32)SID_DIAGRAM_AXIS_X == nSlotID
            || (sal_Int32)SID_DIAGRAM_AXIS_Y == nSlotID
            || (sal_Int32)SID_DIAGRAM_AXIS_Z == nSlotID
            || (sal_Int32)SID_DIAGRAM_AXIS_A == nSlotID //secondary x axis
            || (sal_Int32)SID_DIAGRAM_AXIS_B == nSlotID
            || (sal_Int32)SID_DIAGRAM_AXIS_ALL == nSlotID )
    {
        eObjectType = OBJECTTYPE_AXIS;
        aParticleID = getAxisIDForSlotId( nSlotID, xChartDocument->getDiagram() );
    }
    else if(   (sal_Int32)SID_DIAGRAM_GRID_X_MAIN == nSlotID
            || (sal_Int32)SID_DIAGRAM_GRID_Y_MAIN == nSlotID
            || (sal_Int32)SID_DIAGRAM_GRID_Z_MAIN == nSlotID
            || (sal_Int32)SID_DIAGRAM_GRID_X_HELP == nSlotID
            || (sal_Int32)SID_DIAGRAM_GRID_Y_HELP == nSlotID
            || (sal_Int32)SID_DIAGRAM_GRID_Z_HELP == nSlotID
            || (sal_Int32)SID_DIAGRAM_GRID_ALL == nSlotID )
    {
        eObjectType = OBJECTTYPE_GRID;
        aParticleID = getGridIDForSlotId( nSlotID, xChartDocument->getDiagram() );
    }
    else if( (sal_Int32)SID_LEGEND == nSlotID )
    {
        eObjectType = OBJECTTYPE_LEGEND;
        //@todo set particular aParticleID if we have more than one legend
    }
    else if(   (sal_Int32)SID_DIAGRAM_WALL == nSlotID )
    {
        //OBJECTTYPE_DIAGRAM;
        eObjectType = OBJECTTYPE_DIAGRAM_WALL;
        //@todo set particular aParticleID if we have more than one diagram
    }
    else if(   (sal_Int32)SID_DIAGRAM_FLOOR == nSlotID )
    {
        eObjectType = OBJECTTYPE_DIAGRAM_FLOOR;
        //@todo set particular aParticleID if we have more than one diagram
    }
    else if(   (sal_Int32)SID_DIAGRAM_AREA == nSlotID )
    {
        eObjectType = OBJECTTYPE_PAGE;
    }

    if(    (sal_Int32)SID_DIAGRAM_TITLE_ALL == nSlotID
        || (sal_Int32)SID_DIAGRAM_AXIS_ALL == nSlotID
        || (sal_Int32)SID_DIAGRAM_GRID_ALL == nSlotID )
    {
        aParticleID = C2U("ALLELEMENTS");
    }
    return ObjectIdentifier::createClassifiedIdentifier(
        eObjectType, aParticleID );
}

}
// anonymous namespace

void SAL_CALL ChartController::executeDispatch_FormatObject(sal_Int32 nSlotID)
{
    uno::Reference< XChartDocument > xChartDocument( m_aModel->getModel(), uno::UNO_QUERY );
    rtl::OUString rObjectCID = getObjectCIDForSlotId( nSlotID, xChartDocument );
    executeDlg_ObjectProperties( rObjectCID );
}

void SAL_CALL ChartController::executeDispatch_ObjectProperties()
{
    executeDlg_ObjectProperties( m_aSelectedObjectCID );
}

void SAL_CALL ChartController::executeDlg_ObjectProperties( const ::rtl::OUString& rObjectCID )
{
    bool bChanged = false;
    if( !rObjectCID.getLength() )
    {
        //DBG_ERROR("empty ObjectID");
        return;
    }
    try
    {
        //-------------------------------------------------------------
        //get type of selected object
        ObjectType eObjectType = ObjectIdentifier::getObjectType( rObjectCID );
        if( OBJECTTYPE_UNKNOWN==eObjectType )
        {
            //DBG_ERROR("unknown ObjectType");
            return;
        }
        rtl::OUString aParticleID = ObjectIdentifier::getParticleID( rObjectCID );
        bool bAffectsMultipleObjects = aParticleID.equals(C2U("ALLELEMENTS"));
        //-------------------------------------------------------------
        //convert properties to ItemSet
        ::std::auto_ptr< wrapper::ItemConverter > apItemConverter(
            createItemConverter( rObjectCID, m_aModel->getModel(),
                                 m_pDrawModelWrapper->getSdrModel(),
                                 m_pNumberFormatterWrapper,
                                 m_pChartView ));
        if(!apItemConverter.get())
            return;
        SfxItemSet aItemSet = apItemConverter->CreateEmptyItemSet();
        apItemConverter->FillItemSet( aItemSet );

        //-------------------------------------------------------------
        //prepare dialog
        OldModelWrapper aOldChartModelWrapper = OldModelWrapper();
        Window* pParent( NULL );
        ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper, m_pNumberFormatterWrapper );
        Graphic aSymbolGraphic;

        SchAttribTabDlg aDlg( pParent
            , eObjectType, &aItemSet
            , &aViewElementListProvider, &aOldChartModelWrapper
            , bAffectsMultipleObjects );
            //, pSymbolAttr, aSymbolGraphic );

        //-------------------------------------------------------------
        //open the dialog
        if( aDlg.Execute() == RET_OK )
        {
            const SfxItemSet* pOutItemSet = aDlg.GetOutputItemSet();
            if(pOutItemSet)
            {
                bChanged = apItemConverter->ApplyItemSet( *pOutItemSet );//model should be changed now
            }
        }
        /*
        pDlg = new SchAttribTabDlg(NULL, eType, &aRowAttr,pDoc->GetObjectShell() ,pDoc
                        , 0, pDoc->ChartStyle(),&aSymbolAttr
                        ,GenSymbolGraphic(pDataRow->GetRow()));

        SfxItemSet aSymbolAttr(aPointAttr);
        if(pDoc->HasSymbols(pDataPoint->GetRow()))
        {
            pDoc->GenerateSymbolAttr(aSymbolAttr,pDataPoint->GetRow());
            pDlg = new SchAttribTabDlg(NULL, ATTR_DATA_POINT, &aPointAttr, pDoc->GetObjectShell(), pDoc
                , 0, pDoc->ChartStyle(),&aSymbolAttr
                ,GenSymbolGraphic(pDataPoint->GetRow(),pDataPoint->GetCol()));
        }
        else
        {
            pDlg = new SchAttribTabDlg(NULL, ATTR_DATA_POINT, &aPointAttr, pDoc->GetObjectShell(), pDoc
                , 0, pDoc->ChartStyle());
        }
        */
    }
    catch( util::CloseVetoException& )
    {
    }
    catch( uno::RuntimeException& )
    {
    }

    try
    {
        //make sure that all objects using  m_pDrawModelWrapper or m_pChartView are already deleted
        if(bChanged)
            impl_rebuildView();
    }
    catch( uno::RuntimeException& e)
    {
        e;
    }
}

void SAL_CALL ChartController::executeDispatch_ObjectToDefault()
{
    if( !m_aSelectedObjectCID.getLength() )
    {
        DBG_ERROR("nothing is selected");
        return;
    }

    try
    {
        //-------------------------------------------------------------
        //get type of selected object
        ObjectType eObjectType = ObjectIdentifier::getObjectType( m_aSelectedObjectCID );
        if( OBJECTTYPE_UNKNOWN==eObjectType )
        {
            DBG_ERROR("unknown ObjectType");
            return;
        }
        //-------------------------------------------------------------
        //get properties of selected object
        uno::Reference< beans::XPropertySet > xObjectProperties = NULL;
        xObjectProperties = getObjectPropertySet( m_aSelectedObjectCID, getModel() );
        if(!xObjectProperties.is())
            return;

        uno::Sequence< beans::Property > aProps( xObjectProperties->getPropertySetInfo()->getProperties() );
        uno::Reference< beans::XPropertyState > xState( xObjectProperties, uno::UNO_QUERY );

        for( sal_Int32 i = 0; i < aProps.getLength(); ++i )
        {
            if( aProps[i].Attributes & beans::PropertyAttribute::MAYBEDEFAULT )
                xState->setPropertyToDefault( aProps[i].Name );
        }

        impl_rebuildView();
    }
    catch( uno::Exception& )
    {
        DBG_ERROR( "Exception caught" );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
