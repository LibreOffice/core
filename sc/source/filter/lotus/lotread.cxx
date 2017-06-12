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

#include "document.hxx"

#include "scerrors.hxx"
#include "root.hxx"
#include "lotfilter.hxx"
#include "lotimpop.hxx"
#include "lotattr.hxx"
#include "fprogressbar.hxx"

class ScFormulaCell;

ErrCode ImportLotus::Read()
{
    enum STATE
    {
        S_START,        // analyse first BOF
        S_WK1,          // in WK1-Stream
        S_WK3,          // in WK3-Section
        S_WK4,
        S_FM3,
        S_END           // Import finished
    };

    sal_uInt16          nOp;
    sal_uInt16          nSubType;
    sal_uInt16          nRecLen;
    sal_uInt32          nNextRec = 0UL;
    ErrCode        eRet = ERRCODE_NONE;
//  ScFormulaCell   *pLastFormCell;

    STATE           eAkt = S_START;

    nTab = 0;
    nExtTab = -2;

    pIn->Seek( nNextRec );

    // start progressbar
    ScfStreamProgressBar aPrgrsBar( *pIn, pD->GetDocumentShell() );
    LotusContext &rContext = aConv.getContext();
    while( eAkt != S_END )
    {
        pIn->ReadUInt16( nOp ).ReadUInt16( nRecLen );

        if( pIn->IsEof() || nNextRec > SAL_MAX_UINT32 - nRecLen - 4 )
            eAkt = S_END;

        nNextRec += nRecLen + 4;

        switch( eAkt )
        {

            case S_START:                                           // S_START
            if( nOp )
            {
                eRet = SCERR_IMPORT_UNKNOWN_WK;
                eAkt = S_END;
            }
            else
            {
                if( nRecLen > 2 )
                {
                    Bof();
                    switch (rContext.pLotusRoot->eFirstType)
                    {
                        case Lotus123Typ::WK3: eAkt = S_WK3; break;
                        case Lotus123Typ::WK4: eAkt = S_WK4; break;
                        default:
                        eRet = SCERR_IMPORT_UNKNOWN_WK;
                        eAkt = S_END;
                    }
                }
                else
                {
                    eAkt = S_END;                       // TODO: add here something for <= WK1!
                    eRet = ErrCode(0xFFFFFFFF);
                }
            }
            break;

            case S_WK1:                                 // S_WK1
            break;

            case S_WK3:                                 // S_WK3
            case S_WK4:                                 // S_WK4
            switch( nOp )
            {
                case 0x0001:                            // EOF
                eAkt = S_FM3;
                nTab++;
                break;

                case 0x0002:                            // PASSWORD
                eRet = SCERR_IMPORT_FILEPASSWD;
                eAkt = S_END;
                break;

                case 0x0007:                            // COLUMNWIDTH
                Columnwidth( nRecLen );
                break;

                case 0x0008:                            // HIDDENCOLUMN
                Hiddencolumn( nRecLen );
                break;

                case 0x0009:                            // USERRANGE
                Userrange();
                break;

                case 0x0013:                            // FORMAT

                break;
                case 0x0014:                            // ERRCELL
                Errcell();
                break;

                case 0x0015:                            // NACELL
                Nacell();
                break;

                case 0x0016:                            // LABELCELL
                Labelcell();
                break;

                case 0x0017:                            // NUMBERCELL
                Numbercell();
                break;

                case 0x0018:                            // SMALLNUMCELL
                Smallnumcell();
                break;

                case 0x0019:                            // FORMULACELL
                Formulacell( nRecLen );
                break;

                case 0x001b:                            // extended attributes
                if (nRecLen > 2)
                {
                    Read( nSubType );
                    nRecLen -= 2;
                    switch( nSubType )
                    {
                        case 2007:                      // ROW PRESENTATION
                            RowPresentation( nRecLen );
                            break;

                        case 14000:                     // NAMED SHEET
                            NamedSheet();
                            break;
                    }
                }
                else
                {
                    eRet = SCERR_IMPORT_FORMAT;
                    eAkt = S_END;
                }
            }

            break;

            case S_FM3:                                 // S_FM3
            break;

            case S_END:                                 // S_END
            break;
        }

        OSL_ENSURE( nNextRec >= pIn->Tell(),
            "*ImportLotus::Read(): Read too much..." );

        pIn->Seek( nNextRec );
        aPrgrsBar.Progress();
    }

    // TODO: eliminate stupid names
    SCTAB       nTabs = pD->GetTableCount();
    SCTAB       nCnt;
    OUString aTabName;
    OUString aBaseName;
    OUString aRef( "temp" );
    if( nTabs != 0 )
    {
        if( nTabs > 1 )
        {
            pD->GetName( 0, aBaseName );
            aBaseName = aBaseName.copy(0, aBaseName.getLength()-1);
        }
        for( nCnt = 1 ; nCnt < nTabs ; nCnt++ )
        {
            OSL_ENSURE( pD->HasTable( nCnt ),
                "-ImportLotus::Read(): Where is my table?!" );
            pD->GetName( nCnt, aTabName );
            if( aTabName.equals(aRef) )
            {
                aTabName = aBaseName;
                pD->CreateValidTabName( aTabName );
                pD->RenameTab( nCnt, aTabName );
            }
        }
    }

    pD->CalcAfterLoad();

    return eRet;
}

ErrCode ImportLotus::Read(SvStream& rIn)
{
    pIn = &rIn;

    bool            bRead = true;
    sal_uInt16          nOp;
    sal_uInt16          nRecLen;
    sal_uInt32          nNextRec = 0UL;
    ErrCode             eRet = ERRCODE_NONE;

    nTab = 0;
    nExtTab = -1;

    pIn->Seek( nNextRec );

    // start progressbar
    ScfStreamProgressBar aPrgrsBar( *pIn, pD->GetDocumentShell() );
    LotusContext &rContext = aConv.getContext();
    while( bRead )
    {
        pIn->ReadUInt16( nOp ).ReadUInt16( nRecLen );

        if( pIn->IsEof() || nNextRec > SAL_MAX_UINT32 - nRecLen - 4 )
            bRead = false;
        else
        {
            nNextRec += nRecLen + 4;

            switch( nOp )
            {
                case 0x0000:                            // BOF
                if( nRecLen != 26 || !BofFm3() )
                {
                    bRead = false;
                    eRet = SCERR_IMPORT_FORMAT;
                }
                break;

                case 0x0001:                            // EOF
                    bRead = false;
                    OSL_ENSURE( nTab == 0,
                        "-ImportLotus::Read( SvStream& ): EOF twice!" );
                    nTab++;
                break;

                case 174:                               // FONT_FACE
                    Font_Face();
                break;

                case 176:                               // FONT_TYPE
                    Font_Type();
                break;

                case 177:                               // FONT_YSIZE
                    Font_Ysize();
                break;

                case 195:
                    if( nExtTab >= 0 )
                        rContext.pLotusRoot->maAttrTable.Apply(rContext.pLotusRoot, (SCTAB)nExtTab);
                    nExtTab++;
                    break;
                case 197:
                    Row_( nRecLen );
                break;
            }

            OSL_ENSURE( nNextRec >= pIn->Tell(),
                "*ImportLotus::Read(): Read too much..." );
            pIn->Seek( nNextRec );
            aPrgrsBar.Progress();
        }
    }

    rContext.pLotusRoot->maAttrTable.Apply(rContext.pLotusRoot, (SCTAB)nExtTab);

    return eRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
