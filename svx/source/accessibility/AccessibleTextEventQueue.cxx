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


    // EventQueue implementation



    AccessibleTextEventQueue::AccessibleTextEventQueue()
    {
    }

    AccessibleTextEventQueue::~AccessibleTextEventQueue()
    {
        Clear();
    }

    void AccessibleTextEventQueue::Append( const SdrHint& rHint )
    {
        maEventQueue.push_back( new SdrHint( rHint ) );
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

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< SfxHint > AccessibleTextEventQueue::PopFront()
    {
        ::std::auto_ptr< SfxHint > aRes( *(maEventQueue.begin()) );
        maEventQueue.pop_front();
        return aRes;
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
