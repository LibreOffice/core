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

#include <objectformatterlayfrm.hxx>
#include <anchoredobject.hxx>
#include <sortedobjs.hxx>
#include <layfrm.hxx>
#include <pagefrm.hxx>


#include <layact.hxx>

SwObjectFormatterLayFrm::SwObjectFormatterLayFrm( SwLayoutFrm& _rAnchorLayFrm,
                                                  const SwPageFrm& _rPageFrm,
                                                  SwLayAction* _pLayAction )
    : SwObjectFormatter( _rPageFrm, _pLayAction ),
      mrAnchorLayFrm( _rAnchorLayFrm )
{
}

SwObjectFormatterLayFrm::~SwObjectFormatterLayFrm()
{
}

SwObjectFormatterLayFrm* SwObjectFormatterLayFrm::CreateObjFormatter(
                                                SwLayoutFrm& _rAnchorLayFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction )
{
    if ( !_rAnchorLayFrm.IsPageFrm() &&
         !_rAnchorLayFrm.IsFlyFrm() )
    {
        OSL_FAIL( "<SwObjectFormatterLayFrm::CreateObjFormatter(..)> - unexcepted type of anchor frame " );
        return 0L;
    }

    SwObjectFormatterLayFrm* pObjFormatter = 0L;

    
    
    if ( _rAnchorLayFrm.GetDrawObjs() ||
         ( _rAnchorLayFrm.IsPageFrm() &&
            static_cast<SwPageFrm&>(_rAnchorLayFrm).GetSortedObjs() ) )
    {
        pObjFormatter =
            new SwObjectFormatterLayFrm( _rAnchorLayFrm, _rPageFrm, _pLayAction );
    }

    return pObjFormatter;
}

SwFrm& SwObjectFormatterLayFrm::GetAnchorFrm()
{
    return mrAnchorLayFrm;
}



bool SwObjectFormatterLayFrm::DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                           const bool )
{
    _FormatObj( _rAnchoredObj );

    
    
    return GetLayAction() ? !GetLayAction()->IsAgain() : true;
}

bool SwObjectFormatterLayFrm::DoFormatObjs()
{
    bool bSuccess( true );

    bSuccess = _FormatObjsAtFrm();

    if ( bSuccess && GetAnchorFrm().IsPageFrm() )
    {
        
        
        
        
        bSuccess = _AdditionalFormatObjsOnPage();
    }

    return bSuccess;
}

/** method to format all anchored objects, which are registered at
    the page frame, whose 'anchor' isn't on this page frame and whose
    anchor frame is valid.

    OD 2004-07-02 #i28701#
*/
bool SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()
{
    if ( !GetAnchorFrm().IsPageFrm() )
    {
        OSL_FAIL( "<SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()> - mis-usage of method, call only for anchor frames of type page frame" );
        return true;
    }

    
    
    if ( GetLayAction() && GetLayAction()->IsAgain() )
    {
        return false;
    }


    SwPageFrm& rPageFrm = static_cast<SwPageFrm&>(GetAnchorFrm());

    if ( !rPageFrm.GetSortedObjs() )
    {
        
        return true;
    }

    bool bSuccess( true );

    sal_uInt32 i = 0;
    for ( ; i < rPageFrm.GetSortedObjs()->Count(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*rPageFrm.GetSortedObjs())[i];

        
        
        if ( pAnchoredObj->GetAnchorFrm()->FindFlyFrm() )
        {
            continue;
        }
        
        
        
        SwPageFrm* pPageFrmOfAnchor = pAnchoredObj->FindPageFrmOfAnchor();
        
        
        OSL_ENSURE( pPageFrmOfAnchor,
                "<SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()> - missing page frame" );
        if ( pPageFrmOfAnchor &&
             
             pPageFrmOfAnchor->GetPhyPageNum() < rPageFrm.GetPhyPageNum() )
        {
            
            
            if ( !DoFormatObj( *pAnchoredObj ) )
            {
                bSuccess = false;
                break;
            }

            
            
            if ( !rPageFrm.GetSortedObjs() ||
                 i > rPageFrm.GetSortedObjs()->Count() )
            {
                break;
            }
            else
            {
                sal_uInt32 nActPosOfObj =
                    rPageFrm.GetSortedObjs()->ListPosOf( *pAnchoredObj );
                if ( nActPosOfObj == rPageFrm.GetSortedObjs()->Count() ||
                     nActPosOfObj > i )
                {
                    --i;
                }
                else if ( nActPosOfObj < i )
                {
                    i = nActPosOfObj;
                }
            }
        }
    } 

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
