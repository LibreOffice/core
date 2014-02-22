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

#include <hintids.hxx>
#include <tools/solar.h>
#include <comphelper/string.hxx>
#include <editeng/paperinf.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/graph.hxx>
#include <vcl/wmf.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/tstpitem.hxx>
#include <svl/urihelper.hxx>
#include <fmtfsize.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <pagedesc.hxx>
#include <flddat.hxx>
#include <reffld.hxx>
#include <expfld.hxx>
#include <docufld.hxx>
#include <ftninfo.hxx>
#include <section.hxx>
#include <fltini.hxx>
#include <w1par.hxx>
#include <docsh.hxx>
#include <swerror.h>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <stdio.h>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <vcl/dibtools.hxx>

#define MAX_FIELDLEN 64000

using namespace css;
using namespace nsSwDocInfoSubType;


















Ww1Shell& operator <<(Ww1Shell& rOut, Ww1Manager& This)
{
    
    if (!This.Pushed())
    {
        {
            
            This.SetInStyle( true );
            Ww1StyleSheet(This.aFib).Out(rOut, This);
            This.SetInStyle( false );
        }
        {
            
            Ww1Assoc(This.aFib).Out(rOut);
        }
        
        This.aDop.Out(rOut);
        
        if (This.GetSep().Count() <= 1)
            rOut.SetUseStdPageDesc();
    }
    
    sal_Unicode cUnknown = ' ';
    while (*This.pSeek < This.pDoc->Count())
    {
        
        
        if (!This.Pushed())
            ::SetProgressState(This.Where() * 100 / This.pDoc->Count(),
             rOut.GetDoc().GetDocShell());
        
        
        
        This.Out(rOut, cUnknown);
        
        
        
        
        cUnknown = This.pDoc->Out(rOut, *This.pSeek);
    }
    This.SetStopAll(true);
    This.OutStop(rOut, cUnknown);   
    This.SetStopAll(false);         
    return rOut;
}

void Ww1Manager::OutStop(Ww1Shell& rOut, sal_Unicode cUnknown)
{
    
    if (pFld)
        pFld->Stop(rOut, *this, cUnknown);
    if (!Pushed())
        aFtn.Stop(rOut, *this, cUnknown);
    if (true)
        aChp.Stop(rOut, *this, cUnknown);
    if (true)
        aPap.Stop(rOut, *this, cUnknown);
    if (!Pushed())
        aSep.Stop(rOut, *this, cUnknown);
}

void Ww1Manager::OutStart( Ww1Shell& rOut )
{
    
    if (!Pushed())
        aSep.Start(rOut, *this);
    if (true)
        aPap.Start(rOut, *this);
    if (true)
        aChp.Start(rOut, *this);
    if (!Pushed())
        aFtn.Start(rOut, *this);
    if (pFld)
        pFld->Start(rOut, *this);
    if (!Pushed())
        aBooks.Start(rOut, *this);
    
    sal_uLong ulEnd = pDoc->Count(); 
    if (!Pushed())
        if (ulEnd > aSep.Where()) 
            ulEnd = aSep.Where();
    if (true)
        if (ulEnd > aPap.Where()) 
            ulEnd = aPap.Where();
    if (true)
        if (ulEnd > aChp.Where()) 
            ulEnd = aChp.Where();
    if (!Pushed())
        if (ulEnd > aFtn.Where()) 
            ulEnd = aFtn.Where();
    if (pFld)
        if (ulEnd > pFld->Where()) 
            ulEnd = pFld->Where();
    if (!Pushed())
        if (ulEnd > aBooks.Where()) 
            ulEnd = aBooks.Where();
    *pSeek = Where(); 
    if (*pSeek < ulEnd) 
        *pSeek = ulEnd;
}

void Ww1Manager::Out(Ww1Shell& rOut, sal_Unicode cUnknown)
{






















    bool bLIsTtp = false;
    sal_Bool bLHasTtp = sal_False;
    if( cUnknown == 0x07 )
    {
        bLIsTtp = IsInTtp();
        bLHasTtp = HasTtp();
    }

    OutStop( rOut, cUnknown );      

    
    if (!Ww1PlainText::IsChar(cUnknown))
        switch (cUnknown)
        {
        case 0x02:
            
        break;
        case 0x07: 
            if (rOut.IsInTable() && HasInTable() && !bLIsTtp && !bLHasTtp)
                rOut.NextTableCell();
        break;
        case 0x09: 
            rOut.NextTab();
        break;
        case 0x0a: 
            rOut.NextParagraph();
        break;
        case 0x0b: 
            if (rOut.IsInTable())
                ;
            else
                rOut.NextLine();
        break;
        case 0x0d: 
            
        break;
        case 0x0c: 
            rOut.NextPage();
        break;
        case 0x14: 
            
        break;
        default:
        break;
        }

    OutStart( rOut );   
}

SvxFontItem Ww1Manager::GetFont(sal_uInt16 nFCode)
{
    return aFonts.GetFont(nFCode);
}

void Ww1Manager::Push0(Ww1PlainText* _pDoc, sal_uLong ulSeek, Ww1Fields* _pFld)
{
    OSL_ENSURE(!Pushed(), "Ww1Manager");
    this->pDoc = _pDoc;
    pSeek = new sal_uLong;
    *pSeek = pDoc->Where();
    aPap.Push(ulSeek);
    aChp.Push(ulSeek);
    this->pFld = _pFld;
}




void Ww1Manager::Push1(Ww1PlainText* _pDoc, sal_uLong ulSeek, sal_uLong ulSeek2,
                       Ww1Fields* _pFld)
{
    OSL_ENSURE(!Pushed(), "Ww1Manager");
    this->pDoc = _pDoc;
    pSeek = new sal_uLong;
    *pSeek = pDoc->Where();
    aPap.Push(ulSeek + ulSeek2);
    aChp.Push(ulSeek + ulSeek2);
    if( _pFld )
        _pFld->Seek( ulSeek2 );
    this->pFld = _pFld;
}

void Ww1Manager::Pop()
{
    OSL_ENSURE(Pushed(), "Ww1Manager");
    delete pDoc;
    pDoc = &aDoc;
    delete pSeek;
    pSeek = &ulDocSeek;
    aChp.Pop();
    aPap.Pop();
    delete pFld;
    pFld = &aFld;
}

void Ww1Bookmarks::Out(Ww1Shell& rOut, Ww1Manager& rMan, sal_uInt16)
{
    if (GetIsEnd())
    {
        rOut.SetBookEnd(GetHandle());
        return;
    }

    const OUString & rName = GetName();
    if( rName.startsWith( "_Toc" ) ) 
        return;

    if( rOut.IsFlagSet( SwFltControlStack::HYPO )
        && rName.equalsIgnoreAsciiCase( "FORMULAR" ) )
        rOut.SetProtect();

    
    long nLen = Len();
    if( nLen > MAX_FIELDLEN )
        nLen = MAX_FIELDLEN;

    
    
    OUString aVal( rMan.GetText().GetText( Where(), nLen ) );

    
    SwFltBookmark aBook( rName, aVal, GetHandle() );
    rOut << aBook;
}

void Ww1Bookmarks::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where())
    {
        Out(rOut, rMan);
        ++(*this);
    }
}

void Ww1Footnotes::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where())
    {
        OSL_ENSURE(nPlcIndex < Count(), "WwFootnotes");
        sal_Unicode c;
        rMan.Fill(c);
        OSL_ENSURE(c==0x02, "Ww1Footnotes");
        if (c==0x02)
        {
            Ww1FtnText* pText = new Ww1FtnText(rMan.GetFib());
        
            sal_uLong start = aText.Where(nPlcIndex);
            pText->Seek(start);
        
            sal_uLong count = aText.Where(nPlcIndex+1) - start;
            pText->SetCount(count);
        
            pText->Out(c);
            OSL_ENSURE(c==0x02, "Ww1Footnotes");
            count--; 
        
            rOut.BeginFootnote();
            bStarted = sal_True;
            rMan.Push0(pText, pText->Offset(rMan.GetFib()),
             new Ww1FootnoteFields(rMan.GetFib()));
            rOut << rMan;
            rMan.Pop();
            rOut.EndFootnote();
        }
        else
            ++(*this);
    }
}

void Ww1Footnotes::Stop(Ww1Shell& /*rOut*/, Ww1Manager& rMan, sal_Unicode& c)
{
    if (bStarted && rMan.Where() > Where())
    {
        OSL_ENSURE(nPlcIndex < Count(), "Ww1Footnotes");
        c = ' ';
        ++(*this);
    }
}

void Ww1Fields::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where()){
        OSL_ENSURE(nPlcIndex < Count(), "Ww1Fields");
        if (GetData()->chGet() == 19)
            Out(rOut, rMan);
        else
            ++(*this); 
    }
}

void Ww1Fields::Stop( Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode& c)
{
    if (rMan.Where() >= Where())
    {
        OSL_ENSURE(nPlcIndex < Count(), "Ww1Fields");
        if (GetData()->chGet() != 19)
        {
            rMan.Fill( c );
            OSL_ENSURE(c==21, "Ww1Fields");
            ++(*this);
            c = ' ';
            if (pField)
            
            {
                rOut << *pField;
                delete pField;
                pField = 0;
            
            
            
            }
            if (!sErgebnis.isEmpty())
                rOut << sErgebnis;
        }
    }
}

enum WWDateTime{ WW_DONTKNOW = 0x0, WW_DATE = 0x1, WW_TIME = 0x2, WW_BOTH = 0x3 };

static WWDateTime GetTimeDatePara( const OUString& rForm,
                                    SwTimeFormat* pTime = 0,
                                     SwDateFormat* pDate = 0 )
{
    WWDateTime eDT = WW_BOTH;
    if (rForm.indexOf('H') != -1)         
    {
        if( pTime )
            *pTime = TF_SSMM_24;
    }
    else if (rForm.indexOf('H') != -1)    
    {
        if( pTime )
            *pTime = TF_SSMM_12;
    }
    else                                    
    {
        eDT = (WWDateTime)( eDT & ~(sal_uInt16)WW_TIME );
    }

    sal_Int32 nDPos = 0;
    while (true)
    {
        nDPos = rForm.indexOf('M', nDPos);     
        if (nDPos == 0 || nDPos == -1)
            break;
        sal_Unicode cPrev = rForm[nDPos - 1];
        
        if( 'a' != cPrev && 'A' != cPrev && 'p' != cPrev && 'P' != cPrev )
            break;
        
        ++nDPos;
    }

    if (nDPos != -1)                  
    {
        static SwDateFormat const aDateA[32] =
        {
            DFF_DMY, DFF_DMMY, DFF_DMYY, DFF_DMMYY,
            DFF_DMMMY, DFF_DMMMY, DFF_DMMMYY, DFF_DMMMYY,
            DFF_DDMMY, DFF_DDMMY, DFF_DDMMMYY, DFF_DDMMMYY,
            DFF_DDMMMY, DFF_DDMMMY, DFF_DDMMMYY, DFF_DDMMMYY,
            DFF_DDDMMMY, DFF_DDDMMMY, DFF_DDDMMMYY, DFF_DDDMMMYY,
            DFF_DDDMMMY, DFF_DDDMMMY, DFF_DDDMMMYY, DFF_DDDMMMYY,
            DFF_DDDMMMY, DFF_DDDMMMY, DFF_DDDMMMYY, DFF_DDDMMMYY,
            DFF_DDDMMMY, DFF_DDDMMMY, DFF_DDDMMMYY, DFF_DDDMMMYY
        };

        bool bHasDay = rForm.indexOf('t') != -1 ||
                       rForm.indexOf('T') != -1 ||
                       rForm.indexOf('d') != -1 ||
                       rForm.indexOf('D') != -1;

        sal_Bool bLongDayOfWeek = rForm.indexOf("tttt") != -1 ||
                                  rForm.indexOf("TTTT") != -1 ||
                                  rForm.indexOf("dddd") != -1 ||
                                  rForm.indexOf("DDDD") != -1;

        sal_Bool bDayOfWeek = rForm.indexOf("ttt") != -1 ||
                              rForm.indexOf("TTT") != -1 ||
                              rForm.indexOf("ddd") != -1 ||
                              rForm.indexOf("DDD") != -1;

                    
                    
        sal_Bool bLitMonth = rForm.indexOf("MMM") != -1;
                    
        sal_Bool bFullMonth = rForm.indexOf("MMMM") != -1;
                    
                    
        sal_Bool bFullYear = rForm.indexOf("jjj") != -1 ||
                             rForm.indexOf("JJJ") != -1 ||
                             rForm.indexOf("yyy") != -1 ||
                             rForm.indexOf("YYY") != -1;

        sal_uInt16 i = ( bLitMonth & 1 )
                   | ( ( bFullYear & 1 ) << 1 )
                   | ( ( bFullMonth & 1 ) << 2 )
                   | ( ( bDayOfWeek & 1 ) << 3 )
                   | ( ( bLongDayOfWeek & 1 ) << 4 );
        if( pDate )
        {
            if( !bHasDay && !bFullMonth )
                *pDate = DFF_MY;
            else
                *pDate = aDateA[i];
        }
    }
    else
    {
        eDT = (WWDateTime)( eDT & ~(sal_uInt16)WW_DATE );
    }
    return eDT;
}


extern void sw3io_ConvertFromOldField( SwDoc& rDoc, sal_uInt16& rWhich,
                                sal_uInt16& rSubType, sal_uLong &rFmt,
                                sal_uInt16 nVersion );

void Ww1Fields::Out(Ww1Shell& rOut, Ww1Manager& rMan, sal_uInt16 nDepth)
{
    OUString sType; 
    OUString sFormel; 
    OUString sFormat;
    OUString sDTFormat;   
    W1_FLD* pData = GetData(); 
    OSL_ENSURE(pData->chGet()==19, "Ww1Fields"); 

    sal_Unicode c;
    rMan.Fill( c );
    OSL_ENSURE(c==19, "Ww1Fields"); 
    if (pData->chGet()==19 && c == 19)
    {
        OUString aStr;
        c = rMan.Fill( aStr, GetLength() );
        OSL_ENSURE(Ww1PlainText::IsChar(c), "Ww1Fields");
        sal_Int32 pos = aStr.indexOf(' ');
        
        if ( pos != -1 )
        {
            sType = aStr.copy(0, pos);
            aStr = aStr.copy(pos+1);
        }
        else
        {
            sType = aStr;
            aStr = "";
        }
        sFormel += aStr;
        sal_uInt8 rbType = pData->fltGet();
        do {
        
        
        
        
            ++(*this);
            pData = GetData();
            if (pData->chGet()==19) 
            {
                Out(rOut, rMan, nDepth+1);
                rMan.Fill(c);
                OSL_ENSURE(c==21, "Ww1PlainText");
                sFormel += "Ww" + OUString::number(nPlcIndex);
                c = rMan.Fill(aStr, GetLength());
                OSL_ENSURE(Ww1PlainText::IsChar(c), "Ww1PlainText");
                sFormel += aStr;
            }
        }
        while (pData->chGet()==19);

        
        pos = sFormel.indexOf("\\*");
        if (pos != -1)
        {
            sFormat = sFormel.copy(pos);
            sFormel = sFormel.copy(0, pos);
        }

        pos = sFormel.indexOf( "\\@" );
        if (pos != -1)
        {
            sDTFormat = sFormel.copy(pos);
            sFormel = sFormel.copy(0, pos);
        }

        
        if( pData->chGet() == 20 )
        {
            rMan.Fill( c );
            OSL_ENSURE(c==20, "Ww1PlainText");
            c = rMan.Fill(sErgebnis, GetLength());
            if (!Ww1PlainText::IsChar(c))
                sErgebnis += OUString(c); 
            ++(*this);
            pData = GetData();
        }
        OSL_ENSURE(pData->chGet()==21, "Ww1PlainText");
        bool bKnown = true;
        OSL_ENSURE(pField==0, "Ww1PlainText");
        if (pField != 0)
        {
            rOut << *pField;
            delete pField;
            pField = 0;
        }

oncemore:
        switch (rbType)
        {
        case 3: 
            sFormel = rOut.ConvertUStr(sFormel);
            pField = new SwGetRefField( (SwGetRefFieldType*)
                rOut.GetSysFldType( RES_GETREFFLD ),
                sFormel,
                REF_BOOKMARK,
                0,
                REF_CONTENT );

        break;
        case 6: 
        {
            pos = aStr.indexOf(' ');
            OUString aName;
            if (pos != -1)
            {
                aName = aStr.copy(0, pos);
                aStr = aStr.copy(pos+1);
            }
            else
            {
                aName = aStr;
                aStr = "";
            }
            if (aName.isEmpty())
                break;
            aName = rOut.ConvertUStr(aName);
            SwFieldType* pFT = rOut.GetDoc().InsertFldType(
                SwSetExpFieldType( &rOut.GetDoc(), aName, nsSwGetSetExpType::GSE_STRING ) );
            pField = new SwSetExpField((SwSetExpFieldType*)pFT, aStr);
            ((SwSetExpField*)pField)->SetSubType( nsSwExtendedSubType::SUB_INVISIBLE );
            

            
        }
        break;
        case 14: 
        {
            OUString aSubType;
            pos = aStr.indexOf(' ');
            if (pos != -1)
            {
                aSubType = aStr.copy(0, pos);
                aStr = aStr.copy(pos+1);
            }
            else
            {
                aSubType = aStr;
                aStr = "";
            }

            aSubType = rOut.ConvertUStr(aSubType);


            
            
            
            
#define IS(sd, se, t) \
    if (aSubType == sd || aSubType == se) \
        rbType = t; \
    else

            
            IS("titel",          "title",          15)
            IS("thema",          "subject",        16)
            IS("autor",          "author",         17)
            IS("stichw?rter",    "keywords",       18) 
            IS("kommentar",      "comment",        19)
            IS("gespeichertvon", "lastrevisedby",  20)
            IS("ertelldat",      "creationdate",   21)
            IS("speicherdat",    "revisiondate",   22)
            IS("druckdat",       "printdate",      23)
            IS("version",        "revisionnumber", 24)
            IS("zeit",           "edittime",       25)
            IS("anzseit",        "numberofpages",  26)
            IS("anzw?rter",      "numberofwords",  27) 
            IS("anzzeichen",     "numberofchars",  28)
            IS("dateiname",      "filename",       29)
            IS("vorlage",        "templatename",   30)
                bKnown = false;
#undef IS
            if (rbType != 14)
                goto oncemore;
        }
        break;
        case 15: 
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_TITEL, OUString(), 0);
        break;
        case 16: 
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_THEMA, OUString(), 0);
        break;
        case 17: 
            pField = new SwAuthorField((SwAuthorFieldType*)
             rOut.GetSysFldType(RES_AUTHORFLD), AF_NAME );
        break;
        case 18: 
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_KEYS, OUString(), 0);
        break;
        case 19: 
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_COMMENT, OUString(), 0);
        break;
        case 20: 
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_CHANGE|DI_SUB_AUTHOR, OUString());
        break;
        case 21: 
        case 22: 
        case 23: 
        case 25:{
                    sal_uInt16 nSub;
                    sal_uInt16 nReg = 0;    

                    switch( rbType )
                    {
                        default:
                        case 21: nSub = DI_CREATE;  nReg = DI_SUB_DATE; break;
                        case 23: nSub = DI_PRINT;   nReg = DI_SUB_DATE; break;
                        case 22: nSub = DI_CHANGE;  nReg = DI_SUB_DATE; break;
                        case 25: nSub = DI_CHANGE;  nReg = DI_SUB_TIME; break;
                    }
                    switch( GetTimeDatePara( sDTFormat ) )
                    {
                        case WW_DATE: nReg = DI_SUB_DATE; break;
                        case WW_TIME: nReg = DI_SUB_TIME; break;
                        case WW_BOTH: nReg = DI_SUB_DATE; break;
                        default:
                            break;
                        
                    }
                    pField = new SwDocInfoField((SwDocInfoFieldType*)
                        rOut.GetSysFldType(RES_DOCINFOFLD), nSub | nReg, OUString());
                }
        break;
        case 24: 
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD),  DI_DOCNO, OUString(), 0);
        break;
        case 26: 
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_PAGE, SVX_NUM_ARABIC);
        break;
        case 27: 
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_WORD, SVX_NUM_ARABIC);
        break;
        case 28: 
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_CHAR, SVX_NUM_ARABIC);
        break;
        case 29: 
            pField = new SwFileNameField((SwFileNameFieldType*)
             rOut.GetSysFldType(RES_FILENAMEFLD));
        break;
        case 30: 
            pField = new SwTemplNameField((SwTemplNameFieldType*)
             rOut.GetSysFldType(RES_TEMPLNAMEFLD), FF_NAME);
        break;
        case 31:
        case 32:{
                    SwDateFormat aDate = DF_SSYS;
                    SwTimeFormat aTime = TF_SYSTEM;

                    WWDateTime eDT = GetTimeDatePara(sDTFormat, &aTime, &aDate);
                    if( eDT == WW_DONTKNOW )        
                        eDT = ( rbType == 32 ) ? WW_TIME : WW_DATE;    

                    if( eDT & WW_DATE )
                    {
                        sal_uInt16 nWhich = RES_DATEFLD;
                        sal_uInt16 nSubType = DATEFLD;
                        sal_uLong nFormat = aDate;
                        sw3io_ConvertFromOldField( rOut.GetDoc(),
                            nWhich, nSubType, nFormat, 0x0110 );
                        pField = new SwDateTimeField((SwDateTimeFieldType*)
                            rOut.GetSysFldType(RES_DATETIMEFLD), DATEFLD, nFormat);

                        if( eDT == WW_BOTH )
                            rOut << * pField << ' ';
                                
                    }
                    if( eDT & WW_TIME )
                    {
                        sal_uInt16 nWhich = RES_TIMEFLD;
                        sal_uInt16 nSubType = TIMEFLD;
                        sal_uLong nFormat = aTime;
                        sw3io_ConvertFromOldField( rOut.GetDoc(),
                            nWhich, nSubType, nFormat, 0x0110 );
                        pField = new SwDateTimeField((SwDateTimeFieldType*)
                            rOut.GetSysFldType(RES_DATETIMEFLD), TIMEFLD, nFormat);
                    }

                }
        break;
        case 33: 
            pField = new SwPageNumberField((SwPageNumberFieldType*)
             rOut.GetSysFldType(RES_PAGENUMBERFLD), PG_RANDOM, SVX_NUM_ARABIC);
        break;
        case 34: 
        {
            if (nDepth == 0)
            {
                SwGetExpFieldType* pFieldType =
                 (SwGetExpFieldType*)rOut.GetSysFldType(RES_GETEXPFLD);
                OSL_ENSURE(pFieldType!=0, "Ww1Fields");
                if (pFieldType != 0)
                    pField = new SwGetExpField(pFieldType, sFormel,
                     nsSwGetSetExpType::GSE_STRING, VVF_SYS);
            }
            else 
            {
                OUString aName("Ww");
                aName += OUString::number( nPlcIndex );
                SwFieldType* pFT = rOut.GetDoc().GetFldType( RES_SETEXPFLD, aName, false);
                if (pFT == 0)
                {
                    SwSetExpFieldType aS(&rOut.GetDoc(), aName, nsSwGetSetExpType::GSE_FORMULA);
                    pFT = rOut.GetDoc().InsertFldType(aS);
                }
                SwSetExpField aFld((SwSetExpFieldType*)pFT, sFormel);
                aFld.SetSubType(nsSwExtendedSubType::SUB_INVISIBLE);
                rOut << aFld;
            }
        }
        break;
        case 36: 
        {
            OUString aFName;
            pos = aStr.indexOf(' ');
            if (pos != -1)
            {
                aFName = aStr.copy(0, pos);
                aStr = aStr.copy(pos+1);
            }
            else
            {
                aFName = aStr;
                aStr = "";
            }

            if(aFName.isEmpty())
                break;
            aFName = aFName.replaceFirst("\\\\", "\\");

            aFName = URIHelper::SmartRel2Abs(
                INetURLObject(rOut.GetBaseURL()), aFName );

            const OUString sStr(aStr);
            SwSectionData * pSection = new SwSectionData( FILE_LINK_SECTION,
                rOut.GetDoc().GetUniqueSectionName( &sStr ) );
            pSection->SetLinkFileName( aFName );
            pSection->SetProtectFlag( true );
            rOut << SwFltSection( pSection );
            rOut.EndItem( RES_FLTR_SECTION );
            rOut.NextParagraph();
        }
        case 37: 
            pField = new SwGetRefField(
             (SwGetRefFieldType*)rOut.GetSysFldType(RES_GETREFFLD),
             sFormel, 0, 0, REF_PAGE);
        break;
        case 38: 
        {
            OUString aName;
            pos = aStr.indexOf(' ');
            if (pos != -1)
            {
                aName = aStr.copy(0, pos);
                aStr = aStr.copy(pos+1);
            }
            else
            {
                aName = aStr;
                aStr = "";
            }

            if (aName.isEmpty())
                break;

            SwFieldType* pFT = rOut.GetDoc().InsertFldType(
                SwSetExpFieldType( &rOut.GetDoc(), aName, nsSwGetSetExpType::GSE_STRING ) );
            pField = new SwSetExpField((SwSetExpFieldType*)pFT, aStr );
            ((SwSetExpField*)pField)->SetInputFlag( sal_True );
            ((SwSetExpField*)pField)->SetSubType(nsSwExtendedSubType::SUB_INVISIBLE);
        }
        case 39: 
            pField = new SwInputField(
                static_cast<SwInputFieldType*>(rOut.GetSysFldType( RES_INPUTFLD )),
                OUString(), sFormel,
                INP_TXT, 0, false );
        break;
        case 51: 
        {
            OUString aName;
            pos = aStr.indexOf(' ');
            if (pos != -1)
            {
                aName = aStr.copy(0, pos);
                aStr = aStr.copy(pos+1);
            }
            else
            {
                aName = aStr;
                aStr = "";
            }

            if (aName.isEmpty() || aStr.isEmpty())
                break;

            pField = new SwMacroField( (SwMacroFieldType*)
                            rOut.GetSysFldType( RES_MACROFLD ),
                            "StarOffice.Standard.Modul1." + aName, aStr );
        }
        break;
        case 55: 
        {
            const sal_Unicode* pFormel = sFormel.getStr();
            const sal_Unicode* pDot = 0;
            OUString sName;
            while (*pFormel != '\0' && *pFormel != ' ')
            {
                
                if (*pFormel == '.')
                    pDot = pFormel;
                else
                    
                    if (*pFormel == '\\')
                    {
                        pDot = 0;
                        if (pFormel[1] == '\\')
                            pFormel++;
                    }
                if (*pFormel != '\0')
                    sName += OUString(*pFormel++);
            }
            if( pDot )
            {
                OUStringBuffer sBuf;
                while( *pDot != '\0' && *pDot != ' ')
                    sBuf.append(*pDot++);
                OUString sExt = sBuf.makeStringAndClear();

                if( sExt.equalsIgnoreAsciiCase( ".tiff" )
                 || sExt.equalsIgnoreAsciiCase( ".bmp" )
                 || sExt.equalsIgnoreAsciiCase( ".gif" )
                 || sExt.equalsIgnoreAsciiCase( ".pcx" )
                 || sExt.equalsIgnoreAsciiCase( ".pic" ))
                    rOut.AddGraphic( sName );
                else
                    bKnown = false;
            }
            else
                bKnown = false;
        }
        break;
        default: 
            OSL_ENSURE(false, "Ww1PlainText");
        
        case 1: 
        case 2: 
        case 4: 
        
        case 5: 
        case 7: 
        case 8: 
        
        case 9: 
        
        case 10: 
        case 11: 
        case 12: 
        case 13: 
        
        case 35: 
        
        case 40: 
        case 41: 
        case 42: 
        case 43: 
        case 44: 
        //
        case 45: 
        case 46: 
        case 47: 
        case 48: 
        case 49: 
        case 50: 
        case 52: 
        case 53: 
        case 54: 
            bKnown = false;
        break;
        }
        if( bKnown || sErgebnis == "\270" )
            this->sErgebnis = "";
        else
            this->sErgebnis = sErgebnis;
    }
    else 
        ++(*this);
}

sal_uLong Ww1Fields::GetLength()
{
    
    
    
    sal_uLong ulBeg = Where();
    sal_uLong ulEnd = Where(nPlcIndex+1);
    OSL_ENSURE(ulBeg<ulEnd, "Ww1Fields");
    return (ulEnd - ulBeg) - 1;
}

void Ww1Sep::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where())
    {
        rOut.NextSection();
        SwFrmFmt &rFmt = rOut.GetPageDesc().GetMaster();
        W1_DOP& rDOP = rMan.GetDop().GetDOP();
        rOut.GetPageDesc().SetLandscape(rDOP.fWideGet());
        SwFmtFrmSize aSz(rFmt.GetFrmSize());
        aSz.SetWidth(rDOP.xaPageGet());
        aSz.SetHeight(rDOP.yaPageGet());
        rFmt.SetFmtAttr(aSz);
        SvxLRSpaceItem aLR(rDOP.dxaLeftGet()+rDOP.dxaGutterGet(),
         rDOP.dxaRightGet(), 0, 0, RES_LR_SPACE);
        rFmt.SetFmtAttr(aLR);
        SvxULSpaceItem aUL(rDOP.dyaTopGet(), rDOP.dyaBottomGet(), RES_UL_SPACE);
        rFmt.SetFmtAttr(aUL);
        
        
        
        
        sal_uInt8* pByte = GetData();
        Ww1SprmSep aSprm(rFib, SVBT32ToUInt32(pByte + 2));
        aSprm.Start(rOut, rMan);
        aSprm.Stop(rOut, rMan);
        ++(*this);
        aHdd.Start(rOut, rMan);
    }
}

void Ww1Pap::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where())
    {
        sal_uInt8* pByte;
        sal_uInt16 cb;
        
        if (FillStart(pByte, cb))
        {
            Ww1SprmPapx aSprm(pByte, cb);
            
            aSprm.Start(rOut, rMan);
        }
        ++(*this);
    }
}

void Ww1Pap::Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode&)
{
    if (rMan.Where() >= Where() || rMan.IsStopAll())
    {
        sal_uInt8* pByte;
        sal_uInt16 cb;
        if (FillStop(pByte, cb)){
            Ww1SprmPapx aSprm(pByte, cb);
            aSprm.Stop(rOut, rMan);
        }else{
            OSL_ENSURE( !nPlcIndex || rMan.IsStopAll(), "Pap-Attribut-Stop verloren" );
        }
    }
}



void W1_CHP::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (fBoldGet())
        rOut << SvxWeightItem(
            rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    if (fItalicGet())
        rOut << SvxPostureItem(
            rOut.GetPostureItalic()?ITALIC_NONE:ITALIC_NORMAL, RES_CHRATR_POSTURE);
    if (fStrikeGet())
        rOut << SvxCrossedOutItem(
            rOut.GetCrossedOut()?STRIKEOUT_NONE:STRIKEOUT_SINGLE, RES_CHRATR_CROSSEDOUT);
    if (fOutlineGet())
        rOut << SvxContourItem(!rOut.GetContour(), RES_CHRATR_CONTOUR);
    if (fSmallCapsGet())
        rOut << SvxCaseMapItem(
            rOut.GetCaseKapitaelchen()?SVX_CASEMAP_NOT_MAPPED:SVX_CASEMAP_KAPITAELCHEN, RES_CHRATR_CASEMAP);
    if (fCapsGet())
        rOut << SvxCaseMapItem(
            rOut.GetCaseVersalien()?SVX_CASEMAP_NOT_MAPPED:SVX_CASEMAP_VERSALIEN, RES_CHRATR_CASEMAP);
    if (fsHpsGet())
            rOut << SvxFontHeightItem(hpsGet() * 10, 100, RES_CHRATR_FONTSIZE);
    if (fsKulGet())
        switch (kulGet()) {
        case 0: {
                    rOut << SvxUnderlineItem(UNDERLINE_NONE, RES_CHRATR_UNDERLINE) <<
                        SvxWordLineModeItem(sal_False, RES_CHRATR_WORDLINEMODE);
                } break;
        default: OSL_ENSURE(false, "Chpx");
        case 1: {
                    rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE);
                } break;
        case 2: {
                    rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE) <<
                    SvxWordLineModeItem(sal_True, RES_CHRATR_WORDLINEMODE);
                } break;
        case 3: {
                    rOut << SvxUnderlineItem(UNDERLINE_DOUBLE, RES_CHRATR_UNDERLINE);
                } break;
        case 4: {
                    rOut << SvxUnderlineItem(UNDERLINE_DOTTED, RES_CHRATR_UNDERLINE);
                } break;
        }

    if (fsIcoGet())
        switch(icoGet()) {
        default: OSL_ENSURE(false, "Chpx");
        case 0: { rOut.EndItem(RES_CHRATR_COLOR); } break;
        case 1: { rOut << SvxColorItem(Color(COL_BLACK), RES_CHRATR_COLOR); } break;
        case 2: { rOut << SvxColorItem(Color(COL_LIGHTBLUE), RES_CHRATR_COLOR); } break;
        case 3: { rOut << SvxColorItem(Color(COL_LIGHTCYAN), RES_CHRATR_COLOR); } break;
        case 4: { rOut << SvxColorItem(Color(COL_LIGHTGREEN), RES_CHRATR_COLOR); } break;
        case 5: { rOut << SvxColorItem(Color(COL_LIGHTMAGENTA), RES_CHRATR_COLOR); } break;
        case 6: { rOut << SvxColorItem(Color(COL_LIGHTRED), RES_CHRATR_COLOR); } break;
        case 7: { rOut << SvxColorItem(Color(COL_YELLOW), RES_CHRATR_COLOR); } break;
        case 8: { rOut << SvxColorItem(Color(COL_WHITE), RES_CHRATR_COLOR); } break;
        }
    if (fsSpaceGet()) {
        short sQps = qpsSpaceGet();
        if (sQps > 56)
            sQps = sQps - 64;
        rOut << SvxKerningItem(sQps, RES_CHRATR_KERNING);
    }
    if (fsPosGet()) {
        if (hpsPosGet() == 0)
            rOut << SvxEscapementItem(SVX_ESCAPEMENT_OFF, 100, RES_CHRATR_ESCAPEMENT);
        else {
            short sHps = hpsPosGet();
            if (sHps > 128)
                sHps =  sHps - 256;
            sHps *= 100;
            sHps /= 24;
            rOut << SvxEscapementItem(sHps, 100, RES_CHRATR_ESCAPEMENT);
        }
    }
    if (fsFtcGet()) {
        SvxFontItem aFont(rMan.GetFont(ftcGet()));
        rOut << aFont;
    }
}

void Ww1Chp::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where())
    {
        W1_CHP aChpx;
        if (FillStart(aChpx))
        {
            aChpx.Out(rOut, rMan);
            if (aChpx.fcPicGet())
            {
                Ww1Picture aPic(rMan.GetFib().GetStream(),
                 aChpx.fcPicGet());
                if (!aPic.GetError())
                    aPic.Out(rOut, rMan);
            }
        }
        ++(*this);
    }
}

void Ww1Chp::Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode&)
{
    if (rMan.Where() >= Where())
    {
        W1_CHP aChpx;
        if (FillStop(aChpx))
        {
            
            if (aChpx.fBoldGet())
                rOut.EndItem(RES_CHRATR_WEIGHT);
            if (aChpx.fItalicGet())
                rOut.EndItem(RES_CHRATR_POSTURE);
            if (aChpx.fStrikeGet())
                rOut.EndItem(RES_CHRATR_CROSSEDOUT);
            if (aChpx.fOutlineGet())
                rOut.EndItem(RES_CHRATR_CONTOUR);
            if (aChpx.fSmallCapsGet() || aChpx.fCapsGet())
                rOut.EndItem(RES_CHRATR_CASEMAP);
            
            
            if (aChpx.fsHpsGet())
                rOut.EndItem(RES_CHRATR_FONTSIZE);
            if (aChpx.fsKulGet())
                rOut.EndItem(RES_CHRATR_UNDERLINE)
                    .EndItem(RES_CHRATR_WORDLINEMODE);
            if (aChpx.fsIcoGet())
                rOut.EndItem(RES_CHRATR_COLOR);
            if (aChpx.fsSpaceGet())
                rOut.EndItem(RES_CHRATR_KERNING);
            if (aChpx.fsPosGet())
                rOut.EndItem(RES_CHRATR_ESCAPEMENT);
            if (aChpx.fsFtcGet())
                rOut.EndItem(RES_CHRATR_FONT);
        }else{
            OSL_ENSURE( !nPlcIndex, "Chp-Attribut-Stop verloren" );
        }
    }
}

void Ww1Style::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
    
    
    if(pParent->GetStyle(stcBase).IsUsed() )    
        rOut.BaseStyle(stcBase);

    
    aChpx.Out(rOut, rMan);
    
    if (pPapx)
        pPapx->Start(rOut, rMan);
}














sal_Unicode Ww1PlainText::Out( Ww1Shell& rOut, sal_uLong& ulEnd )
{
    
    
    if (ulEnd > Count())
        ulEnd = Count();
    while (ulSeek < ulEnd)
    {
        sal_Unicode c = (*this)[ulSeek];
        ++(*this);
        if (Ww1PlainText::IsChar(c))
            rOut << c;
        else
            return c;
    }
    return Ww1PlainText::MinChar;
}

sal_Unicode Ww1PlainText::Out(OUString& rStr, sal_uLong ulEnd)
{
    
    rStr = "";
    if (ulEnd > Count())
        ulEnd = Count();
    while (ulSeek < ulEnd)
    {
        sal_Unicode c = (*this)[ulSeek];
        ++(*this);
        if( Ww1PlainText::IsChar(c) )
            rStr += OUString(c);
        else
            return c;
    }
    return Ww1PlainText::MinChar;
}


sal_Unicode Ww1PlainText::Out( sal_Unicode& rRead )
{
    rRead = (*this)[ulSeek];
    ++(*this);
    return rRead;
}

void Ww1SprmPapx::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if( !rMan.IsInStyle() ){        
                                    
        if (!rOut.IsInFly()
            && !rOut.IsInTable()    
            && ( rMan.HasPPc() || rMan.HasPDxaAbs())){ 
            rOut.BeginFly();        
        }
        if (!rOut.IsInTable() && rMan.HasInTable())
        {
            rOut.BeginTable();
        }
        rOut.SetStyle(aPapx.stcGet());
    }
    Ww1Sprm::Start(rOut, rMan);
}

void Ww1SprmPapx::Stop(Ww1Shell& rOut, Ww1Manager& rMan)
{
    Ww1Sprm::Stop(rOut, rMan);

    if( !rMan.IsInStyle() )         
    {                               
        if (rOut.IsInTable() &&( rMan.IsStopAll() || !rMan.HasInTable()))
            rOut.EndTable();

        if( rOut.IsInFly() &&
            ( rMan.IsStopAll()
                || ( !rMan.HasPPc() && !rMan.HasPDxaAbs()   
                    && !rOut.IsInTable())))     
            rOut.EndFly();
    }
}

SvxFontItem Ww1Fonts::GetFont(sal_uInt16 nFCode)
{
    
    FontFamily eFamily = FAMILY_DONTKNOW;
    OUString aName;
    FontPitch ePitch = PITCH_DONTKNOW;
    rtl_TextEncoding eCharSet = RTL_TEXTENCODING_DONTKNOW;
    switch (nFCode)
    {
    
    
    
    case 0:
         eFamily = FAMILY_ROMAN;
         aName = "Tms Rmn";
         ePitch = PITCH_VARIABLE;
         eCharSet = RTL_TEXTENCODING_MS_1252;
    break;
    case 1:
         aName = "Symbol";
         ePitch = PITCH_VARIABLE;
         eCharSet = RTL_TEXTENCODING_SYMBOL;
    break;
    case 2:
         eFamily = FAMILY_SWISS;
         aName = "Helv";
         ePitch = PITCH_VARIABLE;
         eCharSet = RTL_TEXTENCODING_MS_1252;
    break;
    default:
    {
        W1_FFN* pF = GetFFN(nFCode - 3);
        if (pF != 0)
        {
            
            aName = OUString( (sal_Char*)pF->szFfnGet(), strlen( (sal_Char*)pF->szFfnGet() ),
                            RTL_TEXTENCODING_MS_1252 );
            
            static const FontPitch ePitchA[] =
            {
                PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, PITCH_DONTKNOW
            };
            ePitch = ePitchA[pF->prgGet()];
            
            eCharSet = RTL_TEXTENCODING_MS_1252;
            if (aName.equalsIgnoreAsciiCase("Symbol")
             || aName.equalsIgnoreAsciiCase("Symbol Set")
             || aName.equalsIgnoreAsciiCase("Wingdings")
             || aName.equalsIgnoreAsciiCase("ITC Zapf Dingbats") )
                eCharSet = RTL_TEXTENCODING_SYMBOL;
            
            sal_uInt16 b = pF->ffGet();
            static const FontFamily eFamilyA[] =
            {
                FAMILY_DONTKNOW, FAMILY_ROMAN, FAMILY_SWISS, FAMILY_MODERN,
                FAMILY_SCRIPT, FAMILY_DECORATIVE
            };
            if (b < (sizeof(eFamilyA)/sizeof(eFamilyA[0])))
                eFamily = eFamilyA[b];
        }
        else
        {
            OSL_ENSURE(false, "WW1Fonts::GetFont: Nicht existenter Font !");
            eFamily = FAMILY_SWISS;
             aName = "Helv";
            ePitch = PITCH_VARIABLE;
            eCharSet = RTL_TEXTENCODING_MS_1252;
        }
    }
    break;
    }
    
    if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO )
         && ( aName.equalsIgnoreAsciiCase("Helv")
            || aName.equalsIgnoreAsciiCase("Helvetica") ) )
    {
         aName = "Helvetica Neue";
        if (eFamily==FAMILY_DONTKNOW)
            eFamily = FAMILY_SWISS;
    }
    else
    {
        
        
        
        
        
        
        if( aName.equalsIgnoreAsciiCase("Helv"))
        {
            aName  = "Helvetica";
            if (eFamily==FAMILY_DONTKNOW)
                eFamily = FAMILY_SWISS;
        }
        else if (aName.equalsIgnoreAsciiCase("Tms Rmn"))
        {
             aName = "Times New Roman";
            if (eFamily==FAMILY_DONTKNOW)
                eFamily = FAMILY_ROMAN;
        }
        else if (aName.equalsIgnoreAsciiCase("System Monospaced") )
        {
             aName = "Courier";
            ePitch = PITCH_FIXED;
        }
    }
    
    return SvxFontItem(eFamily, aName, OUString(), ePitch, eCharSet, RES_CHRATR_FONT);
}

void Ww1Dop::Out(Ww1Shell& rOut)
{
    
    long nDefTabSiz = aDop.dxaTabGet();
    if (nDefTabSiz < 56)
        nDefTabSiz = 709;

    
    SvxTabStopItem aNewTab(1, sal_uInt16(nDefTabSiz), SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP);
    ((SvxTabStop&)aNewTab[0]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;
    rOut.GetDoc().GetAttrPool().SetPoolDefaultItem( aNewTab); 

    SwFrmFmt &rFmt = rOut.GetPageDesc().GetMaster();
    W1_DOP& rDOP = GetDOP();
    rOut.GetPageDesc().SetLandscape(rDOP.fWideGet());
    SwFmtFrmSize aSz(rFmt.GetFrmSize());
    aSz.SetWidth(rDOP.xaPageGet());
    aSz.SetHeight(rDOP.yaPageGet());
    rFmt.SetFmtAttr(aSz);
    SvxLRSpaceItem aLR(rDOP.dxaLeftGet()+rDOP.dxaGutterGet(),
     rDOP.dxaRightGet(), 0, 0, RES_LR_SPACE);
    rFmt.SetFmtAttr(aLR);
    SvxULSpaceItem aUL(rDOP.dyaTopGet(), rDOP.dyaBottomGet(), RES_UL_SPACE);
    rFmt.SetFmtAttr(aUL);

    SwFtnInfo aInfo;
    aInfo = rOut.GetDoc().GetFtnInfo();     
                
                
    switch( rDOP.fpcGet() ){
    case 1:
    case 2: aInfo.ePos = FTNPOS_PAGE; break;
    default: aInfo.ePos = FTNPOS_CHAPTER; break;
    }
    
    
    aInfo.eNum = FTNNUM_DOC;
                            
                            
    if( rDOP.nFtnGet() > 0 && aInfo.eNum == FTNNUM_DOC )
        aInfo.nFtnOffset = rDOP.nFtnGet() - 1;
    rOut.GetDoc().SetFtnInfo( aInfo );

}

void Ww1Assoc::Out(Ww1Shell& rOut)
{
    
    
    SwDocShell *pDocShell(rOut.GetDoc().GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");
        if (xDocProps.is()) {
            xDocProps->setTitle( GetStr(Title) );
            xDocProps->setSubject( GetStr(Subject) );
            xDocProps->setDescription( GetStr(Comments) );
            xDocProps->setKeywords(
              ::comphelper::string::convertCommaSeparated( GetStr(KeyWords) ) );
            xDocProps->setAuthor( GetStr(Author) );
            xDocProps->setModifiedBy( GetStr(LastRevBy) );
        }
    }
}

void Ww1StyleSheet::OutDefaults(Ww1Shell& rOut, Ww1Manager& rMan, sal_uInt16 stc)
{
    switch (stc){
    case 222: 
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 223: 
        rOut << SvxFontHeightItem(160, 100, RES_CHRATR_FONTSIZE);
        break;
    case 224: 
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 225: 
    case 226: 
    case 227: 
    case 228: 
    case 229: 
    case 230: 
    case 231: 
    case 232: 
        rOut << SvxLRSpaceItem(( 232 - stc ) * 720, 720, 0, 0, RES_LR_SPACE);
            
        break;
    case 233: 
    case 234: 
    case 235: 
    case 236: 
    case 237: 
    case 238: 
        rOut << SvxLRSpaceItem(( 239 - stc ) * 360, 0, 0, 0, RES_LR_SPACE);
        break;
    case 239: 
        break;
    case 240: 
        break;
    case 241: 
        break;
    case 242:  
    case 243:{ 
            SvxTabStopItem aAttr(RES_PARATR_TABSTOP);
            SvxTabStop aTabStop;
            aTabStop.GetTabPos() = 4535;  
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER;
            aAttr.Insert( aTabStop );
            aTabStop.GetTabPos() = 9071;  
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
            aAttr.Insert( aTabStop );
            rOut << aAttr;
        }
        break;
    case 244: 
        rOut << SvxFontHeightItem(160, 100, RES_CHRATR_FONTSIZE);
        rOut << SvxEscapementItem(6 * 100 / 24, 100, RES_CHRATR_ESCAPEMENT);
        break;
    case 245: 
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 246: 
    case 247: 
    case 248: 
        rOut << SvxLRSpaceItem(720, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxPostureItem(
                    rOut.GetPostureItalic()?ITALIC_NONE:ITALIC_NORMAL, RES_CHRATR_POSTURE);
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 249: 
        rOut << SvxLRSpaceItem(720, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE);
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 250: 
        rOut << SvxLRSpaceItem(720, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 251: 
        rOut << SvxLRSpaceItem(360, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE);
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        break;
    case 252: 
        rOut << SvxLRSpaceItem(360, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        break;
    case 253: 
        rOut << SvxULSpaceItem(120, 0, RES_UL_SPACE);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 254: 
        rOut << SvxULSpaceItem(240, 0, RES_UL_SPACE);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE);
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 255: 
        rOut << SvxLRSpaceItem(720, 0, 0, 0, RES_LR_SPACE);
        break;
    case 0: 
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    default: 
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    }
}

void Ww1StyleSheet::OutOne(Ww1Shell& rOut, Ww1Manager& rMan, sal_uInt16 stc)
{
    const RES_POOL_COLLFMT_TYPE RES_NONE = RES_POOLCOLL_DOC_END;
    RES_POOL_COLLFMT_TYPE aType = RES_NONE;

    switch (stc)
    {
    case 222: 
        aType = RES_POOLCOLL_TEXT; break;   
    case 223: 
        break;
    case 224: 
        break;
    case 225: 
        aType = RES_POOLCOLL_TOX_CNTNT8; break;
    case 226: 
        aType = RES_POOLCOLL_TOX_CNTNT7; break;
    case 227: 
        aType = RES_POOLCOLL_TOX_CNTNT6; break;
    case 228: 
        aType = RES_POOLCOLL_TOX_CNTNT5; break;
    case 229: 
        aType = RES_POOLCOLL_TOX_CNTNT4; break;
    case 230: 
        aType = RES_POOLCOLL_TOX_CNTNT3; break;
    case 231: 
        aType = RES_POOLCOLL_TOX_CNTNT2; break;
    case 232: 
        aType = RES_POOLCOLL_TOX_CNTNT1; break;
    case 233: 
        break;
    case 234: 
        break;
    case 235: 
        break;
    case 236: 
        break;
    case 237: 
        aType = RES_POOLCOLL_TOX_IDX3; break;
    case 238: 
        aType = RES_POOLCOLL_TOX_IDX2; break;
    case 239: 
        aType = RES_POOLCOLL_TOX_IDX1; break;
    case 240: 
        break;
    case 241: 
        break;
    case 242: 
        aType = RES_POOLCOLL_FOOTER; break;
    case 243: 
        aType = RES_POOLCOLL_HEADER; break;
    case 244: 
        break;
    case 245: 
        aType = RES_POOLCOLL_FOOTNOTE; break;
    case 246: 
        break;
    case 247: 
        break;
    case 248: 
        break;
    case 249: 
        break;
    case 250: 
        aType = RES_POOLCOLL_HEADLINE5; break;
    case 251: 
        aType = RES_POOLCOLL_HEADLINE4; break;
    case 252: 
        aType = RES_POOLCOLL_HEADLINE3; break;
    case 253: 
        aType = RES_POOLCOLL_HEADLINE2; break;
    case 254: 
        aType = RES_POOLCOLL_HEADLINE1; break;
    case 255: 
        aType = RES_POOLCOLL_TEXT_IDENT; break;
    case 0: 
        aType = RES_POOLCOLL_STANDARD; break;
    }
    if (aType == RES_NONE)
        rOut.BeginStyle(stc, GetStyle(stc).GetName() );
    else
        rOut.BeginStyle(stc, aType);
    OutDefaults(rOut, rMan, stc);
    GetStyle(stc).Out(rOut, rMan);
    rOut.EndStyle();
}


void Ww1StyleSheet::OutOneWithBase(Ww1Shell& rOut, Ww1Manager& rMan,
                                   sal_uInt16 stc, sal_uInt8* pbStopRecur )
{



    Ww1Style& rSty = GetStyle(stc);
    sal_uInt16 nBase = rSty.GetnBase();
    if( nBase != stc
        && !rOut.IsStyleImported( nBase )
        && GetStyle(nBase).IsUsed()
        && !pbStopRecur[nBase] ){

        pbStopRecur[nBase] = 1;
        OutOneWithBase( rOut, rMan, nBase, pbStopRecur ); 
    }
    OutOne( rOut, rMan, stc );
}

void Ww1StyleSheet::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
    sal_uInt16 stc;
    sal_uInt8 bStopRecur[256];
    memset( bStopRecur, sal_False, sizeof(bStopRecur) );


    for (stc=0;stc<Count();stc++)
        if (GetStyle(stc).IsUsed() && !rOut.IsStyleImported( stc ) )
            OutOneWithBase( rOut, rMan, stc, bStopRecur );


    for (stc=0;stc<Count();stc++){
        Ww1Style& rSty = GetStyle(stc);
        if ( rSty.IsUsed() ){
            sal_uInt16 nNext = rSty.GetnNext();
            if( nNext != stc && GetStyle(nNext).IsUsed() )
                rOut.NextStyle( stc, nNext );
        }
    }
}


static sal_uLong GuessPicSize(W1_PIC* pPic)
{
    sal_uInt16 maxx = pPic->mfp.xExtGet();
    sal_uInt16 padx = ((maxx + 7) / 8) * 8;
    sal_uInt16 maxy = pPic->mfp.yExtGet();
    return 120L + (sal_uLong)padx * maxy;
}




void Ww1Picture::WriteBmp(SvStream& rOut)
{
    long nSize = pPic->lcbGet() - (sizeof(*pPic)-sizeof(pPic->rgb));
    sal_uInt8* p = pPic->rgbGet();
    sal_uInt16 maxx = pPic->mfp.xExtGet();
    sal_uInt16 padx = ((maxx + 7) / 8) * 8;
    sal_uInt16 maxy = pPic->mfp.yExtGet();

     p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
     p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 x = SVBT16ToShort(p);
    (void) x;
#endif
    p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 y = SVBT16ToShort(p);
    (void) y;
#endif
    p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 planes = SVBT16ToShort(p);
    (void) planes;
#endif
    p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 bitcount = SVBT16ToShort(p);
    (void) bitcount;
#endif
    p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(x==maxx, "Ww1Picture");
    OSL_ENSURE(y==maxy, "Ww1Picture");
    OSL_ENSURE(planes==1, "Ww1Picture");
    OSL_ENSURE(bitcount==4, "Ww1Picture");
#endif

    OSL_ENSURE(16*3+padx*maxy/2==nSize, "Ww1Picture");

    SVBT32 tmpLong;
    SVBT16 tmpShort;
    sal_uInt8 tmpByte;
#define wLong(n) \
    UInt32ToSVBT32(n, tmpLong); \
    if ((rOut.Write(tmpLong, sizeof(SVBT32))) != sizeof(SVBT32)) goto error;
#define wShort(n) \
    ShortToSVBT16(n, tmpShort); \
    if ((rOut.Write(tmpShort, sizeof(SVBT16))) != sizeof(SVBT16)) goto error;
#define wByte(n) \
    tmpByte = n; \
    if ((rOut.Write(&tmpByte, sizeof(sal_uInt8))) != sizeof(sal_uInt8)) goto error;
    wByte('B'); wByte('M');
    wLong(54 + 4 * 16 + padx * maxy);
    wLong(0);
    wLong(54 + 4 * 16);
    wLong(40);
    wLong(maxx);
    wLong(maxy);
    wShort(1);
    wShort(8);
    wLong(0);
    wLong(0);
    wLong(0);
    wLong(0);
    wLong(16);
    wLong(16);
    sal_uInt16 i;
    for (i=0;nSize>0&&i<16;i++)
    {
        wByte(*p);
        p++;
        nSize -= sizeof(sal_uInt8);
        wByte(*p);
        p++;
        nSize -= sizeof(sal_uInt8);
        wByte(*p);
        p++;
        nSize -= sizeof(sal_uInt8);
        wByte(0);
    }
    OSL_ENSURE(padx*maxy/2==nSize, "Ww1Picture");
    sal_uInt16 j;
    {
        sal_uInt8* pBuf = new sal_uInt8[padx];
        for (j=0;nSize>0&&j<maxy;j++)
        {
            sal_uInt8* q = pBuf;
            for (i=0;nSize>0&&i<maxx;i+=2)
            {
                *q++ = *p>>4;
                *q++ = *p&0xf;
                p++;
                nSize -= sizeof(sal_uInt8);
            }
            for (;i<padx;i+=2)
            {
                *q++ = 0;
                p++;
                nSize -= sizeof(sal_uInt8);
            }
            if(rOut.Write(pBuf, padx) != padx){
                delete [] pBuf;
                goto error;
            }
        }
        delete [] pBuf;
    }
    OSL_ENSURE(nSize==0, "Ww1Picture");
#undef wLong
#undef wShort
#undef wByte
    rOut.Seek(0);
    return;
error:
    ;
}

void Ww1Picture::Out(Ww1Shell& rOut, Ww1Manager& /*rMan*/)
{
    Graphic* pGraphic = 0;
    sal_uInt16 mm;
    switch (mm = pPic->mfp.mmGet())
    {
    case 8: 
    {
        SvMemoryStream aOut(8192, 8192);
        aOut.Write(pPic->rgbGet(), pPic->lcbGet() -
         (sizeof(*pPic)-sizeof(pPic->rgb)));
        aOut.Seek(0);
        GDIMetaFile aWMF;
        if (ReadWindowMetafile( aOut, aWMF, NULL ) && aWMF.GetActionSize() > 0)
        {
            aWMF.SetPrefMapMode(MapMode(MAP_100TH_MM));
            Size aOldSiz(aWMF.GetPrefSize());
            Size aNewSiz(pPic->mfp.xExtGet(), pPic->mfp.yExtGet());
            Fraction aFracX(aNewSiz.Width(), aOldSiz.Width());
            Fraction aFracY(aNewSiz.Height(), aOldSiz.Height());
            aWMF.Scale(aFracX, aFracY);
            aWMF.SetPrefSize(aNewSiz);
            pGraphic = new Graphic(aWMF);
        }
        break;
    }
    case 94: 
    case 98: 
    {
        OUString aDir( (sal_Char*)pPic->rgbGet(),
                (sal_uInt16)(pPic->lcbGet() - (sizeof(*pPic)-sizeof(pPic->rgb))),
                RTL_TEXTENCODING_MS_1252 );

        rOut.AddGraphic( aDir );
    }
    break;
    case 97: 
    {
        sal_uLong nSiz = GuessPicSize(pPic);
        SvMemoryStream aOut(nSiz, 8192);
        WriteBmp(aOut);
        Bitmap aBmp;
        ReadDIB(aBmp, aOut, true);
        pGraphic = new Graphic(aBmp);
    }
    default:
        OSL_ENSURE(pPic->mfp.mmGet() == 97, "Ww1Picture");
    }
    if (pGraphic)
        rOut << *pGraphic;
}

void Ww1HeaderFooter::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    
    
    if (!rMan.Pushed())
    {
        while (++(*this))
            switch (eHeaderFooterMode)
            {
            case FtnSep:
            break;
            case FtnFollowSep:
            break;
            case FtnNote:
            break;
            case EvenHeadL:
            break;
            case OddHeadL:
            {
                sal_uLong begin = 0;
                sal_uLong end = 0;
                if (FillOddHeadL(begin, end))
                {
                    Ww1HddText* pText = new Ww1HddText(rMan.GetFib());
                    pText->Seek(begin);
                    pText->SetCount(end-begin);
                    rOut.BeginHeader();
                    rMan.Push1(pText, pText->Offset(rMan.GetFib()), begin,
                     new Ww1HeaderFooterFields(rMan.GetFib()));
                    rOut << rMan;
                    rMan.Pop();
                    rOut.EndHeaderFooter();
                    return;
                }
            }
            break;
            case EvenFootL:
            break;
            case OddFootL:
            {
                sal_uLong begin = 0;
                sal_uLong end = 0;
                if (FillOddFootL(begin, end))
                {
                    Ww1HddText* pText = new Ww1HddText(rMan.GetFib());
                    pText->Seek(begin);
                    pText->SetCount(end-begin);
                    rOut.BeginFooter();
                    rMan.Push1(pText, pText->Offset(rMan.GetFib()), begin,
                     new Ww1HeaderFooterFields(rMan.GetFib()));
                    rOut << rMan;
                    rMan.Pop();
                    rOut.EndHeaderFooter();
                    return;
                }
            }
            break;
            case FirstHeadL:
            break;
            default:
            break;
            }
    }
}

void Ww1HeaderFooter::Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode&)
{
    if (!rMan.Pushed() && eHeaderFooterMode != None)
    {
        Start(rOut, rMan);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
