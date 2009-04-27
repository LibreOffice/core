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

#include <svx/svxids.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{


DrawCommandDispatch::DrawCommandDispatch( const Reference< uno::XComponentContext >& rxContext,
    ChartController* pController )
    :CommandDispatch( rxContext )
    ,m_pChartController( pController )
{
    fillSupportedFeatures();
}

DrawCommandDispatch::~DrawCommandDispatch()
{
}

void DrawCommandDispatch::initialize()
{
}

bool DrawCommandDispatch::isFeatureSupported( const util::URL& rURL ) const
{
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( rURL.Complete );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        return true;
    }
    return false;
}

void DrawCommandDispatch::fireStatusEvent( const ::rtl::OUString& rURL,
    const Reference< frame::XStatusListener >& xSingleListener /* = 0 */ )
{
    if ( rURL.getLength() == 0 )
    {
        SupportedFeatures::const_iterator aEnd( m_aSupportedFeatures.end() );
        for ( SupportedFeatures::const_iterator aIter( m_aSupportedFeatures.begin() ); aIter != aEnd; ++aIter )
        {
            sal_uInt16 nId = aIter->second.nFeatureId;
            FeatureState aFeatureState( getState( nId ) );
            fireStatusEventForURL( aIter->first, aFeatureState.aState, aFeatureState.bEnabled, xSingleListener );
        }
    }
    else
    {
        sal_uInt16 nId = m_aSupportedFeatures[ rURL ].nFeatureId;
        FeatureState aFeatureState( getState( nId ) );
        fireStatusEventForURL( rURL, aFeatureState.aState, aFeatureState.bEnabled, xSingleListener );
    }
}

// XDispatch
void DrawCommandDispatch::dispatch( const util::URL& URL,
    const Sequence< beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( URL.Complete );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        sal_uInt16 nFeatureId = aIter->second.nFeatureId;
        if ( getState( nFeatureId ).bEnabled )  // ???
        {
            execute( nFeatureId, Arguments );
        }
    }
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

FeatureState DrawCommandDispatch::getState( sal_uInt16 nId ) const
{
    FeatureState aReturn;

    switch ( nId )
    {
        case SID_OBJECT_SELECT:
        case SID_DRAW_LINE:
        case SID_DRAW_RECT:
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

void DrawCommandDispatch::execute( sal_uInt16 nId, const Sequence< beans::PropertyValue>& rArgs )
{
    (void)rArgs;

    ChartDrawMode eDrawMode = CHARTDRAW_SELECT;
    SdrObjKind eKind = OBJ_NONE;
    bool bCreate = false;

    switch ( nId )
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
        m_pChartController->setDrawMode( eDrawMode );
        setInsertObj( sal::static_int_cast< USHORT >( eKind ) );
        if ( bCreate )
        {
            DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
            if ( pDrawViewWrapper )
            {
                pDrawViewWrapper->SetCreateMode();
            }
        }
    }
}

void DrawCommandDispatch::describeSupportedFeatures()
{
    implDescribeSupportedFeature( ".uno:SelectObject",  SID_OBJECT_SELECT,  CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Line",          SID_DRAW_LINE,      CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Rect",          SID_DRAW_RECT,      CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DrawText",      SID_DRAW_TEXT,      CommandGroup::INSERT );
}

void DrawCommandDispatch::implDescribeSupportedFeature( const sal_Char* pAsciiCommandURL,
    sal_uInt16 nId, sal_Int16 nGroup )
{
    ControllerFeature aFeature;
    aFeature.Command = ::rtl::OUString::createFromAscii( pAsciiCommandURL );
    aFeature.nFeatureId = nId;
    aFeature.GroupId = nGroup;

    m_aSupportedFeatures[ aFeature.Command ] = aFeature;
}

void DrawCommandDispatch::fillSupportedFeatures()
{
    describeSupportedFeatures();
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

} //  namespace chart
