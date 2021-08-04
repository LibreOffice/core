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


#include <accel.h>
#include <accel.hxx>
#include <accmgr.hxx>

#include <algorithm>

ImplAccelManager::~ImplAccelManager()
{
}

bool ImplAccelManager::InsertAccel( Accelerator* pAccel )
{
    if ( !mxAccelList ) {
        mxAccelList.emplace();
    } else {
        for (Accelerator* i : *mxAccelList) {
            if ( i == pAccel ) {
                return false;
            }
        }
    }

    mxAccelList->insert( mxAccelList->begin(), pAccel );
    return true;
}

void ImplAccelManager::RemoveAccel( Accelerator const * pAccel )
{
    // do we have a list ?
    if ( !mxAccelList )
        return;

    //e.g. #i90599#. Someone starts typing a sequence in a dialog, but doesn't
    //end it, and then closes the dialog, deleting the accelerators. So if
    //we're removing an accelerator that a sub-accelerator which is in the
    //sequence list, throw away the entire sequence
    if ( mxSequenceList ) {
        for (sal_uInt16 i = 0; i < pAccel->GetItemCount(); ++i) {
            Accelerator* pSubAccel = pAccel->GetAccel( pAccel->GetItemId(i) );
            for (Accelerator* j : *mxSequenceList) {
                if ( j == pSubAccel ) {
                    EndSequence();
                    i = pAccel->GetItemCount();
                    break;
                }
            }
        }
    }

    // throw it away
    auto it = std::find(mxAccelList->begin(), mxAccelList->end(), pAccel);
    if (it != mxAccelList->end())
        mxAccelList->erase( it );
}

void ImplAccelManager::EndSequence()
{
    // are we in a list ?
    if ( !mxSequenceList )
        return;

    for (Accelerator* pTempAccel : *mxSequenceList)
    {
        pTempAccel->mpDel = nullptr;
    }

    // delete sequence-list
    mxSequenceList.reset();
}

bool ImplAccelManager::IsAccelKey( const vcl::KeyCode& rKeyCode )
{
    Accelerator* pAccel;

    // do we have accelerators ??
    if ( !mxAccelList )
        return false;
    if ( mxAccelList->empty() )
        return false;

    // are we in a sequence ?
    if ( mxSequenceList )
    {
        pAccel = mxSequenceList->empty() ? nullptr : (*mxSequenceList)[ 0 ];

        // not found ?
        if ( !pAccel )
        {
            // abort sequence
            FlushAccel();
            return false;
        }

        // can the entry be found ?
        ImplAccelEntry* pEntry = pAccel->ImplGetAccelData( rKeyCode );
        if ( pEntry )
        {
            Accelerator* pNextAccel = pEntry->mpAccel;

            // is an accelerator coupled ?
            if ( pNextAccel )
            {

                mxSequenceList->insert( mxSequenceList->begin(), pNextAccel );

                // call Activate-Handler of the new one
                pNextAccel->Activate();
                return true;
            }
            else
            {
                // it is there already !
                if ( pEntry->mbEnabled )
                {
                    // stop sequence (first call deactivate-handler)
                    EndSequence();

                    // set accelerator of the actual item
                    // and call the handler
                    bool bDel = false;
                    pAccel->mnCurId         = pEntry->mnId;
                    pAccel->mpDel           = &bDel;
                    pAccel->Select();

                    // did the accelerator survive the call
                    if ( !bDel )
                    {
                        pAccel->mnCurId         = 0;
                        pAccel->mpDel           = nullptr;
                    }

                    return true;
                }
                else
                {
                    // stop sequence as the accelerator was disabled
                    // transfer the key (to the system)
                    FlushAccel();
                    return false;
                }
            }
        }
        else
        {
            // wrong key => stop sequence
            FlushAccel();
            return false;
        }
    }

    // step through the list of accelerators
    for (Accelerator* i : *mxAccelList)
    {
        pAccel = i;

        // is the entry contained ?
        ImplAccelEntry* pEntry = pAccel->ImplGetAccelData( rKeyCode );
        if ( pEntry )
        {
            Accelerator* pNextAccel = pEntry->mpAccel;

            // is an accelerator assigned ?
            if ( pNextAccel )
            {

                // create sequence list
                mxSequenceList.emplace();
                mxSequenceList->insert( mxSequenceList->begin(), pAccel     );
                mxSequenceList->insert( mxSequenceList->begin(), pNextAccel );

                // call activate-Handler of the new one
                pNextAccel->Activate();

                return true;
            }
            else
            {
                // already assigned !
                if ( pEntry->mbEnabled )
                {
                    // first call activate/deactivate-Handler
                    pAccel->Activate();

                    // define accelerator of the actual item
                    // and call the handler
                    bool bDel = false;
                    pAccel->mnCurId         = pEntry->mnId;
                    pAccel->mpDel           = &bDel;
                    pAccel->Select();

                    // if the accelerator did survive the call
                    if ( !bDel )
                    {
                        pAccel->mnCurId         = 0;
                        pAccel->mpDel           = nullptr;
                    }

                    return true;
                }
                else
                    return false;
            }
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
