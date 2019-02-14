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

#include <ViewShell.hxx>
#include <smarttag.hxx>
#include <Window.hxx>
#include <View.hxx>

namespace sd
{

SmartTag::SmartTag( ::sd::View& rView )
: mrView( rView )
, mbSelected( false )
{
    SmartTagReference xThis( this );
    mrView.getSmartTags().add( xThis );
}

SmartTag::~SmartTag()
{
}

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

void SmartTag::addCustomHandles( SdrHdlList& /*rHandlerList*/ )
{
}

void SmartTag::select()
{
    mbSelected = true;
}

void SmartTag::deselect()
{
    mbSelected = false;
}

void SmartTag::disposing()
{
    SmartTagReference xThis( this );
    mrView.getSmartTags().remove( xThis );
}

bool SmartTag::getContext( SdrViewContext& /*rContext*/ )
{
    return false;
}

sal_Int32 SmartTag::GetMarkablePointCount() const
{
    return 0;
}

sal_Int32 SmartTag::GetMarkedPointCount() const
{
    return 0;
}

bool SmartTag::MarkPoint(SdrHdl& /*rHdl*/, bool /*bUnmark*/ )
{
    return false;
}

bool SmartTag::MarkPoints(const ::tools::Rectangle* /*pRect*/, bool /*bUnmark*/ )
{
    return false;
}

void SmartTag::CheckPossibilities()
{
}

SmartTagSet::SmartTagSet( View& rView )
: mrView( rView )
{
}

SmartTagSet::~SmartTagSet()
{
}

void SmartTagSet::add( const SmartTagReference& xTag )
{
    maSet.insert( xTag );
    mrView.InvalidateAllWin();

    if( xTag == mxMouseOverTag )
        mxMouseOverTag.clear();

    if( xTag == mxSelectedTag )
        mxSelectedTag.clear();
}

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

void SmartTagSet::Dispose()
{
    std::set< SmartTagReference > aSet;
    aSet.swap( maSet );
    for( auto& rxItem : aSet )
        rxItem->Dispose();
    mrView.InvalidateAllWin();
    mxMouseOverTag.clear();
    mxSelectedTag.clear();
}

void SmartTagSet::select( const SmartTagReference& xTag )
{
    if( mxSelectedTag == xTag )
        return;

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

    // if a smart tag handle is hit, forward event to its smart tag
    SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( pHdl );
    if(pSmartHdl && pSmartHdl->getTag().is() )
    {
        SmartTagReference xTag( pSmartHdl->getTag() );
        return xTag->MouseButtonDown( rMEvt, *pSmartHdl );
    }

    return false;
}

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
            const SmartTagReference& xTag( pSmartHdl->getTag() );
            select( xTag );
            return true;
        }
    }

    return false;
}

bool SmartTagSet::RequestHelp( const HelpEvent& rHEvt )
{
    Point aMDPos( mrView.GetViewShell()->GetActiveWindow()->PixelToLogic( rHEvt.GetMousePosPixel() ) );
    SdrHdl* pHdl = mrView.PickHandle(aMDPos);

    if( pHdl )
    {
        // if a smart tag handle is hit, forward event to its smart tag
        SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( pHdl );
        if(pSmartHdl && pSmartHdl->getTag().is() )
        {
            const SmartTagReference& xTag( pSmartHdl->getTag() );
            return xTag->RequestHelp( rHEvt );
        }
    }

    return false;
}

/** returns true if the SmartTag consumes this event. */
bool SmartTagSet::Command( const CommandEvent& rCEvt )
{
    if( rCEvt.IsMouseEvent() )
    {
        Point aMDPos( mrView.GetViewShell()->GetActiveWindow()->PixelToLogic( rCEvt.GetMousePosPixel() ) );
        SdrHdl* pHdl = mrView.PickHandle(aMDPos);

        if( pHdl )
        {
            // if a smart tag handle is hit, forward event to its smart tag
            SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( pHdl );
            if(pSmartHdl && pSmartHdl->getTag().is() )
            {
                const SmartTagReference& xTag( pSmartHdl->getTag() );
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

void SmartTagSet::addCustomHandles( SdrHdlList& rHandlerList )
{
    for( auto& rxItem : maSet )
        rxItem->addCustomHandles( rHandlerList );
}

/** returns true if the currently selected smart tag has
    a special context, returned in rContext. */
bool SmartTagSet::getContext( SdrViewContext& rContext ) const
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->getContext( rContext );
    else
        return false;
}

// support point editing

bool SmartTagSet::HasMarkablePoints() const
{
    return GetMarkablePointCount() != 0;
}

sal_uLong SmartTagSet::GetMarkablePointCount() const
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->GetMarkablePointCount();
    return 0;
}

bool SmartTagSet::HasMarkedPoints() const
{
    return GetMarkedPointCount() != 0;
}

sal_uLong SmartTagSet::GetMarkedPointCount() const
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->GetMarkedPointCount();
    else
        return 0;
}

bool SmartTagSet::IsPointMarkable(const SdrHdl& rHdl)
{
    const SmartHdl* pSmartHdl = dynamic_cast< const SmartHdl* >( &rHdl );

    return pSmartHdl && pSmartHdl->isMarkable();
}

bool SmartTagSet::MarkPoint(SdrHdl& rHdl, bool bUnmark )
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->MarkPoint( rHdl, bUnmark );

    return false;
}

bool SmartTagSet::MarkPoints(const ::tools::Rectangle* pRect, bool bUnmark)
{
    if( mxSelectedTag.is() )
        return mxSelectedTag->MarkPoints( pRect, bUnmark );
    return false;
}

void SmartTagSet::CheckPossibilities()
{
    if( mxSelectedTag.is() )
        mxSelectedTag->CheckPossibilities();
}

SmartHdl::SmartHdl( const SmartTagReference& xTag, SdrObject* pObject, const Point& rPnt, SdrHdlKind eNewKind /*=SdrHdlKind::Move*/ )
: SdrHdl( rPnt, eNewKind )
, mxSmartTag( xTag )
{
    SetObj( pObject );
}

SmartHdl::SmartHdl( const SmartTagReference& xTag, const Point& rPnt, SdrHdlKind eNewKind /*=SdrHdlKind::Move*/ )
: SdrHdl( rPnt, eNewKind )
, mxSmartTag( xTag )
{
}

bool SmartHdl::isMarkable() const
{
    return false;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
