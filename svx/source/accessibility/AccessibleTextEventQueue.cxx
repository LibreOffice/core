/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleTextEventQueue.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:03:28 $
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
#include "precompiled_svx.hxx"

#ifndef _SVX_TEXT_CHANGED_QUEUE_HXX
#include "AccessibleTextEventQueue.hxx"
#endif

#ifndef _SVX_UNOSHAPE_HXX
#include "unoshape.hxx"
#endif

#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif

#ifndef _SVX_UNOTEXT_HXX
#include "unotext.hxx"
#endif

#include "unoedhlp.hxx"
#include "unopracc.hxx"
#include "svdmodel.hxx"
#include "svdpntv.hxx"
#include "editdata.hxx"
#include "editeng.hxx"
#include "editview.hxx"

namespace accessibility
{
    //------------------------------------------------------------------------
    //
    // EventQueue implementation
    //
    //------------------------------------------------------------------------

    AccessibleTextEventQueue::AccessibleTextEventQueue()
    {
    }

    AccessibleTextEventQueue::~AccessibleTextEventQueue()
    {
        Clear();
    }

    void AccessibleTextEventQueue::Append( const SfxHint& rHint )
    {
        maEventQueue.push_back( new SfxHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const SdrHint& rHint )
    {
        maEventQueue.push_back( new SdrHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const SfxSimpleHint& rHint )
    {
        maEventQueue.push_back( new SfxSimpleHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const TextHint& rHint )
    {
        maEventQueue.push_back( new TextHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const SvxViewHint& rHint )
    {
        maEventQueue.push_back( new SvxViewHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const SvxEditSourceHint& rHint )
    {
        maEventQueue.push_back( new SvxEditSourceHint( rHint ) );
    }

    ::std::auto_ptr< SfxHint > AccessibleTextEventQueue::PopFront()
    {
        ::std::auto_ptr< SfxHint > aRes( *(maEventQueue.begin()) );
        maEventQueue.pop_front();
        return aRes;
    }

    bool AccessibleTextEventQueue::IsEmpty() const
    {
        return maEventQueue.empty();
    }

    void AccessibleTextEventQueue::Clear()
    {
        // clear queue
        while( !IsEmpty() )
            PopFront();
    }

} // end of namespace accessibility

//------------------------------------------------------------------------
