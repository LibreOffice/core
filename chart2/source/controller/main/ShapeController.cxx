/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ShapeController.cxx,v $
 * $Revision: 1.0 $
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
#include "chartview/DrawModelWrapper.hxx"

#include <vos/mutex.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <svx/drawitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

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

FeatureState ShapeController::getState( const ::rtl::OUString& rCommand ) const
{
    FeatureState aReturn;
    aReturn.bEnabled = false;
    aReturn.aState <<= false;
    sal_uInt16 nFeatureId = m_aSupportedFeatures[ rCommand ].nFeatureId;

    switch ( nFeatureId )
    {
        case COMMAND_ID_FORMAT_LINE:
        case COMMAND_ID_FORMAT_AREA:
        case COMMAND_ID_TEXT_ATTRIBUTES:
        case COMMAND_ID_TRANSFORM_DIALOG:
        case COMMAND_ID_OBJECT_TITLE_DESCRIPTION:
        case COMMAND_ID_RENAME_OBJECT:
        case COMMAND_ID_FONT_DIALOG:
            {
                aReturn.bEnabled = true;
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

    return aReturn;
}

void ShapeController::execute( const ::rtl::OUString& rCommand, const Sequence< beans::PropertyValue>& rArgs )
{
    (void)rArgs;

    sal_uInt16 nFeatureId = m_aSupportedFeatures[ rCommand ].nFeatureId;

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
        case COMMAND_ID_FONT_DIALOG:
            {
                executeDispatch_FontDialog();
            }
            break;
        default:
            {
            }
            break;
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
    implDescribeSupportedFeature( ".uno:FontDialog",                COMMAND_ID_FONT_DIALOG,                 CommandGroup::EDIT );
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
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawModelWrapper && pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            BOOL bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, FALSE );
            }
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                    pFact->CreateSvxLineTabDialog( pParent, &aAttr, &pDrawModelWrapper->getSdrModel(),
                        RID_SVXDLG_LINE, pSelectedObj, bHasMarked ) );
                if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
                {
                    const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                    if ( bHasMarked )
                    {
                        pDrawViewWrapper->SetAttrToMarked( *pOutAttr, FALSE );
                    }
                    else
                    {
                        pDrawViewWrapper->SetDefaultAttr( *pOutAttr, FALSE );
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_FormatArea()
{
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawModelWrapper && pDrawViewWrapper )
        {
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            BOOL bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, FALSE );
            }
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::boost::scoped_ptr< AbstractSvxAreaTabDialog > pDlg(
                    pFact->CreateSvxAreaTabDialog( pParent, &aAttr, &pDrawModelWrapper->getSdrModel(),
                        RID_SVXDLG_AREA, pDrawViewWrapper ) );
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
                            pDrawViewWrapper->SetAttrToMarked( *pOutAttr, FALSE );
                        }
                        else
                        {
                            pDrawViewWrapper->SetDefaultAttr( *pOutAttr, FALSE );
                        }
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_TextAttributes()
{
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pParent && pDrawViewWrapper )
        {
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            BOOL bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, FALSE );
            }
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                    pFact->CreateTextTabDialog( pParent, &aAttr, RID_SVXDLG_TEXT, pDrawViewWrapper ) );
                if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
                {
                    const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                    if ( bHasMarked )
                    {
                        pDrawViewWrapper->SetAttributes( *pOutAttr );
                    }
                    else
                    {
                        pDrawViewWrapper->SetDefaultAttr( *pOutAttr, FALSE );
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_TransformDialog()
{
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
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                        pFact->CreateCaptionDialog( pParent, pDrawViewWrapper, RID_SVXDLG_CAPTION ) );
                    if ( pDlg.get() )
                    {
                        const USHORT* pRange = pDlg->GetInputRanges( *aAttr.GetPool() );
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
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                        pFact->CreateSvxTransformTabDialog( pParent, &aGeoAttr, pDrawViewWrapper, RID_SVXDLG_TRANSFORM ) );
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
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< AbstractSvxObjectTitleDescDialog > pDlg(
                        pFact->CreateSvxObjectTitleDescDialog( NULL, aTitle, aDescription, RID_SVXDLG_OBJECT_TITLE_DESC ) );
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
    if ( m_pChartController )
    {
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper && pDrawViewWrapper->GetMarkedObjectCount() == 1 )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            if ( pSelectedObj )
            {
                String aName( pSelectedObj->GetName() );
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< AbstractSvxObjectNameDialog > pDlg(
                        pFact->CreateSvxObjectNameDialog( NULL, aName, RID_SVXDLG_OBJECT_NAME ) );
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

void ShapeController::executeDispatch_FontDialog()
{
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
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            ::boost::scoped_ptr< ShapeFontDialog > pDlg( new ShapeFontDialog( pParent, &aAttr, &aViewElementListProvider ) );
            if ( pDlg.get() && ( pDlg->Execute() == RET_OK ) )
            {
                const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                pDrawViewWrapper->SetAttributes( *pOutAttr );
            }
        }
    }
}

//.............................................................................
} //  namespace chart
//.............................................................................
