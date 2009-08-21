/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DrawCommandDispatch.cxx,v $
 * $Revision: 1.4 $
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

#include "DrawCommandDispatch.hxx"
#include "ChartController.hxx"
#include "DrawViewWrapper.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "macros.hxx"

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/svxids.hrc>
#include <svx/unoapi.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;


//.............................................................................
namespace
{
//.............................................................................

    // comparing two PropertyValue instances
    struct PropertyValueCompare : public ::std::binary_function< beans::PropertyValue, ::rtl::OUString, bool >
    {
        bool operator() ( const beans::PropertyValue& rPropValue, const ::rtl::OUString& rName ) const
        {
            return rPropValue.Name.equals( rName );
        }
        bool operator() ( const ::rtl::OUString& rName, const beans::PropertyValue& rPropValue ) const
        {
            return rName.equals( rPropValue.Name );
        }
    };

//.............................................................................
} // anonymous namespace
//.............................................................................


//.............................................................................
namespace chart
{
//.............................................................................

DrawCommandDispatch::DrawCommandDispatch( const Reference< uno::XComponentContext >& rxContext,
    ChartController* pController )
    :FeatureCommandDispatchBase( rxContext )
    ,m_pChartController( pController )
{
}

DrawCommandDispatch::~DrawCommandDispatch()
{
}

void DrawCommandDispatch::initialize()
{
    FeatureCommandDispatchBase::initialize();
}

// WeakComponentImplHelperBase
void DrawCommandDispatch::disposing()
{
}

// XEventListener
void DrawCommandDispatch::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
}

FeatureState DrawCommandDispatch::getState( const ::rtl::OUString& rCommand )
{
    FeatureState aReturn;
    aReturn.bEnabled = false;
    aReturn.aState <<= false;
    sal_uInt16 nFeatureId = m_aSupportedFeatures[ rCommand ].nFeatureId;

    switch ( nFeatureId )
    {
        case SID_OBJECT_SELECT:
        case SID_DRAW_LINE:
        case SID_DRAW_RECT:
        case SID_DRAW_ELLIPSE:
        case SID_DRAW_TEXT:
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

void DrawCommandDispatch::execute( const ::rtl::OUString& rCommand, const Sequence< beans::PropertyValue>& rArgs )
{
    (void)rArgs;

    ChartDrawMode eDrawMode = CHARTDRAW_SELECT;
    SdrObjKind eKind = OBJ_NONE;
    bool bCreate = false;
    sal_uInt16 nFeatureId = m_aSupportedFeatures[ rCommand ].nFeatureId;

    switch ( nFeatureId )
    {
        case SID_OBJECT_SELECT:
            {
                eDrawMode = CHARTDRAW_SELECT;
                eKind = OBJ_NONE;
            }
            break;
        case SID_DRAW_LINE:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = OBJ_LINE;
            }
            break;
        case SID_DRAW_RECT:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = OBJ_RECT;
            }
            break;
        case SID_DRAW_ELLIPSE:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = OBJ_CIRC;
            }
            break;
        case SID_DRAW_TEXT:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = OBJ_TEXT;
                bCreate = true;
            }
            break;
        default:
            {
                eDrawMode = CHARTDRAW_SELECT;
                eKind = OBJ_NONE;
            }
            break;
    }

    if ( m_pChartController )
    {
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper )
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            m_pChartController->setDrawMode( eDrawMode );
            setInsertObj( sal::static_int_cast< USHORT >( eKind ) );
            if ( bCreate )
            {
                pDrawViewWrapper->SetCreateMode();
            }

            const ::rtl::OUString sKeyModifier( C2U( "KeyModifier" ) );
            const beans::PropertyValue* pIter = rArgs.getConstArray();
            const beans::PropertyValue* pEnd  = pIter + rArgs.getLength();
            const beans::PropertyValue* pKeyModifier = ::std::find_if(
                pIter, pEnd, ::std::bind2nd( PropertyValueCompare(), boost::cref( sKeyModifier ) ) );
            sal_Int16 nKeyModifier = 0;
            if ( pKeyModifier && ( pKeyModifier->Value >>= nKeyModifier ) && nKeyModifier == KEY_MOD1 )
            {
                if ( eDrawMode == CHARTDRAW_INSERT )
                {
                    SdrObject* pObj = createDefaultObject( nFeatureId );
                    if ( pObj )
                    {
                        SdrPageView* pPageView = pDrawViewWrapper->GetSdrPageView();
                        pDrawViewWrapper->InsertObjectAtView( pObj, *pPageView );
                        Reference< drawing::XShape > xShape( pObj->getUnoShape(), uno::UNO_QUERY );
                        if ( xShape.is() )
                        {
                            m_pChartController->m_aSelection.setSelection( xShape );
                            m_pChartController->m_aSelection.applySelection( pDrawViewWrapper );
                        }
                        if ( nFeatureId == SID_DRAW_TEXT )
                        {
                            m_pChartController->StartTextEdit();
                        }
                    }
                }
            }
        }
    }
}

void DrawCommandDispatch::describeSupportedFeatures()
{
    implDescribeSupportedFeature( ".uno:SelectObject",  SID_OBJECT_SELECT,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Line",          SID_DRAW_LINE,      CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Rect",          SID_DRAW_RECT,      CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Ellipse",       SID_DRAW_ELLIPSE,   CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DrawText",      SID_DRAW_TEXT,      CommandGroup::INSERT );
}

void DrawCommandDispatch::setInsertObj( USHORT eObj, const ::rtl::OUString& rShapeType )
{
    (void)rShapeType;

    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : NULL );
    if ( pDrawViewWrapper )
    {
        pDrawViewWrapper->SetCurrentObj( eObj /*, Inventor */);
    }
}

SdrObject* DrawCommandDispatch::createDefaultObject( const sal_uInt16 nID )
{
    SdrObject* pObj = NULL;
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : NULL );
    DrawModelWrapper* pDrawModelWrapper = ( m_pChartController ? m_pChartController->GetDrawModelWrapper() : NULL );

    if ( pDrawViewWrapper && pDrawModelWrapper )
    {
        Reference< drawing::XDrawPage > xDrawPage( pDrawModelWrapper->getMainDrawPage() );
        SdrPage* pPage = GetSdrPageFromXDrawPage( xDrawPage );
        if ( pPage )
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            pObj = SdrObjFactory::MakeNewObject( pDrawViewWrapper->GetCurrentObjInventor(),
                pDrawViewWrapper->GetCurrentObjIdentifier(), pPage );
            if ( pObj )
            {
                long nDefaultObjectSizeWidth = 4000;
                long nDefaultObjectSizeHeight = 2500;
                Size aObjectSize( nDefaultObjectSizeWidth, nDefaultObjectSizeHeight );
                Rectangle aPageRect( Rectangle( Point( 0, 0 ), pPage->GetSize() ) );
                Point aObjectPos = aPageRect.Center();
                aObjectPos.X() -= aObjectSize.Width() / 2;
                aObjectPos.Y() -= aObjectSize.Height() / 2;
                Rectangle aRect( aObjectPos, aObjectSize );

                switch ( nID )
                {
                    case SID_DRAW_LINE:
                        {
                            if ( pObj->ISA( SdrPathObj ) )
                            {
                                Point aStart = aRect.TopLeft();
                                Point aEnd = aRect.BottomRight();
                                sal_Int32 nYMiddle( ( aRect.Top() + aRect.Bottom() ) / 2 );
                                basegfx::B2DPolygon aPoly;
                                aPoly.append( basegfx::B2DPoint( aStart.X(), nYMiddle ) );
                                aPoly.append( basegfx::B2DPoint( aEnd.X(), nYMiddle ) );
                                ( dynamic_cast< SdrPathObj* >( pObj ) )->SetPathPoly( basegfx::B2DPolyPolygon( aPoly ) );
                                SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                                pObj->SetMergedItemSet( aSet );
                            }
                        }
                        break;
                    case SID_DRAW_TEXT:
                    case SID_DRAW_TEXT_VERTICAL:
                        {
                            if ( pObj->ISA( SdrTextObj ) )
                            {
                                SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                                if ( pTextObj )
                                {
                                    pTextObj->SetLogicRect( aRect );
                                    BOOL bVertical = ( nID == SID_DRAW_TEXT_VERTICAL );
                                    pTextObj->SetVerticalWriting( bVertical );
                                    if ( bVertical )
                                    {
                                        SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                                        aSet.Put( SdrTextAutoGrowWidthItem( TRUE ) );
                                        aSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                                        aSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_TOP ) );
                                        aSet.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_RIGHT ) );
                                        pTextObj->SetMergedItemSet( aSet );
                                    }
                                }
                            }
                        }
                        break;
                    default:
                        {
                            pObj->SetLogicRect( aRect );
                            SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                            pObj->SetMergedItemSet( aSet );
                        }
                        break;
                }
            }
        }
    }

    return pObj;
}

//.............................................................................
} //  namespace chart
//.............................................................................
