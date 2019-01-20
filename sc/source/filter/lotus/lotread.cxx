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

#include <document.hxx>
#include <docoptio.hxx>

#include <scdll.hxx>
#include <scerrors.hxx>
#include <root.hxx>
#include "lotfilter.hxx"
#include <lotimpop.hxx>
#include <lotattr.hxx>
#include <fprogressbar.hxx>

#include <sal/log.hxx>

ErrCode ImportLotus::parse()
{
    enum STATE
    {
        S_START,        // analyse first BOF
        S_WK3,          // in WK3-Section
        S_WK4,
        S_FM3,
        S_END           // Import finished
    };

    sal_uInt16          nOp;
    sal_uInt16          nSubType;
    sal_uInt16          nRecLen;
    sal_uInt32          nNextRec = 0;
    ErrCode        eRet = ERRCODE_NONE;
//  ScFormulaCell   *pLastFormCell;

    STATE               eCurrent = S_START;

    nTab = 0;
    nExtTab = -2;

    pIn->Seek( nNextRec );

    // start progressbar
    ScfStreamProgressBar aPrgrsBar( *pIn, pD->GetDocumentShell() );
    LotusContext &rContext = aConv.getContext();
    while( eCurrent != S_END )
    {
        pIn->ReadUInt16( nOp ).ReadUInt16( nRecLen );

        if (!pIn->good() || nNextRec > SAL_MAX_UINT32 - nRecLen - 4)
        {
            eRet = SCERR_IMPORT_FORMAT;
            eCurrent = S_END;
            if (!pIn->good())
                break;  // while
        }

        nNextRec += nRecLen + 4;

        switch( eCurrent )
        {

            case S_START:                                           // S_START
            if( nOp )
            {
                eRet = SCERR_IMPORT_UNKNOWN_WK;
                eCurrent = S_END;
            }
            else
            {
                if( nRecLen > 2 )
                {
                    Bof();
                    switch (rContext.pLotusRoot->eFirstType)
                    {
                        case Lotus123Typ::WK3: eCurrent = S_WK3; break;
                        case Lotus123Typ::WK4: eCurrent = S_WK4; break;
                        default:
                        eRet = SCERR_IMPORT_UNKNOWN_WK;
                        eCurrent = S_END;
                    }
                }
                else
                {
                    eCurrent = S_END;                   // TODO: add here something for <= WK1!
                    eRet = ErrCode(0xFFFFFFFF);
                }
            }
            break;

            case S_WK3:                                 // S_WK3
            case S_WK4:                                 // S_WK4
            switch( nOp )
            {
                case 0x0001:                            // EOF
                eCurrent = S_FM3;
                nTab++;
                break;

                case 0x0002:                            // PASSWORD
                eRet = SCERR_IMPORT_FILEPASSWD;
                eCurrent = S_END;
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
                    eCurrent = S_END;
                }
            }

            break;

            case S_FM3:                                 // S_FM3
            break;

            case S_END:                                 // S_END
            break;
        }

        SAL_WARN_IF( nNextRec < pIn->Tell(), "sc.filter",
            "*ImportLotus::Read(): Read too much..." );

        pIn->Seek( nNextRec );
        aPrgrsBar.Progress();
    }

    // TODO: eliminate stupid names
    SCTAB       nTabs = pD->GetTableCount();
    SCTAB       nCnt;
    OUString aTabName;
    OUString aBaseName;
    if( nTabs != 0 )
    {
        if( nTabs > 1 )
        {
            pD->GetName( 0, aBaseName );
            aBaseName = aBaseName.copy(0, aBaseName.getLength()-1);
        }
        for( nCnt = 1 ; nCnt < nTabs ; nCnt++ )
        {
            SAL_WARN_IF( !pD->HasTable( nCnt ), "sc.filter",
                "-ImportLotus::Read(): Where is my table?!" );
            pD->GetName( nCnt, aTabName );
            if( aTabName == "temp" )
            {
                aTabName = aBaseName;
                pD->CreateValidTabName( aTabName );
                pD->RenameTab( nCnt, aTabName );
            }
        }
    }

    return eRet;
}

ErrCode ImportLotus::Read()
{
    ErrCode eRet = parse();
    pD->CalcAfterLoad();
    return eRet;
}

ErrCode ImportLotus::Read(SvStream& rIn)
{
    pIn = &rIn;

    bool            bRead = true;
    sal_uInt16          nOp;
    sal_uInt16          nRecLen;
    sal_uInt32          nNextRec = 0;
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

        if (!pIn->good() || nNextRec > SAL_MAX_UINT32 - nRecLen - 4)
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
                    SAL_WARN_IF( nTab != 0, "sc.filter",
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
                        rContext.pLotusRoot->maAttrTable.Apply(rContext.pLotusRoot, static_cast<SCTAB>(nExtTab));
                    nExtTab++;
                    break;
                case 197:
                    Row_( nRecLen );
                break;
            }

            SAL_WARN_IF( nNextRec < pIn->Tell(), "sc.filter",
                "*ImportLotus::Read(): Read too much..." );
            pIn->Seek( nNextRec );
            aPrgrsBar.Progress();
        }
    }

    rContext.pLotusRoot->maAttrTable.Apply(rContext.pLotusRoot, static_cast<SCTAB>(nExtTab));

    return eRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportWKS(SvStream& rStream)
{
    ScDLL::Init();
    ScDocument aDocument;
    ScDocOptions aDocOpt = aDocument.GetDocOptions();
    aDocOpt.SetLookUpColRowNames(false);
    aDocument.SetDocOptions(aDocOpt);
    aDocument.MakeTable(0);
    aDocument.EnableExecuteLink(false);
    aDocument.SetInsertingFromOtherDoc(true);

    LotusContext aContext;
    ImportLotus aLotusImport(aContext, rStream, &aDocument, RTL_TEXTENCODING_ASCII_US);

    ErrCode eRet = aLotusImport.parse();
    if (eRet == ErrCode(0xFFFFFFFF))
    {
        rStream.Seek(0);
        eRet = ScImportLotus123old(aContext, rStream, &aDocument, RTL_TEXTENCODING_ASCII_US);
    }

    return eRet == ERRCODE_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
