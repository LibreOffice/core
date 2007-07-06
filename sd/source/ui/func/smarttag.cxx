/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smarttag.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 13:13:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "ViewShell.hxx"
#include "smarttag.hxx"
#include "Window.hxx"
#include "View.hxx"

namespace sd
{

// ====================================================================

SmartTag::SmartTag( ::sd::View& rView )
: mrView( rView )
, mbSelected( false )
{
    SmartTagReference xThis( this );
    mrView.getSmartTags().add( xThis );
}

// --------------------------------------------------------------------

SmartTag::~SmartTag()
{
}

// --------------------------------------------------------------------

bool SmartTag::MouseButtonDown( const MouseEvent&, SmartHdl&  )
{
    return false;
}

/** returns true if the SmartTag consumes this event. */
bool SmartTag::KeyInput( const KeyEvent& /*rKEvt*/ )
{
    return false;
}

// --------------------------------------------------------------------

void SmartTag::addCustomHandles( SdrHdlList& /*rHandlerList*/ )
{
}

// --------------------------------------------------------------------

void SmartTag::select()
{
    mbSelected = true;
}

// --------------------------------------------------------------------

void SmartTag::deselect()
{
    mbSelected = false;
}

// --------------------------------------------------------------------

bool SmartTag::isSelected() const
{
    return mbSelected;
}

// --------------------------------------------------------------------

void SmartTag::disposing()
{
    SmartTagReference xThis( this );
    mrView.getSmartTags().remove( xThis );
}

// --------------------------------------------------------------------

bool SmartTag::getContext( SdrViewContext& /*rContext*/ )
{
    return false;
}

// --------------------------------------------------------------------

ULONG SmartTag::GetMarkablePointCount() const
{
    return 0;
}

// --------------------------------------------------------------------

ULONG SmartTag::GetMarkedPointCount() const
{
    return 0;
}

// --------------------------------------------------------------------

BOOL SmartTag::MarkPoint(SdrHdl& /*rHdl*/, BOOL /*bUnmark*/ )
{
    return FALSE;
}

// --------------------------------------------------------------------

BOOL SmartTag::MarkPoints(const Rectangle* /*pRect*/, BOOL /*bUnmark*/ )
{
    return FALSE;
}

// --------------------------------------------------------------------

void SmartTag::CheckPossibilities()
{
}

// ====================================================================

SmartTagSet::SmartTagSet( View& rView )
: mrView( rView )
{
}

// --------------------------------------------------------------------

SmartTagSet::~SmartTagSet()
{
}

// --------------------------------------------------------------------

void SmartTagSet::add( const SmartTagReference& xTag )
{
    maSet.insert( xTag );
}

// --------------------------------------------------------------------

void SmartTagSet::remove( const SmartTagReference& xTag )
{
    std::set< SmartTagReference >::iterator aIter( maSet.find( xTag ) );
    if( aIter != maSet.end() )
        maSet.erase( aIter );
}

// --------------------------------------------------------------------

void SmartTagSet::Dispose()
{
    std::set< SmartTagReference > aSet;
    aSet.swap( maSet );
    for( std::set< SmartTagReference >::iterator aIter( aSet.begin() ); aIter != aSet.end(); )
        (*aIter++)->Dispose();
}

// --------------------------------------------------------------------

void SmartTagSet::select( const SmartTagReference& xTag )
{
    if( mxSelectedTag != xTag )
    {
        if( mxSelectedTag.is() )
            mxSelectedTag->deselect();

        mxSelectedTag = xTag;
        mxSelectedTag->select();
        mrView.SetPossibilitiesDirty();
        if( mrView.GetMarkedObjectCount() > 0 )
            mrView.UnmarkAllObj();
        else
            mrView.updateHandles();
    }
}

// --------------------------------------------------------------------

void SmartTagSet::deselect()
{
    if( mxSelectedTag.is() )
    {
        mxSelectedTag->deselect();
        mxSelectedTag.clear();
        mrView.SetPossibilitiesDirty();
        mrView.updateHandles();
    }
}

// --------------------------------------------------------------------

bool SmartTagSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aMDPos( mrView.GetViewShell()->GetActiveWindow()->PixelToLogic( rMEvt.GetPosPixel() ) );
    SdrHdl* pHdl = mrView.PickHandle(aMDPos);

    // check if a smart tag is selected and no handle is hit
    if( mxSelectedTag.is() && !pHdl )
    {
        // deselect smart tag
        deselect();
        return false;
    }

    // if a smart tag handle is hit, foreward event to its smart tag
    SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( pHdl );
    if(pSmartHdl && pSmartHdl->getTag().is() )
    {
        SmartTagReference xTag( pSmartHdl->getTag() );
        return xTag->MouseButtonDown( rMEvt, *pSmartHdl );
    }

    return false;
}

// --------------------------------------------------------------------

bool SmartTagSet::KeyInput( const KeyEvent& rKEvt )
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->KeyInput( rKEvt );
    else
        return false;
}

// --------------------------------------------------------------------

void SmartTagSet::addCustomHandles( SdrHdlList& rHandlerList )
{
    if( !maSet.empty() )
    {
        for( std::set< SmartTagReference >::iterator aIter( maSet.begin() ); aIter != maSet.end(); )
            (*aIter++)->addCustomHandles( rHandlerList );
    }
}

// --------------------------------------------------------------------

/** returns true if the currently selected smart tag has
    a special context, returned in rContext. */
bool SmartTagSet::getContext( SdrViewContext& rContext ) const
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->getContext( rContext );
    else
        return false;
}

// --------------------------------------------------------------------
// support point editing
// --------------------------------------------------------------------

BOOL SmartTagSet::HasMarkablePoints() const
{
    return GetMarkablePointCount() != 0 ? TRUE : FALSE;
}

// --------------------------------------------------------------------

ULONG SmartTagSet::GetMarkablePointCount() const
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->GetMarkablePointCount();
    return 0;
}

// --------------------------------------------------------------------

BOOL SmartTagSet::HasMarkedPoints() const
{
    return GetMarkedPointCount() != 0 ? TRUE : FALSE;
}

// --------------------------------------------------------------------

ULONG SmartTagSet::GetMarkedPointCount() const
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->GetMarkedPointCount();
    else
        return 0;
}

// --------------------------------------------------------------------

BOOL SmartTagSet::IsPointMarkable(const SdrHdl& rHdl) const
{
    const SmartHdl* pSmartHdl = dynamic_cast< const SmartHdl* >( &rHdl );

    return pSmartHdl && pSmartHdl->isMarkable();
}

// --------------------------------------------------------------------

BOOL SmartTagSet::MarkPoint(SdrHdl& rHdl, BOOL bUnmark )
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->MarkPoint( rHdl, bUnmark );

    return FALSE;
}

// --------------------------------------------------------------------

BOOL SmartTagSet::MarkPoints(const Rectangle* pRect, BOOL bUnmark)
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->MarkPoints( pRect, bUnmark );
    return FALSE;
}

// --------------------------------------------------------------------

void SmartTagSet::CheckPossibilities()
{
    if( mxSelectedTag.is() )
        mxSelectedTag->CheckPossibilities();
}

// ====================================================================

SmartHdl::SmartHdl( const SmartTagReference& xTag, SdrObject* pObject, const Point& rPnt, SdrHdlKind eNewKind /*=HDL_MOVE*/ )
: SdrHdl( rPnt, eNewKind )
, mxTag( xTag )
{
    SetObj( pObject );
}

// --------------------------------------------------------------------

SmartHdl::SmartHdl( const SmartTagReference& xTag, const Point& rPnt, SdrHdlKind eNewKind /*=HDL_MOVE*/ )
: SdrHdl( rPnt, eNewKind )
, mxTag( xTag )
{
}

// --------------------------------------------------------------------

bool SmartHdl::isMarkable() const
{
    return false;
}

// ====================================================================

/*
SmartProxyHdl::SmartProxyHdl( const SmartTagReference& xTag, SdrHdl* pProxyHdl )
: SmartHdl( xTag, pProxyHdl->GetPos(), pProxyHdl->GetKind() )
, mpProxyHdl( pProxyHdl )
{
}

// --------------------------------------------------------------------

SmartProxyHdl::~SmartProxyHdl()
{
    delete mpProxyHdl;
}
*/
} // end of namespace sd

