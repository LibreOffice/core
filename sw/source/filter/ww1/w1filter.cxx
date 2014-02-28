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

// The methods operator<<, Out, Start and Stop are to be used like
// this: If possible, information from the document is transferred to
// the shell with
//   operator<<()
// If additional parameters are required, the method
//   Out()
// needs to be used. The methods for marking a range (attribute range,
// for example), are
//   Start(), Stop()
// This module contains all of these methods. It is necessary for the
// filter, but not for the dumper, with one exception though: the
// module also contains methods, which are useless for the dumper, for
// example because they operate on sv structures like GetFont() does on
// SvxFontItem.

// Manager
Ww1Shell& operator <<(Ww1Shell& rOut, Ww1Manager& This)
{
    // prohibit action in case of recursive call
    if (!This.Pushed())
    {
        {
            // is only needed temporarily
            This.SetInStyle( true );
            Ww1StyleSheet(This.aFib).Out(rOut, This);
            This.SetInStyle( false );
        }
        {
            // this one, too
            Ww1Assoc(This.aFib).Out(rOut);
        }
        This.aDop.Out(rOut);
        // Decide now how page templates will be created
        if (This.GetSep().Count() <= 1)
            rOut.SetUseStdPageDesc();
    }
    // and now the actual dok
    sal_Unicode cUnknown = ' ';
    while (*This.pSeek < This.pDoc->Count())
    {
        // output the ProgressState only for main text, because we
        // cannot determine the correct value otherwise
        if (!This.Pushed())
            ::SetProgressState(This.Where() * 100 / This.pDoc->Count(),
             rOut.GetDoc().GetDocShell());
        // Here, attributes and characters get pumped into the shell
        // alternatingly. The positions get incremented by reading
        // chars from the manager. First, the attributes:
        This.Out(rOut, cUnknown);
        // The text document pDoc is a Ww1PlainText, whose Out()
        // method outputs until the next special character or the
        // requested number of characters has been reached.
        cUnknown = This.pDoc->Out(rOut, *This.pSeek);
    }
    This.SetStopAll(true);
    This.OutStop(rOut, cUnknown);   // So that attributes get closed at the end
    This.SetStopAll(false);
    return rOut;
}

void Ww1Manager::OutStop(Ww1Shell& rOut, sal_Unicode cUnknown)
{
    // bookmarks don't need to be ended?
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
    // start all attributes which need it
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
    // determine next event
    sal_uLong ulEnd = pDoc->Count(); // at the latest at the end of the text
    if (!Pushed())
        if (ulEnd > aSep.Where()) // next Sep prior?
            ulEnd = aSep.Where();
    if (true)
        if (ulEnd > aPap.Where()) // next Pap prior?
            ulEnd = aPap.Where();
    if (true)
        if (ulEnd > aChp.Where()) // next Chp prior?
            ulEnd = aChp.Where();
    if (!Pushed())
        if (ulEnd > aFtn.Where()) // next Ftn prior?
            ulEnd = aFtn.Where();
    if (pFld)
        if (ulEnd > pFld->Where()) // next Fld prior?
            ulEnd = pFld->Where();
    if (!Pushed())
        if (ulEnd > aBooks.Where()) // next Bookmark prior?
            ulEnd = aBooks.Where();
    *pSeek = Where(); // current position
    if (*pSeek < ulEnd)
        *pSeek = ulEnd;
}

void Ww1Manager::Out(Ww1Shell& rOut, sal_Unicode cUnknown)
{
// Depending on the mode, fields, footnotes, character attributes,
// paragraph attributes and sections are handled like this: First, the
// Stop() method is called. If the object determines that there is
// something to end (obviously not during the first pass), it will be
// ended, otherwise the call is without effect. Next, unhandled
// special characters get output. Then, all Start() methods get
// called, and then Where() is used to find the next interesting
// position.

// With the manager in 'push'ed mode, some elements will be
// skipped. Fields are only skipped under special circumstances, more
// precisely only during output of ranges which cannot contain
// fields. Character and paragraph attributes are never skipped. The
// if (1) instructions were added for uniformity.

// First, some special-casing for tables:
// the important attributes are best queried before they end
// Optimization: they only get set to meaningful values when the 0x07
// character is pending.

    bool bLIsTtp = false;
    sal_Bool bLHasTtp = sal_False;
    if( cUnknown == 0x07 )
    {
        bLIsTtp = IsInTtp();
        bLHasTtp = HasTtp();
    }

    OutStop( rOut, cUnknown );      // End attrs if necessary

    // interpret meta characters:
    if (!Ww1PlainText::IsChar(cUnknown))
        switch (cUnknown)
        {
        case 0x02:
            // dontknow
        break;
        case 0x07: // table
            if (rOut.IsInTable() && HasInTable() && !bLIsTtp && !bLHasTtp)
                rOut.NextTableCell();
        break;
        case 0x09: // tab
            rOut.NextTab();
        break;
        case 0x0a: // linefeed
            rOut.NextParagraph();
        break;
        case 0x0b: // linebreak
            if (rOut.IsInTable())
                ;
            else
                rOut.NextLine();
        break;
        case 0x0d: // carriage return
            // ignore
        break;
        case 0x0c: // pagebreak
            rOut.NextPage();
        break;
        case 0x14: // sectionendchar
            // ignore here
        break;
        default:
        break;
        }

    OutStart( rOut );   // End attrs if necessary and calculate next pos
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

// ulSeek is the FC distance between start of main text and start of special text
// ulSeek2 is this special text's offset in the special case range
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
    if( rName.startsWith( "_Toc" ) ) // "_Toc*" is superfluous
        return;

    if( rOut.IsFlagSet( SwFltControlStack::HYPO )
        && rName.equalsIgnoreAsciiCase( "FORMULAR" ) )
        rOut.SetProtect();

    // Set for transformation bookmark -> variable
    long nLen = Len();
    if( nLen > MAX_FIELDLEN )
        nLen = MAX_FIELDLEN;

    // read content of the bookmark
    // Would supposedly also work via Ww1PlainText
    OUString aVal( rMan.GetText().GetText( Where(), nLen ) );

    // in 2 steps, since OS/2 is too stupid
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
            // first byte should be foot note marker
            pText->Out(c);
            OSL_ENSURE(c==0x02, "Ww1Footnotes");
            count--; // for the just read marker byte
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
            ++(*this); // ignore
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
            // Do we have a complete field for insertion?
            {
                rOut << *pField;
                delete pField;
                pField = 0;
            // The filter does it like this so that attributes
            // applying to this field actually get read and applied
            }
            if (!sResult.isEmpty())
                rOut << sResult;
        }
    }
}

enum WWDateTime{ WW_DONTKNOW = 0x0, WW_DATE = 0x1, WW_TIME = 0x2, WW_BOTH = 0x3 };

static WWDateTime GetTimeDatePara( const OUString& rForm,
                                    SwTimeFormat* pTime = 0,
                                     SwDateFormat* pDate = 0 )
{
    WWDateTime eDT = WW_BOTH;
    if (rForm.indexOf('H') != -1)         // H    -> 24h
    {
        if( pTime )
            *pTime = TF_SSMM_24;
    }
    else if (rForm.indexOf('H') != -1)    // h    -> 24h
    {
        if( pTime )
            *pTime = TF_SSMM_12;
    }
    else                                    // no time
    {
        eDT = (WWDateTime)( eDT & ~(sal_uInt16)WW_TIME );
    }

    sal_Int32 nDPos = 0;
    while (true)
    {
        nDPos = rForm.indexOf('M', nDPos);     // M    -> date
        if (nDPos == 0 || nDPos == -1)
            break;
        sal_Unicode cPrev = rForm[nDPos - 1];
        // Ignore "AM", "aM", "PM", "pM" here
        if( 'a' != cPrev && 'A' != cPrev && 'p' != cPrev && 'P' != cPrev )
            break;
        // else search again
        ++nDPos;
    }

    if (nDPos != -1)                  // month -> date ?
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

                    //  M, MM -> numeric month
                    //  MMM, MMMM -> text. month
        sal_Bool bLitMonth = rForm.indexOf("MMM") != -1;
                    //  MMMM -> full month
        sal_Bool bFullMonth = rForm.indexOf("MMMM") != -1;
                    //  jj, JJ -> 2-col-year
                    //  jjjj, JJJJ -> 4-col-year
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
    OUString sType; // the type as string
    OUString sFormula;
    OUString sFormat;
    OUString sDTFormat;   // date/time format
    W1_FLD* pData = GetData(); // data bound to plc
    OSL_ENSURE(pData->chGet()==19, "Ww1Fields"); // should be begin

    sal_Unicode c;
    rMan.Fill( c );
    OSL_ENSURE(c==19, "Ww1Fields"); // should also be begin
    if (pData->chGet()==19 && c == 19)
    {
        OUString aStr;
        c = rMan.Fill( aStr, GetLength() );
        OSL_ENSURE(Ww1PlainText::IsChar(c), "Ww1Fields");
        sal_Int32 pos = aStr.indexOf(' ');
        // get type out of text
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
        sFormula += aStr;
        sal_uInt8 rbType = pData->fltGet();
        do {
            // Read the formula part until the entire field ends or
            // the result part starts. Of course, other fields can
            // start during this (Word supports nested fields).
            ++(*this);
            pData = GetData();
            if (pData->chGet()==19) // nested field
            {
                Out(rOut, rMan, nDepth+1);
                rMan.Fill(c);
                OSL_ENSURE(c==21, "Ww1PlainText");
                sFormula += "Ww" + OUString::number(nPlcIndex);
                c = rMan.Fill(aStr, GetLength());
                OSL_ENSURE(Ww1PlainText::IsChar(c), "Ww1PlainText");
                sFormula += aStr;
            }
        }
        while (pData->chGet()==19);

        // get format out of text
        pos = sFormula.indexOf("\\*");
        if (pos != -1)
        {
            sFormat = sFormula.copy(pos);
            sFormula = sFormula.copy(0, pos);
        }

        pos = sFormula.indexOf( "\\@" );
        if (pos != -1)
        {
            sDTFormat = sFormula.copy(pos);
            sFormula = sFormula.copy(0, pos);
        }

        // The formula part is done, does a result part follow?
        if( pData->chGet() == 20 )
        {
            rMan.Fill( c );
            OSL_ENSURE(c==20, "Ww1PlainText");
            c = rMan.Fill(sResult, GetLength());
            if (!Ww1PlainText::IsChar(c))
                sResult += OUString(c); //~ mdt: handle special chars
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
        case 3: // bookmark reference
            sFormula = rOut.ConvertUStr(sFormula);
            pField = new SwGetRefField( (SwGetRefFieldType*)
                rOut.GetSysFldType( RES_GETREFFLD ),
                sFormula,
                REF_BOOKMARK,
                0,
                REF_CONTENT );

        break;
        case 6: // set command
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
            // Invisible causes trouble in 378; should work soon

            // Ignoring bookmarks is not implemented
        }
        break;
        case 14: // info variable
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

            // huge pile of shit: the type 'info' can represent one of
            // types 15 to 31. it contains as formula the actual field
            // of the doc info.
            // Don't use ';' with the following macro
#define IS(sd, se, t) \
    if (aSubType == sd || aSubType == se) \
        rbType = t; \
    else

            // german name       english name    type-code
            IS("titel",          "title",          15)
            IS("thema",          "subject",        16)
            IS("autor",          "author",         17)
            IS("stichw?rter",    "keywords",       18) //~ mdt: umlaut
            IS("kommentar",      "comment",        19)
            IS("gespeichertvon", "lastrevisedby",  20)
            IS("ertelldat",      "creationdate",   21)
            IS("speicherdat",    "revisiondate",   22)
            IS("druckdat",       "printdate",      23)
            IS("version",        "revisionnumber", 24)
            IS("zeit",           "edittime",       25)
            IS("anzseit",        "numberofpages",  26)
            IS("anzw?rter",      "numberofwords",  27) //~ mdt: umlaut
            IS("anzzeichen",     "numberofchars",  28)
            IS("dateiname",      "filename",       29)
            IS("vorlage",        "templatename",   30)
                bKnown = false;
#undef IS
            if (rbType != 14)
                goto oncemore;
        }
        break;
        case 15: // title
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_TITEL, OUString(), 0);
        break;
        case 16: // subject
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_THEMA, OUString(), 0);
        break;
        case 17: // author
            pField = new SwAuthorField((SwAuthorFieldType*)
             rOut.GetSysFldType(RES_AUTHORFLD), AF_NAME );
        break;
        case 18: // keywords
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_KEYS, OUString(), 0);
        break;
        case 19: // comments
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_COMMENT, OUString(), 0);
        break;
        case 20: // last revised by
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_CHANGE|DI_SUB_AUTHOR, OUString());
        break;
        case 21: // creation date
        case 22: // revision date
        case 23: // print date
        case 25:{// edit time
                    sal_uInt16 nSub;
                    sal_uInt16 nReg = 0;    // RegInfoFormat, DefaultFormat for DocInfo fields

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
                        // WW_DONTKNOW -> Default already set
                    }
                    pField = new SwDocInfoField((SwDocInfoFieldType*)
                        rOut.GetSysFldType(RES_DOCINFOFLD), nSub | nReg, OUString());
                }
        break;
        case 24: // revision number
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD),  DI_DOCNO, OUString(), 0);
        break;
        case 26: // number of pages
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_PAGE, SVX_NUM_ARABIC);
        break;
        case 27: // number of words
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_WORD, SVX_NUM_ARABIC);
        break;
        case 28: // number of chars
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_CHAR, SVX_NUM_ARABIC);
        break;
        case 29: // file name
            pField = new SwFileNameField((SwFileNameFieldType*)
             rOut.GetSysFldType(RES_FILENAMEFLD));
        break;
        case 30: // doc template name
            pField = new SwTemplNameField((SwTemplNameFieldType*)
             rOut.GetSysFldType(RES_TEMPLNAMEFLD), FF_NAME);
        break;
        case 31:
        case 32:{
                    SwDateFormat aDate = DF_SSYS;
                    SwTimeFormat aTime = TF_SYSTEM;

                    WWDateTime eDT = GetTimeDatePara(sDTFormat, &aTime, &aDate);
                    if( eDT == WW_DONTKNOW )        // no D/T-format string
                        eDT = ( rbType == 32 ) ? WW_TIME : WW_DATE;    // use ID

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
                                // Cheat: insert directly and a space afterwards
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
        case 33: // current page
            pField = new SwPageNumberField((SwPageNumberFieldType*)
             rOut.GetSysFldType(RES_PAGENUMBERFLD), PG_RANDOM, SVX_NUM_ARABIC);
        break;
        case 34: // evaluation exp
        {
            if (nDepth == 0)
            {
                SwGetExpFieldType* pFieldType =
                 (SwGetExpFieldType*)rOut.GetSysFldType(RES_GETEXPFLD);
                OSL_ENSURE(pFieldType!=0, "Ww1Fields");
                if (pFieldType != 0)
                    pField = new SwGetExpField(pFieldType, sFormula,
                     nsSwGetSetExpType::GSE_STRING, VVF_SYS);
            }
            else // recursion:
            {
                OUString aName("Ww");
                aName += OUString::number( nPlcIndex );
                SwFieldType* pFT = rOut.GetDoc().GetFldType( RES_SETEXPFLD, aName, false);
                if (pFT == 0)
                {
                    SwSetExpFieldType aS(&rOut.GetDoc(), aName, nsSwGetSetExpType::GSE_FORMULA);
                    pFT = rOut.GetDoc().InsertFldType(aS);
                }
                SwSetExpField aFld((SwSetExpFieldType*)pFT, sFormula);
                aFld.SetSubType(nsSwExtendedSubType::SUB_INVISIBLE);
                rOut << aFld;
            }
        }
        break;
        case 36: // print command, insert file
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
        case 37: // page ref
            pField = new SwGetRefField(
             (SwGetRefFieldType*)rOut.GetSysFldType(RES_GETREFFLD),
             sFormula, 0, 0, REF_PAGE);
        break;
        case 38: // ask command
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
        case 39: // filling command
            pField = new SwInputField(
                static_cast<SwInputFieldType*>(rOut.GetSysFldType( RES_INPUTFLD )),
                OUString(), sFormula,
                INP_TXT, 0, false );
        break;
        case 51: // macro button
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
        case 55: // read tiff / or better: import anything
        {
            const sal_Unicode* pFormula = sFormula.getStr();
            const sal_Unicode* pDot = 0;
            OUString sName;
            while (*pFormula != '\0' && *pFormula != ' ')
            {
                // from here on an extension could appear
                if (*pFormula == '.')
                    pDot = pFormula;
                else
                    // so far we were in directories
                    if (*pFormula == '\\')
                    {
                        pDot = 0;
                        if (pFormula[1] == '\\')
                            pFormula++;
                    }
                if (*pFormula != '\0')
                    sName += OUString(*pFormula++);
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
        default: // unknown
            OSL_ENSURE(false, "Ww1PlainText");
        // unsupported:
        case 1: // unknown
        case 2: // possible bookmark
        case 4: // index entry
        // wwpar5: 1351/1454
        case 5: // footnote ref
        case 7: // if command
        case 8: // create index
        // wwpar5: 1351/1454
        case 9: // table of contents entry
        // wwpar5: 1351/1454
        case 10: // style ref
        case 11: // doc ref
        case 12: // seq ref
        case 13: // create table of contents
        // wwpar5: 1351/1454
        case 35: // literal text
        // print merge:
        case 40: // data command
        case 41: // next command
        case 42: // nextif command
        case 43: // skipif command
        case 44: // number of record

        case 45: // dde ref
        case 46: // dde auto ref
        case 47: // glossary entry
        case 48: // print char
        case 49: // formula def
        case 50: // goto button
        case 52: // auto number outline
        case 53: // auto number legal
        case 54: // auto number arabic
            bKnown = false;
        break;
        }
        if( bKnown || sResult == "\270" )
            this->sResult = "";
        else
            this->sResult = sResult;
    }
    else // oops: we are terribly wrong: skip this
        ++(*this);
}

sal_uLong Ww1Fields::GetLength()
{
    // Calculates a field part's length, excluding the terminating
    // chars in the text (19, 20, 21) meaning begin, separator and
    // end, respectively.
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

        // As soon as we've reached reading the start of the current
        // sep's attribute, we insert it. This method is the same for
        // most member classes of the manager.
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
        // Supply the attributes to be started
        if (FillStart(pByte, cb))
        {
            Ww1SprmPapx aSprm(pByte, cb);
            // and give the following output
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

// Currently, the output of W1CHPxes cannot be solved only by define...
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
        case 0: rOut << SvxUnderlineItem(UNDERLINE_NONE, RES_CHRATR_UNDERLINE)
                     << SvxWordLineModeItem(false, RES_CHRATR_WORDLINEMODE);
                break;
        case 1: rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE);
                break;
        case 2: rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE)
                     << SvxWordLineModeItem(true, RES_CHRATR_WORDLINEMODE);
                break;
        case 3: rOut << SvxUnderlineItem(UNDERLINE_DOUBLE, RES_CHRATR_UNDERLINE);
                break;
        case 4: rOut << SvxUnderlineItem(UNDERLINE_DOTTED, RES_CHRATR_UNDERLINE);
                break;
        default: OSL_ENSURE(false, "Chpx");
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
            // First all toggle flags
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
            // then all number values; these have flags when they are set...
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
            OSL_ENSURE( !nPlcIndex, "Lost Chp-Attribut-Stop" );
        }
    }
}

void Ww1Style::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
    // Base first, so that base style attributes can be recognized
    // first: Base
    if(pParent->GetStyle(stcBase).IsUsed() )    // Base valid ?
        rOut.BaseStyle(stcBase);

    // next of all: CHP
    aChpx.Out(rOut, rMan);
    // Last: PAP
    if (pPapx)
        pPapx->Start(rOut, rMan);
}

// The Out() methods output a number of characters to the shell, a
// string or a character. The amount of output is determined by ulEnd,
// which specifies the position at which to stop. Control characters
// also end the output.
// They are defined by MinChar. All characters with a value lower than
// that are treated as control characters. This classification is
// handled by IsChar. If a control character is encountered, the
// output method returns immediately and returns it. Pay attention to
// this; ulEnd might not have been reached if !IsChar(Out(...)). In
// this case, the control character has been consumed, but not been
// output.
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
    // Like Out(Shell..., but output into a string
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

// This makes ulEnd unnecessary, right?
sal_Unicode Ww1PlainText::Out( sal_Unicode& rRead )
{
    rRead = (*this)[ulSeek];
    ++(*this);
    return rRead;
}

void Ww1SprmPapx::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if( !rMan.IsInStyle() ){        // Normal attributes apply within style
        if (!rOut.IsInFly()
            && !rOut.IsInTable()
            && ( rMan.HasPPc() || rMan.HasPDxaAbs())){ // Fly-Start
            rOut.BeginFly();        // eAnchor );
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

    if( !rMan.IsInStyle() )         // Normal attributes apply within style
    {
        if (rOut.IsInTable() &&( rMan.IsStopAll() || !rMan.HasInTable()))
            rOut.EndTable();

        if( rOut.IsInFly() &&
            ( rMan.IsStopAll()
                || ( !rMan.HasPPc() && !rMan.HasPDxaAbs()   // Fly end
                    && !rOut.IsInTable())))
            rOut.EndFly();
    }
}

SvxFontItem Ww1Fonts::GetFont(sal_uInt16 nFCode)
{
    // Create a font in sw fashion from existing word structures
    FontFamily eFamily = FAMILY_DONTKNOW;
    OUString aName;
    FontPitch ePitch = PITCH_DONTKNOW;
    rtl_TextEncoding eCharSet = RTL_TEXTENCODING_DONTKNOW;
    switch (nFCode)
    {
    // In the Winword 1.x format, the names of the first three fonts were
    // omitted from the table and assumed to be "Tms Rmn" (for ftc = 0),
    // "Symbol", and "Helv"
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
            // Fontname
            aName = OUString( (sal_Char*)pF->szFfnGet(), strlen( (sal_Char*)pF->szFfnGet() ),
                            RTL_TEXTENCODING_MS_1252 );
            // Pitch
            static const FontPitch ePitchA[] =
            {
                PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, PITCH_DONTKNOW
            };
            ePitch = ePitchA[pF->prgGet()];
            // CharSet
            eCharSet = RTL_TEXTENCODING_MS_1252;
            if (aName.equalsIgnoreAsciiCase("Symbol")
             || aName.equalsIgnoreAsciiCase("Symbol Set")
             || aName.equalsIgnoreAsciiCase("Wingdings")
             || aName.equalsIgnoreAsciiCase("ITC Zapf Dingbats") )
                eCharSet = RTL_TEXTENCODING_SYMBOL;
            // FontFamily
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
            OSL_ENSURE(false, "WW1Fonts::GetFont: Non-existent font !");
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
        // VCL matches fonts on its own
        // Unfortunately, this breaks for Helv, Tms Rmn and System
        // Monospaced, so they need to be replaced.
        // According to TH they should be replaced by fixed values,
        // not using System::GetStandardFont, so no name lists appear
        // (this could confuse the user)
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
    //~ mdt: missing
    long nDefTabSiz = aDop.dxaTabGet();
    if (nDefTabSiz < 56)
        nDefTabSiz = 709;

    // we want exactly one DefaultTab
    SvxTabStopItem aNewTab(1, sal_uInt16(nDefTabSiz), SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP);
    ((SvxTabStop&)aNewTab[0]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;
    rOut.GetDoc().GetAttrPool().SetPoolDefaultItem( aNewTab); //~ mdt: better (GetDoc)

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
    aInfo = rOut.GetDoc().GetFtnInfo();     // Copy-Ctor private
                // where positioned ? ( 0 == Section, 1 == Page,
                // 2 == next to Text -> Page, 3 == Doc  )
    switch( rDOP.fpcGet() ){
    case 1:
    case 2: aInfo.ePos = FTNPOS_PAGE; break;
    default: aInfo.ePos = FTNPOS_CHAPTER; break;
    }
    // Always doc, because Sw seems to interpret Chapter differently from PMW
    aInfo.eNum = FTNNUM_DOC;
                            // how to renumber ?
                            // SW-UI allows number only with FTNNUM_DOC
    if( rDOP.nFtnGet() > 0 && aInfo.eNum == FTNNUM_DOC )
        aInfo.nFtnOffset = rDOP.nFtnGet() - 1;
    rOut.GetDoc().SetFtnInfo( aInfo );

}

void Ww1Assoc::Out(Ww1Shell& rOut)
{
    //~ mdt: missing: FileNext, Dot, DataDoc, HeaderDoc, Criteria1,
    // Criteria2, Criteria3, Criteria4, Criteria5, Criteria6, Criteria7
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
    case 222: // Null
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 223: // annotation reference
        rOut << SvxFontHeightItem(160, 100, RES_CHRATR_FONTSIZE);
        break;
    case 224: // annotation text
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 225: // table of contents 8
    case 226: // table of contents 7
    case 227: // table of contents 6
    case 228: // table of contents 5
    case 229: // table of contents 4
    case 230: // table of contents 3
    case 231: // table of contents 2
    case 232: // table of contents 1
        rOut << SvxLRSpaceItem(( 232 - stc ) * 720, 720, 0, 0, RES_LR_SPACE);
            // tabs still missing !
        break;
    case 233: // index 7
    case 234: // and index 6
    case 235: // and index 5
    case 236: // and index 4
    case 237: // and index 3
    case 238: // and index 2
        rOut << SvxLRSpaceItem(( 239 - stc ) * 360, 0, 0, 0, RES_LR_SPACE);
        break;
    case 239: // index 1
        break;
    case 240: // line number
        break;
    case 241: // index heading
        break;
    case 242:  // footer
    case 243:{ // ... and header
            SvxTabStopItem aAttr(RES_PARATR_TABSTOP);
            SvxTabStop aTabStop;
            aTabStop.GetTabPos() = 4535;  // 8 cm
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER;
            aAttr.Insert( aTabStop );
            aTabStop.GetTabPos() = 9071;  // 16 cm
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
            aAttr.Insert( aTabStop );
            rOut << aAttr;
        }
        break;
    case 244: // footnote reference
        rOut << SvxFontHeightItem(160, 100, RES_CHRATR_FONTSIZE);
        rOut << SvxEscapementItem(6 * 100 / 24, 100, RES_CHRATR_ESCAPEMENT);
        break;
    case 245: // footnote text
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 246: // heading 9
    case 247: // und heading 8
    case 248: // und heading 7
        rOut << SvxLRSpaceItem(720, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxPostureItem(
                    rOut.GetPostureItalic()?ITALIC_NONE:ITALIC_NORMAL, RES_CHRATR_POSTURE);
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 249: // heading 6
        rOut << SvxLRSpaceItem(720, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE);
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 250: // heading 5
        rOut << SvxLRSpaceItem(720, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    case 251: // heading 4
        rOut << SvxLRSpaceItem(360, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE);
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        break;
    case 252: // heading 3
        rOut << SvxLRSpaceItem(360, 0, 0, 0, RES_LR_SPACE);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        break;
    case 253: // heading 2
        rOut << SvxULSpaceItem(120, 0, RES_UL_SPACE);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 254: // heading 1
        rOut << SvxULSpaceItem(240, 0, RES_UL_SPACE);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD, RES_CHRATR_WEIGHT);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE);
        rOut << SvxFontHeightItem(240, 100, RES_CHRATR_FONTSIZE);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 255: // Normal indent
        rOut << SvxLRSpaceItem(720, 0, 0, 0, RES_LR_SPACE);
        break;
    case 0: // Normal
        rOut << SvxFontHeightItem(200, 100, RES_CHRATR_FONTSIZE);
        break;
    default: // custom defined
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
    case 222: // Null
        aType = RES_POOLCOLL_TEXT; break;   //???
    case 223: // annotation reference
        break;
    case 224: // annotation text
        break;
    case 225: // table of contents 8
        aType = RES_POOLCOLL_TOX_CNTNT8; break;
    case 226: // table of contents 7
        aType = RES_POOLCOLL_TOX_CNTNT7; break;
    case 227: // table of contents 6
        aType = RES_POOLCOLL_TOX_CNTNT6; break;
    case 228: // table of contents 5
        aType = RES_POOLCOLL_TOX_CNTNT5; break;
    case 229: // table of contents 4
        aType = RES_POOLCOLL_TOX_CNTNT4; break;
    case 230: // table of contents 3
        aType = RES_POOLCOLL_TOX_CNTNT3; break;
    case 231: // table of contents 2
        aType = RES_POOLCOLL_TOX_CNTNT2; break;
    case 232: // table of contents 1
        aType = RES_POOLCOLL_TOX_CNTNT1; break;
    case 233: // index 7
        break;
    case 234: // index 6
        break;
    case 235: // index 5
        break;
    case 236: // index 4
        break;
    case 237: // index 3
        aType = RES_POOLCOLL_TOX_IDX3; break;
    case 238: // index 2
        aType = RES_POOLCOLL_TOX_IDX2; break;
    case 239: // index 1
        aType = RES_POOLCOLL_TOX_IDX1; break;
    case 240: // line number
        break;
    case 241: // index heading
        break;
    case 242: // footer
        aType = RES_POOLCOLL_FOOTER; break;
    case 243: // header
        aType = RES_POOLCOLL_HEADER; break;
    case 244: // footnote reference
        break;
    case 245: // footnote text
        aType = RES_POOLCOLL_FOOTNOTE; break;
    case 246: // heading 9
        break;
    case 247: // heading 8
        break;
    case 248: // heading 7
        break;
    case 249: // heading 6
        break;
    case 250: // heading 5
        aType = RES_POOLCOLL_HEADLINE5; break;
    case 251: // heading 4
        aType = RES_POOLCOLL_HEADLINE4; break;
    case 252: // heading 3
        aType = RES_POOLCOLL_HEADLINE3; break;
    case 253: // heading 2
        aType = RES_POOLCOLL_HEADLINE2; break;
    case 254: // heading 1
        aType = RES_POOLCOLL_HEADLINE1; break;
    case 255: // Normal indent
        aType = RES_POOLCOLL_TEXT_IDENT; break;
    case 0: // Normal
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
// OutOneWithBase() reads a style using OutOne()
// Additionally, it reads the base style, if this has not happened yet
void Ww1StyleSheet::OutOneWithBase(Ww1Shell& rOut, Ww1Manager& rMan,
                                   sal_uInt16 stc, sal_uInt8* pbStopRecur )
{
// SH: reading linearly is shit, because BasedOn cannot be set. Also,
// toggle and modify attributes (tabs, for example) don't work.

    Ww1Style& rSty = GetStyle(stc);
    sal_uInt16 nBase = rSty.GetnBase();
    if( nBase != stc
        && !rOut.IsStyleImported( nBase )
        && GetStyle(nBase).IsUsed()
        && !pbStopRecur[nBase] ){

        pbStopRecur[nBase] = 1;
        OutOneWithBase( rOut, rMan, nBase, pbStopRecur ); // Recursive
    }
    OutOne( rOut, rMan, stc );
}

void Ww1StyleSheet::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
    sal_uInt16 stc;
    sal_uInt8 bStopRecur[256];
    memset( bStopRecur, sal_False, sizeof(bStopRecur) );

// First pass: Styles with base styles recursively
    for (stc=0;stc<Count();stc++)
        if (GetStyle(stc).IsUsed() && !rOut.IsStyleImported( stc ) )
            OutOneWithBase( rOut, rMan, stc, bStopRecur );

// Second pass: Follow-Styles
    for (stc=0;stc<Count();stc++){
        Ww1Style& rSty = GetStyle(stc);
        if ( rSty.IsUsed() ){
            sal_uInt16 nNext = rSty.GetnNext();
            if( nNext != stc && GetStyle(nNext).IsUsed() )
                rOut.NextStyle( stc, nNext );
        }
    }
}

// Picture
static sal_uLong GuessPicSize(W1_PIC* pPic)
{
    sal_uInt16 maxx = pPic->mfp.xExtGet();
    sal_uInt16 padx = ((maxx + 7) / 8) * 8;
    sal_uInt16 maxy = pPic->mfp.yExtGet();
    return 120L + (sal_uLong)padx * maxy;
}

// Reads 4-bit format, writes 8-bit
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
    case 8: // embedded metafile
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
    case 94: // embedded name SH:??? Which one is it? Embedded or Name ?
    case 98: // TIFF name
    {
        OUString aDir( (sal_Char*)pPic->rgbGet(),
                (sal_uInt16)(pPic->lcbGet() - (sizeof(*pPic)-sizeof(pPic->rgb))),
                RTL_TEXTENCODING_MS_1252 );

        rOut.AddGraphic( aDir );
    }
    break;
    case 97: // embedded bitmap
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
    // Only gets called for SEPs anyway, no further validation necessary:
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
