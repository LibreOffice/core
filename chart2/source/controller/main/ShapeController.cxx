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
#include "ChartController.hxx"
#include "ChartWindow.hxx"
#include "ViewElementListProvider.hxx"
#include "dlg_ShapeFont.hxx"

#include <vos/mutex.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
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
        case SID_ATTR_TRANSFORM:
        case SID_CHAR_DLG:
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
        case SID_ATTR_TRANSFORM:
            {
                executeDispatch_TransformDialog();
            }
            break;
        case SID_CHAR_DLG:
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
    implDescribeSupportedFeature( ".uno:TransformDialog",   SID_ATTR_TRANSFORM,   CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FontDialog",        SID_CHAR_DLG,         CommandGroup::EDIT );
}

void ShapeController::executeDispatch_TransformDialog()
{
    if ( m_pChartController )
    {
        Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper )
        {
            SdrObject* pObj = pDrawViewWrapper->getSelectedObject();
            if ( pObj && pObj->GetObjIdentifier() == OBJ_CAPTION )
            {
                // item set for caption
                SfxItemSet aSet( pDrawViewWrapper->GetModel()->GetItemPool() );
                pDrawViewWrapper->GetAttributes( aSet );
                // item set for position and size
                SfxItemSet aGeoSet( pDrawViewWrapper->GetGeoAttrFromMarked() );
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                        pFact->CreateCaptionDialog( pParent, pDrawViewWrapper, RID_SVXDLG_CAPTION ) );
                    const USHORT* pRange = pDlg->GetInputRanges( *aSet.GetPool() );
                    SfxItemSet aCombSet( *aSet.GetPool(), pRange );
                    aCombSet.Put( aSet );
                    aCombSet.Put( aGeoSet );
                    pDlg->SetInputSet( &aCombSet );
                    if ( pDlg->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                        pDrawViewWrapper->SetAttributes( *pOutSet );
                        pDrawViewWrapper->SetGeoAttrToMarked( *pOutSet );
                    }
                }
            }
            else
            {
                SfxItemSet aGeoSet( pDrawViewWrapper->GetGeoAttrFromMarked() );
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    ::boost::scoped_ptr< SfxAbstractTabDialog > pDlg(
                        pFact->CreateSvxTransformTabDialog( pParent, &aGeoSet, pDrawViewWrapper, RID_SVXDLG_TRANSFORM ) );
                    if ( pDlg->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                        pDrawViewWrapper->SetGeoAttrToMarked( *pOutSet );
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
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawModelWrapper && pDrawViewWrapper )
        {
            Window* pParent = dynamic_cast< Window* >( m_pChartController->m_pChartWindow );
            SfxItemSet aSet( pDrawViewWrapper->GetModel()->GetItemPool() );
            pDrawViewWrapper->GetAttributes( aSet );
            ViewElementListProvider aViewElementListProvider( pDrawModelWrapper );
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            ::boost::scoped_ptr< ShapeFontDialog > pDlg( new ShapeFontDialog( pParent, &aSet, &aViewElementListProvider ) );
            if ( pDlg->Execute() == RET_OK )
            {
                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                pDrawViewWrapper->SetAttributes( *pOutSet );
            }
        }
    }
}

//.............................................................................
} //  namespace chart
//.............................................................................
