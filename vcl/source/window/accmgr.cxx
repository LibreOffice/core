/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <tools/debug.hxx>

#include <accel.h>
#include <vcl/accel.hxx>
#include <accmgr.hxx>






ImplAccelManager::~ImplAccelManager()
{
    delete mpAccelList;
    delete mpSequenceList;
}



bool ImplAccelManager::InsertAccel( Accelerator* pAccel )
{
    if ( !mpAccelList ) {
        mpAccelList = new ImplAccelList;
    } else {
        for ( size_t i = 0, n = mpAccelList->size(); i < n; ++i ) {
            if ( (*mpAccelList)[ i ] == pAccel ) {
                return false;
            }
        }
    }

    mpAccelList->insert( mpAccelList->begin(), pAccel );
    return true;
}



void ImplAccelManager::RemoveAccel( Accelerator* pAccel )
{
    
    if ( !mpAccelList )
        return;

    
    
    
    
    if ( mpSequenceList ) {
        for (sal_uInt16 i = 0; i < pAccel->GetItemCount(); ++i) {
            Accelerator* pSubAccel = pAccel->GetAccel( pAccel->GetItemId(i) );
            for ( size_t j = 0, n = mpSequenceList->size(); j < n; ++j ) {
                if ( (*mpSequenceList)[ j ] == pSubAccel ) {
                    EndSequence( true );
                    i = pAccel->GetItemCount();
                    break;
                }
            }
        }
    }

    
    for ( ImplAccelList::iterator it = mpAccelList->begin();
          it != mpAccelList->end();
          ++it
    ) {
        if ( *it == pAccel ) {
            mpAccelList->erase( it );
            break;
        }
    }
}



void ImplAccelManager::EndSequence( bool bCancel )
{
    
    if ( !mpSequenceList )
        return;

    
    for ( size_t i = 0, n = mpSequenceList->size(); i < n; ++i )
    {
        Accelerator* pTempAccel = (*mpSequenceList)[ i ];
        sal_Bool bDel = sal_False;
        pTempAccel->mbIsCancel = bCancel;
        pTempAccel->mpDel = &bDel;
        pTempAccel->Deactivate();
        if ( !bDel )
        {
            pTempAccel->mbIsCancel = false;
            pTempAccel->mpDel = NULL;
        }
    }

    
    delete mpSequenceList;
    mpSequenceList = NULL;
}



bool ImplAccelManager::IsAccelKey( const KeyCode& rKeyCode, sal_uInt16 nRepeat )
{
    Accelerator* pAccel;

    
    if ( !mpAccelList )
        return false;
    if ( mpAccelList->empty() )
        return false;

    
    if ( mpSequenceList )
    {
        pAccel = mpSequenceList->empty() ? NULL : (*mpSequenceList)[ 0 ];

        
        if ( !pAccel )
        {
            
            FlushAccel();
            return false;
        }

        
        ImplAccelEntry* pEntry = pAccel->ImplGetAccelData( rKeyCode );
        if ( pEntry )
        {
            Accelerator* pNextAccel = pEntry->mpAccel;

            
            if ( pNextAccel )
            {

                mpSequenceList->insert( mpSequenceList->begin(), pNextAccel );

                
                pNextAccel->Activate();
                return true;
            }
            else
            {
                
                if ( pEntry->mbEnabled )
                {
                    
                    EndSequence();

                    
                    
                    sal_Bool bDel = sal_False;
                    pAccel->maCurKeyCode    = rKeyCode;
                    pAccel->mnCurId         = pEntry->mnId;
                    pAccel->mnCurRepeat     = nRepeat;
                    pAccel->mpDel           = &bDel;
                    pAccel->Select();

                    
                    if ( !bDel )
                    {
                        pAccel->maCurKeyCode    = KeyCode();
                        pAccel->mnCurId         = 0;
                        pAccel->mnCurRepeat     = 0;
                        pAccel->mpDel           = NULL;
                    }

                    return true;
                }
                else
                {
                    
                    
                    FlushAccel();
                    return false;
                }
            }
        }
        else
        {
            
            FlushAccel();
            return false;
        }
    }

    
    for ( size_t i = 0, n = mpAccelList->size(); i < n; ++i )
    {
        pAccel = (*mpAccelList)[ i ];

        
        ImplAccelEntry* pEntry = pAccel->ImplGetAccelData( rKeyCode );
        if ( pEntry )
        {
            Accelerator* pNextAccel = pEntry->mpAccel;

            
            if ( pNextAccel )
            {

                
                mpSequenceList = new ImplAccelList;
                mpSequenceList->insert( mpSequenceList->begin(), pAccel     );
                mpSequenceList->insert( mpSequenceList->begin(), pNextAccel );

                
                pNextAccel->Activate();

                return true;
            }
            else
            {
                
                if ( pEntry->mbEnabled )
                {
                    
                    pAccel->Activate();
                    pAccel->Deactivate();

                    
                    
                    sal_Bool bDel = sal_False;
                    pAccel->maCurKeyCode    = rKeyCode;
                    pAccel->mnCurId         = pEntry->mnId;
                    pAccel->mnCurRepeat     = nRepeat;
                    pAccel->mpDel           = &bDel;
                    pAccel->Select();

                    
                    if ( !bDel )
                    {
                        pAccel->maCurKeyCode    = KeyCode();
                        pAccel->mnCurId         = 0;
                        pAccel->mnCurRepeat     = 0;
                        pAccel->mpDel           = NULL;
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
