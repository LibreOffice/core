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

#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <fltini.hxx>
#include <w1par.hxx>
#include <swfltopt.hxx>
#include <mdiexp.hxx>
#include <swerror.h>
#include <statstr.hrc>


static sal_uLong WW1_Read_FieldIniFlags()
{
    
    static const sal_Char* aNames[ 1 ] = { "WinWord/WW1F" };
    sal_uInt32 aVal[ 1 ];
    SwFilterOptions aOpt( 1, aNames, aVal );
    sal_uLong nFieldFlags = aVal[ 0 ];

    if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO ) )
    {
        SwFltSetFlag( nFieldFlags, SwFltControlStack::BOOK_TO_VAR_REF );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_DO_ID );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_IN_TEXT );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::ALLOW_FLD_CR );
    }
    return nFieldFlags;
}






sal_uLong WW1Reader::Read(SwDoc& rDoc, const OUString& rBaseURL, SwPaM& rPam, const OUString& /*cName*/)
{
    sal_uLong nRet = ERR_SWG_READ_ERROR;
    OSL_ENSURE(pStrm!=NULL, "W1-Read ohne Stream");
    if (pStrm != NULL)
    {
        sal_Bool bNew = !bInsertMode;           

        
        
        sal_uLong nFieldFlags = WW1_Read_FieldIniFlags();
        Ww1Shell* pRdr = new Ww1Shell( rDoc, rPam, rBaseURL, bNew, nFieldFlags );
        if( pRdr )
        {
            
            Ww1Manager* pMan = new Ww1Manager( *pStrm, nFieldFlags );
            if( pMan )
            {
                if( !pMan->GetError() )
                {
                    ::StartProgress( STR_STATSTR_W4WREAD, 0, 100,
                                        rDoc.GetDocShell() );
                    ::SetProgressState( 0, rDoc.GetDocShell() );
                    
                    *pRdr << *pMan;
                    if( !pMan->GetError() )
                        
                        
                        nRet = 0; 
                    ::EndProgress( rDoc.GetDocShell() );
                }
                else
                {
                    if( pMan->GetFib().GetFIB().fComplexGet() )
                        
                        
                        
                        
                        
                        
                        
                        nRet = ERR_WW6_FASTSAVE_ERR;
                }
            }
            delete pMan;
        }
        delete pRdr;
    }
    Ww1Sprm::DeinitTab();
    return nRet;
}






Ww1Shell::Ww1Shell( SwDoc& rD, SwPaM& rPam, const OUString& rBaseURL, sal_Bool bNew, sal_uLong nFieldFlags)
    : SwFltShell(&rD, rPam, rBaseURL, bNew, nFieldFlags)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
