/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ShapeController.hxx"
#include "ShapeController.hrc"
#include "ChartController.hxx"
#include "ChartWindow.hxx"
#include "ViewElementListProvider.hxx"
#include "dlg_ShapeFont.hxx"
#include "dlg_ShapeParagraph.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "macros.hxx"

#include <com/sun/star/frame/XStorable.hpp>

#include <osl/mutex.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <editeng/brkitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/drawitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/spltitem.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/widwitem.hxx>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

//.............................................................................
namespace chart
{
//.............................................................................

ShapeController::ShapeController( const Reference< uno::XComponentContext >& rxContext,
        ChartController* pController )
    :FeatureCommandDispatchBase( rxContext )
    ,m_pChartController( pController )
{
}

ShapeController::~ShapeController()
{
}

void ShapeController::initialize()
{
    FeatureCommandDispatchBase::initialize();
}

// WeakComponentImplHelperBase
void ShapeController::disposing()
{
}

// XEventListener
void ShapeController::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
}

FeatureState ShapeController::getState( const ::rtl::OUString& rCommand )
{
    FeatureState aReturn;
    aReturn.bEnabled = false;
    aReturn.aState <<= false;

    bool bWritable = false;
    if ( m_pChartController )
    {
        Reference< frame::XStorable > xStorable( m_pChartController->getModel(), uno::UNO_QUERY );
        if ( xStorable.is() )
        {
            bWritable = !xStorable->isReadonly();
        }
    }

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( rCommand );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        sal_uInt16 nFeatureId = aIter->second.nFeatureId;
        switch ( nFeatureId )
        {
            case COMMAND_ID_FORMAT_LINE:
            case COMMAND_ID_FORMAT_AREA:
            case COMMAND_ID_TEXT_ATTRIBUTES:
            case COMMAND_ID_TRANSFORM_DIALOG:
            case COMMAND_ID_OBJECT_TITLE_DESCRIPTION:
            case COMMAND_ID_RENAME_OBJECT:
                {
                    aReturn.bEnabled = bWritable;
                    aReturn.aState <<= false;
                }
                break;
            case COMMAND_ID_BRING_TO_FRONT:
            case COMMAND_ID_FORWARD:
                {
                    aReturn.bEnabled = ( bWritable && isForwardPossible() );
                    aReturn.aState <<= false;
                }
                break;
            case COMMAND_ID_BACKWARD:
            case COMMAND_ID_SEND_TO_BACK:
                {

                    aReturn.bEnabled = ( bWritable && isBackwardPossible() );
                    aReturn.aState <<= false;
                }
                break;
            case COMMAND_ID_FONT_DIALOG:
            case COMMAND_ID_PARAGRAPH_DIALOG:
                {
                    aReturn.bEnabled = bWritable;
                    aReturn.aState <<= false;
                }
                break;
            default:
                {
                    aReturn.bEnabled = false;
                    aReturn.aState <<= false;
                }
                break;
        }
    }

    return aReturn;
}

void ShapeController::execute( const ::rtl::OUString& rCommand, const Sequence< beans::PropertyValue>& rArgs )
{
    (void)rArgs;

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( rCommand );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        sal_uInt16 nFeatureId = aIter->second.nFeatureId;
        switch ( nFeatureId )
        {
            case COMMAND_ID_FORMAT_LINE:
                {
                    executeDispatch_FormatLine();
                }
                break;
            case COMMAND_ID_FORMAT_AREA:
                {
                    executeDispatch_FormatArea();
                }
                break;
            case COMMAND_ID_TEXT_ATTRIBUTES:
                {
                    executeDispatch_TextAttributes();
                }
                break;
            case COMMAND_ID_TRANSFORM_DIALOG:
                {
                    executeDispatch_TransformDialog();
                }
                break;
            case COMMAND_ID_OBJECT_TITLE_DESCRIPTION:
                {
                    executeDispatch_ObjectTitleDescription();
                }
                break;
            case COMMAND_ID_RENAME_OBJECT:
                {
                    executeDispatch_RenameObject();
                }
                break;
            case COMMAND_ID_BRING_TO_FRONT:
            case COMMAND_ID_FORWARD:
            case COMMAND_ID_BACKWARD:
            case COMMAND_ID_SEND_TO_BACK:
                {
                    executeDispatch_ChangeZOrder( nFeatureId );
                }
                break;
            case COMMAND_ID_FONT_DIALOG:
                {
                    executeDispatch_FontDialog();
                }
                break;
            case COMMAND_ID_PARAGRAPH_DIALOG:
                {
                    executeDispatch_ParagraphDialog();
                }
                break;
            default:
                {
                }
                break;
        }
    }
}

void ShapeController::describeSupportedFeatures()
{
    implDescribeSupportedFeature( ".uno:FormatLine",                COMMAND_ID_FORMAT_LINE,                 CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FormatArea",                COMMAND_ID_FORMAT_AREA,                 CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:TextAttributes",            COMMAND_ID_TEXT_ATTRIBUTES,             CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:TransformDialog",           COMMAND_ID_TRANSFORM_DIALOG,            CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:ObjectTitleDescription",    COMMAND_ID_OBJECT_TITLE_DESCRIPTION,    CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:RenameObject",              COMMAND_ID_RENAME_OBJECT,               CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:BringToFront",              COMMAND_ID_BRING_TO_FRONT,              CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Forward",                   COMMAND_ID_FORWARD,                     CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Backward",                  COMMAND_ID_BACKWARD,                    CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SendToBack",                COMMAND_ID_SEND_TO_BACK,                CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FontDialog",                COMMAND_ID_FONT_DIALOG,                 CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:ParagraphDialog",           COMMAND_ID_PARAGRAPH_DIALOG,            CommandGroup::EDIT );
}

IMPL_LINK( ShapeController, CheckNameHdl, AbstractSvxNameDialog*, pDialog )
{
    String aName;
    if ( pDialog )
    {
        pDialog->GetName( aName );
    }
    if ( aName.Len() )
    {
        DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : NULL );
        if ( pDrawViewWrapper && pDrawViewWrapper->getNamedSdrObject( aName ) )
        {
            return 0;
        }
    }
    return 1;
}

void ShapeController::executeDispatch_FormatLine()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawModelWrapper && pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            sal_Bool bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, sal_False );
            }
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                    pFact->CreateSvxLineTabDialog( pParent, &aAttr, &pDrawModelWrapper->getSdrModel(),
                        pSelectedObj, bHasMarked ) );
                if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
                {
                    const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                    if ( bHasMarked )
                    {
                        pDrawViewWrapper->SetAttrToMarked( *pOutAttr, sal_False );
                    }
                    else
                    {
                        pDrawViewWrapper->SetDefaultAttr( *pOutAttr, sal_False );
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_FormatArea()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawModelWrapper && pDrawViewWrapper )
        {
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            sal_Bool bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, sal_False );
            }
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::boost::scoped_ptr< AbstractSvxAreaTabDialog > pDlg(
                    pFact->CreateSvxAreaTabDialog( pParent, &aAttr, &pDrawModelWrapper->getSdrModel(),
                        pDrawViewWrapper ) );
                if ( pDlg.get() )
                {
                    SfxItemPool& rItemPool = pDrawViewWrapper->GetModel()->GetItemPool();
                    SfxItemSet aSet( rItemPool, rItemPool.GetFirstWhich(), rItemPool.GetLastWhich() );
                    const SvxColorTableItem* pColorItem = static_cast< const SvxColorTableItem* >( aSet.GetItem( SID_COLOR_TABLE ) );
                    if ( pColorItem && pColorItem->GetColorTable() == XColorTable::GetStdColorTable() )
                    {
                        pDlg->DontDeleteColorTable();
                    }
                    if ( pDlg->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                        if ( bHasMarked )
                        {
                            pDrawViewWrapper->SetAttrToMarked( *pOutAttr, sal_False );
                        }
                        else
                        {
                            pDrawViewWrapper->SetDefaultAttr( *pOutAttr, sal_False );
                        }
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_TextAttributes()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawViewWrapper )
        {
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            sal_Bool bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, sal_False );
            }
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                    pFact->CreateTextTabDialog( pParent, &aAttr, pDrawViewWrapper ) );
                if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
                {
                    const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                    if ( bHasMarked )
                    {
                        pDrawViewWrapper->SetAttributes( *pOutAttr );
                    }
                    else
                    {
                        pDrawViewWrapper->SetDefaultAttr( *pOutAttr, sal_False );
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_TransformDialog()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            if ( pSelectedObj && pSelectedObj->GetObjIdentifier() == OBJ_CAPTION )
            {
                // item set for caption
                SfxItemSet aAttr( pDrawViewWrapper->GetModel()->GetItemPool() );
                pDrawViewWrapper->GetAttributes( aAttr );
                // item set for position and size
                SfxItemSet aGeoAttr( pDrawViewWrapper->GetGeoAttrFromMarked() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                        pFact->CreateCaptionDialog( pParent, pDrawViewWrapper ) );
                    if ( pDlg.get() )
                    {
                        const sal_uInt16* pRange = pDlg->GetInputRanges( *aAttr.GetPool() );
                        SfxItemSet aCombAttr( *aAttr.GetPool(), pRange );
                        aCombAttr.Put( aAttr );
                        aCombAttr.Put( aGeoAttr );
                        pDlg->SetInputSet( &aCombAttr );
                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                            pDrawViewWrapper->SetAttributes( *pOutAttr );
                            pDrawViewWrapper->SetGeoAttrToMarked( *pOutAttr );
                        }
                    }
                }
            }
            else
            {
                SfxItemSet aGeoAttr( pDrawViewWrapper->GetGeoAttrFromMarked() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                        pFact->CreateSvxTransformTabDialog( pParent, &aGeoAttr, pDrawViewWrapper ) );
                    if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
                    {
                        const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                        pDrawViewWrapper->SetGeoAttrToMarked( *pOutAttr );
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_ObjectTitleDescription()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pChartController )
    {
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper && pDrawViewWrapper->GetMarkedObjectCount() == 1 )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            if ( pSelectedObj )
            {
                String aTitle( pSelectedObj->GetTitle() );
                String aDescription( pSelectedObj->GetDescription() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< AbstractSvxObjectTitleDescDialog > pDlg(
                        pFact->CreateSvxObjectTitleDescDialog( NULL, aTitle, aDescription ) );
                    if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
                    {
                        pDlg->GetTitle( aTitle );
                        pDlg->GetDescription( aDescription );
                        pSelectedObj->SetTitle( aTitle );
                        pSelectedObj->SetDescription( aDescription );
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_RenameObject()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pChartController )
    {
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper && pDrawViewWrapper->GetMarkedObjectCount() == 1 )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            if ( pSelectedObj )
            {
                String aName( pSelectedObj->GetName() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< AbstractSvxObjectNameDialog > pDlg(
                        pFact->CreateSvxObjectNameDialog( NULL, aName ) );
                    pDlg->SetCheckNameHdl( LINK( this, ShapeController, CheckNameHdl ) );
                    if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
                    {
                        pDlg->GetName( aName );
                        if ( aName != pSelectedObj->GetName() )
                        {
                            pSelectedObj->SetName( aName );
                        }
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_ChangeZOrder( sal_uInt16 nId )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : NULL );
    if ( pDrawViewWrapper )
    {
        switch ( nId )
        {
            case COMMAND_ID_BRING_TO_FRONT:
                {
                    if ( isForwardPossible() )
                    {
                        pDrawViewWrapper->PutMarkedToTop();
                    }
                }
                break;
            case COMMAND_ID_FORWARD:
                {
                    if ( isForwardPossible() )
                    {
                        pDrawViewWrapper->MovMarkedToTop();
                    }
                }
                break;
            case COMMAND_ID_BACKWARD:
                {
                    if ( isBackwardPossible() )
                    {
                        pDrawViewWrapper->MovMarkedToBtm();
                    }
                }
                break;
            case COMMAND_ID_SEND_TO_BACK:
                {
                    if ( isBackwardPossible() )
                    {
                        SdrObject* pFirstObj = getFirstAdditionalShape();
                        pDrawViewWrapper->PutMarkedBehindObj( pFirstObj );
                    }
                }
                break;
            default:
                {
                }
                break;
        }
    }
}

void ShapeController::executeDispatch_FontDialog()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawModelWrapper && pDrawViewWrapper )
        {
            SfxItemSet aAttr( pDrawViewWrapper->GetModel()->GetItemPool() );
            pDrawViewWrapper->GetAttributes( aAttr );
            ViewElementListProvider aViewElementListProvider( pDrawModelWrapper );
            ::boost::scoped_ptr< ShapeFontDialog > pDlg( new ShapeFontDialog( pParent, &aAttr, &aViewElementListProvider ) );
            if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
            {
                const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                pDrawViewWrapper->SetAttributes( *pOutAttr );
            }
        }
    }
}

void ShapeController::executeDispatch_ParagraphDialog()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawViewWrapper )
        {
            SfxItemPool& rPool = pDrawViewWrapper->GetModel()->GetItemPool();
            SfxItemSet aAttr( rPool );
            pDrawViewWrapper->GetAttributes( aAttr );

            SfxItemSet aNewAttr( rPool,
                                    EE_ITEMS_START, EE_ITEMS_END,
                                    SID_ATTR_PARA_HYPHENZONE, SID_ATTR_PARA_HYPHENZONE,
                                    SID_ATTR_PARA_PAGEBREAK, SID_ATTR_PARA_PAGEBREAK,
                                    SID_ATTR_PARA_SPLIT, SID_ATTR_PARA_SPLIT,
                                    SID_ATTR_PARA_WIDOWS, SID_ATTR_PARA_WIDOWS,
                                    SID_ATTR_PARA_ORPHANS, SID_ATTR_PARA_ORPHANS,
                                    0 );
            aNewAttr.Put( aAttr );
            aNewAttr.Put( SvxHyphenZoneItem( sal_False, SID_ATTR_PARA_HYPHENZONE ) );
            aNewAttr.Put( SvxFmtBreakItem( SVX_BREAK_NONE, SID_ATTR_PARA_PAGEBREAK ) );
            aNewAttr.Put( SvxFmtSplitItem( sal_True, SID_ATTR_PARA_SPLIT)  );
            aNewAttr.Put( SvxWidowsItem( 0, SID_ATTR_PARA_WIDOWS) );
            aNewAttr.Put( SvxOrphansItem( 0, SID_ATTR_PARA_ORPHANS) );

            ::boost::scoped_ptr< ShapeParagraphDialog > pDlg( new ShapeParagraphDialog( pParent, &aNewAttr ) );
            if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
            {
                const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                pDrawViewWrapper->SetAttributes( *pOutAttr );
            }
        }
    }
}

SdrObject* ShapeController::getFirstAdditionalShape()
{
    SdrObject* pFirstObj = NULL;

    try
    {
        DrawModelWrapper* pDrawModelWrapper = ( m_pChartController ? m_pChartController->GetDrawModelWrapper() : NULL );
        if ( pDrawModelWrapper )
        {
            Reference< drawing::XShape > xFirstShape;
            Reference< drawing::XDrawPage > xDrawPage( pDrawModelWrapper->getMainDrawPage() );
            Reference< drawing::XShapes > xDrawPageShapes( xDrawPage, uno::UNO_QUERY_THROW );
            Reference< drawing::XShapes > xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );
            sal_Int32 nCount = xDrawPageShapes->getCount();
            for ( sal_Int32 i = 0; i < nCount; ++i )
            {
                Reference< drawing::XShape > xShape;
                if ( xDrawPageShapes->getByIndex( i ) >>= xShape )
                {
                    if ( xShape.is() && xShape != xChartRoot )
                    {
                        xFirstShape = xShape;
                        break;
                    }
                }
            }
            if ( xFirstShape.is() )
            {
                pFirstObj = DrawViewWrapper::getSdrObject( xFirstShape );
            }
        }
    }
    catch ( uno::Exception& ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return pFirstObj;
}

SdrObject* ShapeController::getLastAdditionalShape()
{
    SdrObject* pLastObj = NULL;

    try
    {
        DrawModelWrapper* pDrawModelWrapper = ( m_pChartController ? m_pChartController->GetDrawModelWrapper() : NULL );
        if ( pDrawModelWrapper )
        {
            Reference< drawing::XShape > xLastShape;
            Reference< drawing::XDrawPage > xDrawPage( pDrawModelWrapper->getMainDrawPage() );
            Reference< drawing::XShapes > xDrawPageShapes( xDrawPage, uno::UNO_QUERY_THROW );
            Reference< drawing::XShapes > xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );
            sal_Int32 nCount = xDrawPageShapes->getCount();
            for ( sal_Int32 i = nCount - 1; i >= 0; --i )
            {
                Reference< drawing::XShape > xShape;
                if ( xDrawPageShapes->getByIndex( i ) >>= xShape )
                {
                    if ( xShape.is() && xShape != xChartRoot )
                    {
                        xLastShape = xShape;
                        break;
                    }
                }
            }
            if ( xLastShape.is() )
            {
                pLastObj = DrawViewWrapper::getSdrObject( xLastShape );
            }
        }
    }
    catch ( uno::Exception& ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return pLastObj;
}

bool ShapeController::isBackwardPossible()
{
    if ( m_pChartController && m_pChartController->m_aSelection.isAdditionalShapeSelected() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            SdrObject* pFirstObj = getFirstAdditionalShape();
            if ( pSelectedObj && pFirstObj && pSelectedObj != pFirstObj )
            {
                return true;
            }
        }
    }
    return false;
}

bool ShapeController::isForwardPossible()
{
    if ( m_pChartController && m_pChartController->m_aSelection.isAdditionalShapeSelected() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            SdrObject* pLastObj = getLastAdditionalShape();
            if ( pSelectedObj && pLastObj && pSelectedObj != pLastObj )
            {
                return true;
            }
        }
    }
    return false;
}

//.............................................................................
} //  namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
