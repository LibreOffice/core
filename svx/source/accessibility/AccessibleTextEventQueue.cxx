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
#include "precompiled_svx.hxx"
#include "AccessibleTextEventQueue.hxx"
#include <svx/unoshape.hxx>
#include "editeng/unolingu.hxx"
#include <editeng/unotext.hxx>

#include "editeng/unoedhlp.hxx"
#include "editeng/unopracc.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdpntv.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
