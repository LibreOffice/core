/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

/** returns true if the SmartTag consumes this event. */
bool SmartTag::RequestHelp( const HelpEvent& /*rHEvt*/ )
{
    return false;
}

/** returns true if the SmartTag consumes this event. */
bool SmartTag::Command( const CommandEvent& /*rCEvt*/ )
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

sal_uInt32 SmartTag::GetMarkablePointCount() const
{
    return 0;
}

// --------------------------------------------------------------------

sal_uInt32 SmartTag::GetMarkedPointCount() const
{
    return 0;
}

// --------------------------------------------------------------------

bool SmartTag::MarkPoint(SdrHdl& /*rHdl*/, bool /*bUnmark*/ )
{
    return false;
}

// --------------------------------------------------------------------

bool SmartTag::MarkPoints(const basegfx::B2DRange* /*pRange*/, bool /*bUnmark*/ )
{
    return false;
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
    mrView.InvalidateAllWin();

    if( xTag == mxMouseOverTag )
        mxMouseOverTag.clear();

    if( xTag == mxSelectedTag )
        mxSelectedTag.clear();
}

// --------------------------------------------------------------------

void SmartTagSet::remove( const SmartTagReference& xTag )
{
    std::set< SmartTagReference >::iterator aIter( maSet.find( xTag ) );
    if( aIter != maSet.end() )
        maSet.erase( aIter );
    mrView.InvalidateAllWin();

    if( xTag == mxMouseOverTag )
        mxMouseOverTag.clear();

    if( xTag == mxSelectedTag )
        mxSelectedTag.clear();
}

// --------------------------------------------------------------------

void SmartTagSet::Dispose()
{
    std::set< SmartTagReference > aSet;
    aSet.swap( maSet );
    for( std::set< SmartTagReference >::iterator aIter( aSet.begin() ); aIter != aSet.end(); )
        (*aIter++)->Dispose();
    mrView.InvalidateAllWin();
    mxMouseOverTag.clear();
    mxSelectedTag.clear();
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
        if( mrView.areSdrObjectsSelected() )
            mrView.UnmarkAllObj();
        else
            mrView.RecreateAllMarkHandles();
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
        mrView.RecreateAllMarkHandles();
    }
}

// --------------------------------------------------------------------

bool SmartTagSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aMDPos(mrView.GetViewShell()->GetActiveWindow()->GetInverseViewTransformation() * aPixelPos);
    SdrHdl* pHdl = mrView.PickHandle(aMDPos);

    // check if a smart tag is selected and no handle is hit
    if( mxSelectedTag.is() && !pHdl )
    {
        // deselect smart tag
        deselect();
        return false;
    }

    // if a smart tag handle is hit, foreword event to its smart tag
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
    else if( rKEvt.GetKeyCode().GetCode() == KEY_SPACE )
    {
        SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( mrView.GetHdlList().GetFocusHdl() );
        if( pSmartHdl )
        {
            const_cast< SdrHdlList& >( mrView.GetHdlList() ).ResetFocusHdl();
            SmartTagReference xTag( pSmartHdl->getTag() );
            select( xTag );
            return true;
        }
    }


    return false;
}

// --------------------------------------------------------------------

bool SmartTagSet::RequestHelp( const HelpEvent& rHEvt )
{
    const basegfx::B2DPoint aPixelPos(rHEvt.GetMousePosPixel().X(), rHEvt.GetMousePosPixel().Y());
    const basegfx::B2DPoint aMDPos(mrView.GetViewShell()->GetActiveWindow()->GetInverseViewTransformation() * aPixelPos);
    SdrHdl* pHdl = mrView.PickHandle(aMDPos);

    if( pHdl )
    {
        // if a smart tag handle is hit, foreword event to its smart tag
        SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( pHdl );
        if(pSmartHdl && pSmartHdl->getTag().is() )
        {
            SmartTagReference xTag( pSmartHdl->getTag() );
            return xTag->RequestHelp( rHEvt );
        }
    }

    return false;
}

// --------------------------------------------------------------------

/** returns true if the SmartTag consumes this event. */
bool SmartTagSet::Command( const CommandEvent& rCEvt )
{
    if( rCEvt.IsMouseEvent() )
    {
        const basegfx::B2DPoint aPixelPos(rCEvt.GetMousePosPixel().X(), rCEvt.GetMousePosPixel().Y());
        const basegfx::B2DPoint aMDPos(mrView.GetViewShell()->GetActiveWindow()->GetInverseViewTransformation() * aPixelPos);
        SdrHdl* pHdl = mrView.PickHandle(aMDPos);

        if( pHdl )
        {
            // if a smart tag handle is hit, foreword event to its smart tag
            SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( pHdl );
            if(pSmartHdl && pSmartHdl->getTag().is() )
            {
                SmartTagReference xTag( pSmartHdl->getTag() );
                return xTag->Command( rCEvt );
            }
        }
    }
    else
    {
        if( mxSelectedTag.is() )
            return mxSelectedTag->Command( rCEvt );

    }

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

bool SmartTagSet::HasMarkablePoints() const
{
    return 0 != GetMarkablePointCount();
}

// --------------------------------------------------------------------

sal_uInt32 SmartTagSet::GetMarkablePointCount() const
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->GetMarkablePointCount();
    return 0;
}

// --------------------------------------------------------------------

bool SmartTagSet::HasMarkedPoints() const
{
    return 0 != GetMarkedPointCount();
}

// --------------------------------------------------------------------

sal_uInt32 SmartTagSet::GetMarkedPointCount() const
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->GetMarkedPointCount();
    else
        return 0;
}

// --------------------------------------------------------------------

bool SmartTagSet::IsPointMarkable(const SdrHdl& rHdl) const
{
    const SmartHdl* pSmartHdl = dynamic_cast< const SmartHdl* >( &rHdl );

    return pSmartHdl && pSmartHdl->isMarkable();
}

// --------------------------------------------------------------------

bool SmartTagSet::MarkPoint(SdrHdl& rHdl, bool bUnmark )
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->MarkPoint( rHdl, bUnmark );

    return false;
}

// --------------------------------------------------------------------

bool SmartTagSet::MarkPoints(const basegfx::B2DRange* pRange, bool bUnmark)
{
    if( mxSelectedTag.is() )
    {
        return mxSelectedTag->MarkPoints( pRange, bUnmark );
    }

    return false;
}

// --------------------------------------------------------------------

void SmartTagSet::CheckPossibilities()
{
    if( mxSelectedTag.is() )
        mxSelectedTag->CheckPossibilities();
}

// ====================================================================

SmartHdl::SmartHdl(
    SdrHdlList& rHdlList,
    const SdrObject* pSdrHdlObject,
    const SmartTagReference& xTag,
    SdrHdlKind eNewKind,
    const basegfx::B2DPoint& rPnt)
: SdrHdl( rHdlList, pSdrHdlObject, eNewKind, rPnt )
, mxTag( xTag )
{
}

SmartHdl::SmartHdl(
    SdrHdlList& rHdlList,
    const SdrObject* pSdrHdlObject,
    const SmartTagReference& xTag,
    sal_uInt32 nObjHandleNum,
    SdrHdl& rSource)
: SdrHdl( rHdlList, pSdrHdlObject, rSource.GetKind(), rSource.getPosition())
, mxTag( xTag )
{
    mnObjHdlNum = nObjHandleNum;
    mnPolyNum = rSource.GetPolyNum();
    mnPPntNum = rSource.GetPointNum();
    mbPlusHdl = rSource.IsPlusHdl();
    mnSourceHdlNum = rSource.GetSourceHdlNum();
}

SmartHdl::~SmartHdl()
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

