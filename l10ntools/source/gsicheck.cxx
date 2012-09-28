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

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <string>

#include <stdio.h>

#include <rtl/strbuf.hxx>
#include "sal/main.h"
#include "helper.hxx"
#include "tagtest.hxx"
#include "gsicheck.hxx"

sal_Int32 const MAX_GID_LID_LEN = 250;

namespace {

rtl::OString copyUpTo(
    rtl::OString const & text, sal_Int32 start, sal_Int32 maximumLength)
{
    assert(start >= 0 && start <= text.getLength());
    return text.copy(start, std::min(text.getLength() - start, maximumLength));
}

rtl::OString addSuffix(
    rtl::OString const & pathname, rtl::OString const & suffix)
{
    sal_Int32 n = pathname.lastIndexOf('.');
    if (n == -1) {
        fprintf(
            stderr,
            ("Error: pathname \"%s\" does not contain dot to add suffix in"
             " front of\n"),
            pathname.getStr());
        exit(EXIT_FAILURE);
    }
    return pathname.replaceAt(n, 0, suffix);
}

}

/*****************************************************************************/
void PrintMessage( rtl::OString const & aType, rtl::OString const & aMsg, rtl::OString const & aPrefix,
                   rtl::OString const & aContext, sal_Bool bPrintContext, std::size_t nLine, rtl::OString aUniqueId = rtl::OString() )
/*****************************************************************************/
{
    fprintf( stdout, "%s %s, Line %u", aType.getStr(), aPrefix.getStr(), static_cast<unsigned>( nLine ) );
    if ( !aUniqueId.isEmpty() )
        fprintf( stdout, ", UniqueID %s", aUniqueId.getStr() );
    fprintf( stdout, ": %s", aMsg.getStr() );

    if ( bPrintContext )
        fprintf( stdout, "  \"%s\"", aContext.getStr() );
    fprintf( stdout, "\n" );
}

/*****************************************************************************/
void PrintError( rtl::OString const & aMsg, rtl::OString const & aPrefix,
                 rtl::OString const & aContext, sal_Bool bPrintContext, std::size_t nLine, rtl::OString const & aUniqueId = rtl::OString() )
/*****************************************************************************/
{
    PrintMessage( "Error:", aMsg, aPrefix, aContext, bPrintContext, nLine, aUniqueId );
}

bool LanguageOK( rtl::OString const & aLang )
{
    sal_Int32 n = 0;
    rtl::OString t0(aLang.getToken(0, '-', n));
    if (n == -1) {
        return !t0.isEmpty()
            && (helper::isAllAsciiDigits(t0)
                || helper::isAllAsciiLowerCase(t0));
    }
    rtl::OString t1(aLang.getToken(0, '-', n));
    return n == -1
        && !t0.isEmpty() && helper::isAllAsciiLowerCase(t0)
        && !t1.isEmpty() && helper::isAllAsciiUpperCase(t1)
        && !t0.equalsIgnoreAsciiCase(t1);
}

class LazyStream: public std::ofstream
{

private:
    rtl::OString aFileName;
    bool bOpened;

public:
    LazyStream()
    : aFileName()
    , bOpened(false)
    {};

    void SetFileName( const rtl::OString& rFileName )
    {
        aFileName = rFileName;
    };

    void LazyOpen();
};

void LazyStream::LazyOpen()
{
    if ( !bOpened )
    {
        open(aFileName.getStr(), std::ios_base::out | std::ios_base::trunc);
        if (!is_open())
        {
            fprintf( stderr, "\nERROR: Could not open Output-File %s!\n\n",
                     aFileName.getStr() );
            exit ( 4 );
        }
        bOpened = true;
    }
}


//
// class GSILine
//

/*****************************************************************************/
GSILine::GSILine( const rtl::OString &rLine, std::size_t nLine )
/*****************************************************************************/
                : nLineNumber( nLine )
                , bOK( sal_True )
                , bFixed ( sal_False )
                , data_( rLine )
{
    if (rLine.isEmpty()) {
        NotOK();
        return;
    }

    aFormat = FORMAT_SDF;
    sal_Int32 n = 0;
    aUniqId = rLine.getToken(0, '\t', n); // token 0
    aUniqId += "/";
    aUniqId += rLine.getToken(0, '\t', n); // token 1
    aUniqId += "/";
    aUniqId += rLine.getToken(1, '\t', n); // token 3
    aUniqId += "/";
    rtl::OString gid(rLine.getToken(0, '\t', n)); // token 4
    aUniqId += gid;
    aUniqId += "/";
    rtl::OString lid(rLine.getToken(0, '\t', n)); // token 5
    aUniqId += lid;
    aUniqId += "/";
    aUniqId += rLine.getToken(0, '\t', n); // token 6
    aUniqId += "/";
    aUniqId += rLine.getToken(0, '\t', n); // token 7
    rtl::OString length(rLine.getToken(0, '\t', n)); // token 8
    aLineType = rtl::OString();
    aLangId = rLine.getToken(0, '\t', n); // token 9
    aText = rLine.getToken(0, '\t', n); // token 10
    aQuickHelpText = rLine.getToken(1, '\t', n); // token 12
    aTitle = rLine.getToken(0, '\t', n); // token 13
    if (n == -1) {
        NotOK();
        return;
    }
    rLine.getToken(0, '\t', n); // token 14
    if (n != -1) {
        NotOK();
        return;
    }

    // do some more format checks here
    if (!helper::isAllAsciiDigits(length)) {
        PrintError(
            "The length field does not contain a number!", "Line format",
            length, true, GetLineNumber(), GetUniqId());
        NotOK();
    }
    if (!LanguageOK(aLangId)) {
        PrintError(
            "The Language is invalid!", "Line format", aLangId, true,
            GetLineNumber(), GetUniqId());
        NotOK();
    }
    // Limit GID and LID to MAX_GID_LID_LEN chars each for database conformity,
    // see #137575#:
    if (gid.getLength() > MAX_GID_LID_LEN || lid.getLength() > MAX_GID_LID_LEN)
    {
        PrintError(
            (rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("GID and LID may only be "))
             + rtl::OString::valueOf(MAX_GID_LID_LEN)
             + rtl::OString(RTL_CONSTASCII_STRINGPARAM(" chars long each"))),
            "Line format", aLangId, true, GetLineNumber(), GetUniqId());
        NotOK();
    }
}

/*****************************************************************************/
void GSILine::NotOK()
/*****************************************************************************/
{
    bOK = sal_False;
}

/*****************************************************************************/
void GSILine::ReassembleLine()
/*****************************************************************************/
{
    if (GetLineFormat() != FORMAT_SDF) {
        PrintError(
            "Cannot reassemble line of unknown type (internal Error).",
            "Line format", rtl::OString(), false, GetLineNumber(),
            GetUniqId());
        return;
    }
    rtl::OStringBuffer b;
    sal_Int32 n = 0;
    for (sal_Int32 i = 0; i != 10; ++i) {
        b.append(data_.getToken(0, '\t', n)); // token 0--9
        b.append('\t');
    }
    b.append(aText);
    b.append('\t');
    b.append(data_.getToken(1, '\t', n));
        // token 11; should be empty but there are some places in sc not
        // reflected to sources
    b.append('\t');
    b.append(aQuickHelpText);
    b.append('\t');
    b.append(aTitle);
    b.append('\t');
    b.append(data_.getToken(2, '\t', n)); // token 14
    data_ = b.makeStringAndClear();
}

//
// class GSIBlock
//
/*****************************************************************************/
GSIBlock::GSIBlock( sal_Bool PbPrintContext, sal_Bool bSource, sal_Bool bTrans, sal_Bool bRef, sal_Bool bAllowSusp )
/*****************************************************************************/
            : pSourceLine( NULL )
            , pReferenceLine( NULL )
            , bPrintContext( PbPrintContext )
            , bCheckSourceLang( bSource )
            , bCheckTranslationLang( bTrans )
            , bReference( bRef )
            , bAllowSuspicious( bAllowSusp )
            , bHasBlockError( sal_False )
{
}

/*****************************************************************************/
GSIBlock::~GSIBlock()
/*****************************************************************************/
{
    delete pSourceLine;
    delete pReferenceLine;

    for ( size_t i = 0, n = maList.size(); i < n; ++i )
        delete maList[ i ];
    maList.clear();
}

void GSIBlock::InsertLine( GSILine* pLine, const rtl::OString &rSourceLang)
{
    if ( pLine->GetLanguageId() == rSourceLang )
    {
        if ( pSourceLine )
        {
            PrintError( "Source Language entry double. Treating as Translation.", "File format", "", pLine->GetLineNumber(), pLine->GetUniqId() );
            bHasBlockError = sal_True;
            pSourceLine->NotOK();
            pLine->NotOK();
        }
        else
        {
            pSourceLine = pLine;
            return;
        }
    }

    if (!rSourceLang.isEmpty()) // only check blockstructure if source lang is given
    {
        for ( size_t nPos = 0, n = maList.size(); nPos < n; ++nPos )
        {
            if ( maList[ nPos ]->GetLanguageId() == pLine->GetLanguageId() )
            {
                PrintError( "Translation Language entry double. Checking both.", "File format", "", pLine->GetLineNumber(), pLine->GetUniqId() );
                bHasBlockError = sal_True;
                maList[ nPos ]->NotOK();
                pLine->NotOK();
            }
            nPos++;
        }
    }
    maList.push_back( pLine );
}

/*****************************************************************************/
void GSIBlock::SetReferenceLine( GSILine* pLine )
/*****************************************************************************/
{
    pReferenceLine = pLine;
}

/*****************************************************************************/
void GSIBlock::PrintMessage( rtl::OString const & aType, rtl::OString const & aMsg, rtl::OString const & aPrefix,
                             rtl::OString const & aContext, std::size_t nLine, rtl::OString const & aUniqueId )
/*****************************************************************************/
{
    ::PrintMessage( aType, aMsg, aPrefix, aContext, bPrintContext, nLine, aUniqueId );
}

/*****************************************************************************/
void GSIBlock::PrintError( rtl::OString const & aMsg, rtl::OString const & aPrefix,
                           rtl::OString const & aContext, std::size_t nLine, rtl::OString const & aUniqueId )
/*****************************************************************************/
{
    PrintMessage( "Error:", aMsg, aPrefix, aContext, nLine, aUniqueId );
}

/*****************************************************************************/
void GSIBlock::PrintList( ParserMessageList *pList, rtl::OString const & aPrefix,
    GSILine *pLine )
/*****************************************************************************/
{
    for ( size_t i = 0 ; i < pList->size() ; i++ )
    {
        ParserMessage *pMsg = (*pList)[ i ];
        rtl::OString aContext;
        if ( bPrintContext )
        {
            if ( pMsg->GetTagBegin() == -1 )
                aContext = pLine->GetText().copy( 0, 300 );
            else
                aContext = helper::abbreviate( pLine->data_, pMsg->GetTagBegin()-150, 300 );
            aContext = aContext.trim();
        }

        PrintMessage( pMsg->Prefix(), pMsg->GetErrorText(), aPrefix, aContext, pLine->GetLineNumber(), pLine->GetUniqId() );
    }
}

/*****************************************************************************/
sal_Bool GSIBlock::IsUTF8( const rtl::OString &aTestee, sal_Bool bFixTags, sal_Int32 &nErrorPos, rtl::OString &aErrorMsg, sal_Bool &bHasBeenFixed, rtl::OString &aFixed ) const
/*****************************************************************************/
{
    rtl::OUString aUTF8Tester(
        rtl::OStringToOUString(aTestee, RTL_TEXTENCODING_UTF8));
    rtl::OString aTestee2(
        rtl::OUStringToOString(aUTF8Tester, RTL_TEXTENCODING_UTF8));
    sal_Int32 i = 0;
    while (i != std::min(aTestee.getLength(), aTestee2.getLength())
           && aTestee[i] == aTestee2[i])
    {
        ++i;
    }
    if (i != aTestee.getLength() || i != aTestee2.getLength())
    {
        aUTF8Tester = rtl::OUString(aTestee.getStr(), i, RTL_TEXTENCODING_UTF8);
        nErrorPos = aUTF8Tester.getLength();
        aErrorMsg = "UTF8 Encoding seems to be broken";
        return sal_False;
    }

    nErrorPos = helper::indexOfAnyAsciiL(
        aUTF8Tester,
        RTL_CONSTASCII_STRINGPARAM(
            "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0b\x0c\x0e\x0f\x10\x11\x12"
            "\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f"));
    if (nErrorPos != -1)
    {
        aErrorMsg = "String contains illegal character";
        return sal_False;
    }

    if ( bFixTags )
    {
        bHasBeenFixed = sal_False;
        aFixed = rtl::OString();
    }

    return sal_True;
}

/*****************************************************************************/
sal_Bool GSIBlock::TestUTF8( GSILine* pTestee, sal_Bool bFixTags )
/*****************************************************************************/
{
    sal_Int32 nErrorPos = 0;
    rtl::OString aErrorMsg;
    sal_Bool bError = sal_False;
    rtl::OString aFixed;
    sal_Bool bHasBeenFixed = sal_False;
    if ( !IsUTF8( pTestee->GetText(), bFixTags, nErrorPos, aErrorMsg, bHasBeenFixed, aFixed ) )
    {
        rtl::OString aContext(copyUpTo(pTestee->GetText(), nErrorPos, 20));
        PrintError(rtl::OStringBuffer(aErrorMsg).append(RTL_CONSTASCII_STRINGPARAM(" in Text at Position "))
             .append(nErrorPos).getStr(),
            "Text format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId());
        bError = sal_True;
        if ( bHasBeenFixed )
        {
            pTestee->SetText( aFixed );
            pTestee->SetFixed();
        }
    }
    if ( !IsUTF8( pTestee->GetQuickHelpText(), bFixTags, nErrorPos, aErrorMsg, bHasBeenFixed, aFixed ) )
    {
        rtl::OString aContext(
            copyUpTo(pTestee->GetQuickHelpText(), nErrorPos, 20));
        PrintError(rtl::OStringBuffer(aErrorMsg).append(RTL_CONSTASCII_STRINGPARAM(" in QuickHelpText at Position "))
             .append(nErrorPos).getStr(),
            "Text format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId());
        bError = sal_True;
        if ( bHasBeenFixed )
        {
            pTestee->SetQuickHelpText( aFixed );
            pTestee->SetFixed();
        }
    }
    if ( !IsUTF8( pTestee->GetTitle(), bFixTags, nErrorPos, aErrorMsg, bHasBeenFixed, aFixed ) )
    {
        rtl::OString aContext( pTestee->GetTitle().copy( nErrorPos, 20 ) );
        PrintError(rtl::OStringBuffer(aErrorMsg).append(RTL_CONSTASCII_STRINGPARAM(" in Title at Position "))
             .append(nErrorPos).getStr(),
            "Text format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId());
        bError = sal_True;
        if ( bHasBeenFixed )
        {
            pTestee->SetTitle( aFixed );
            pTestee->SetFixed();
        }
    }
    if ( bError )
        pTestee->NotOK();
    return !bError;
}


/*****************************************************************************/
sal_Bool GSIBlock::HasSuspiciousChars( GSILine* pTestee, GSILine* pSource )
/*****************************************************************************/
{
    sal_Int32 nPos = 0;
    if ( !bAllowSuspicious && ( nPos = pTestee->GetText().indexOf("??")) != -1 )
        if ( pSource->GetText().indexOf("??") == -1 )
        {
            rtl::OUString aUTF8Tester(
                rtl::OStringToOUString(
                    pTestee->GetText().copy(0, nPos), RTL_TEXTENCODING_UTF8));
            sal_Int32 nErrorPos = aUTF8Tester.getLength();
            rtl::OString aContext( helper::abbreviate( pTestee->GetText(), nPos, 20 ) );
            PrintError(rtl::OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Found double questionmark in translation only. Looks like an encoding problem at Position "))
                 .append(nErrorPos).getStr(),
                "Text format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId());
            pTestee->NotOK();
            return sal_True;
        }

    return sal_False;
}


/*****************************************************************************/
sal_Bool GSIBlock::CheckSyntax( std::size_t nLine, sal_Bool bRequireSourceLine, sal_Bool bFixTags )
/*****************************************************************************/
{
    static LingTest aTester;
    sal_Bool bHasError = sal_False;

    if ( !pSourceLine )
    {
        if ( bRequireSourceLine )
        {
            PrintError( "No source language entry defined!", "File format", "", nLine );
            bHasBlockError = sal_True;
        }
    }
    else
    {
        aTester.CheckReference( pSourceLine );
        if ( pSourceLine->HasMessages() )
        {
            PrintList( pSourceLine->GetMessageList(), "ReferenceString", pSourceLine );
            pSourceLine->NotOK();
            bHasError = sal_True;
        }
    }
    if ( bReference )
    {
        if ( !pReferenceLine )
        {
            GSILine *pSource;
            if ( pSourceLine )
                pSource = pSourceLine;
            else
                pSource = maList.empty() ? NULL : maList[ 0 ];   // get some other line
            if ( pSource )
                PrintError( "No reference line found. Entry is new in source file", "File format", "", pSource->GetLineNumber(), pSource->GetUniqId() );
            else
                PrintError( "No reference line found. Entry is new in source file", "File format", "", nLine );
            bHasBlockError = sal_True;
        }
        else
        {
            if ( pSourceLine && pSourceLine->data_ != pReferenceLine->data_ )
            {
                sal_Int32 nPos = pSourceLine->data_.indexOf( pReferenceLine->data_ );
                rtl::OStringBuffer aContext( pReferenceLine->data_.copy( nPos - 5, 15) );
                aContext.append( "\" --> \"" ).append( pSourceLine->data_.copy( nPos - 5, 15) );
                PrintError( "Source Language Entry has changed.", "File format", aContext.makeStringAndClear(), pSourceLine->GetLineNumber(), pSourceLine->GetUniqId() );
                pSourceLine->NotOK();
                bHasError = sal_True;
            }
        }
    }

    if ( pSourceLine )
        bHasError |= !TestUTF8( pSourceLine, bFixTags );

    for ( size_t i = 0, n = maList.size(); i < n; ++i )
    {
        GSILine* pItem = maList[ i ];
        aTester.CheckTestee( pItem, pSourceLine != NULL, bFixTags );
        if ( pItem->HasMessages() || aTester.HasCompareWarnings() )
        {
            if ( pItem->HasMessages() || aTester.GetCompareWarnings().HasErrors() )
                pItem->NotOK();
            bHasError = sal_True;
            PrintList( pItem->GetMessageList(), "Translation", pItem );
            PrintList( &(aTester.GetCompareWarnings()), "Translation Tag Mismatch", pItem );
        }
        bHasError |= !TestUTF8( pItem, bFixTags );
        if ( pSourceLine )
            bHasError |= HasSuspiciousChars( pItem, pSourceLine );
    }

    return bHasError || bHasBlockError;
}

void GSIBlock::WriteError( LazyStream &aErrOut, sal_Bool bRequireSourceLine  )
{
    if ( pSourceLine && pSourceLine->IsOK() && bCheckSourceLang && !bHasBlockError )
        return;

    sal_Bool bHasError = sal_False;
    sal_Bool bCopyAll = ( !pSourceLine && bRequireSourceLine ) || ( pSourceLine && !pSourceLine->IsOK() && !bCheckTranslationLang ) || bHasBlockError;
    for ( size_t i = 0, n = maList.size(); i < n; ++i )
    {
        GSILine* pItem = maList[ i ];
        if ( !pItem->IsOK() || bCopyAll )
        {
            bHasError = sal_True;
            aErrOut.LazyOpen();
            aErrOut << pItem->data_.getStr() << '\n';
        }
    }

    if ( pSourceLine && ( bHasError || !pSourceLine->IsOK() ) && !( !bHasError && bCheckTranslationLang ) )
    {
        aErrOut.LazyOpen();
        aErrOut << pSourceLine->data_.getStr() << '\n';
    }
}

void GSIBlock::WriteCorrect( LazyStream &aOkOut, sal_Bool bRequireSourceLine )
{
    if ( ( !pSourceLine && bRequireSourceLine ) || ( pSourceLine && !pSourceLine->IsOK() && !bCheckTranslationLang ) )
        return;

    sal_Bool bHasOK = sal_False;
    for ( size_t i = 0, n = maList.size(); i < n; ++i )
    {
        GSILine* pItem = maList[ i ];
        if ( ( pItem->IsOK() || bCheckSourceLang ) && !bHasBlockError )
        {
            bHasOK = sal_True;
            aOkOut.LazyOpen();
            aOkOut << pItem->data_.getStr() << '\n';
        }
    }

    if ( ( pSourceLine && pSourceLine->IsOK() && ( !maList.empty() || !bCheckTranslationLang ) ) || ( bHasOK && bCheckTranslationLang ) )
    {
        aOkOut.LazyOpen();
        aOkOut << pSourceLine->data_.getStr() << '\n';
    }
}

void GSIBlock::WriteFixed( LazyStream &aFixOut )
{
    if ( pSourceLine && !pSourceLine->IsFixed() && bCheckSourceLang )
        return;

    sal_Bool bHasFixes = sal_False;
    for ( size_t i = 0, n = maList.size(); i < n; ++i )
    {
        GSILine* pItem = maList[ i ];
        if ( pItem->IsFixed() )
        {
            bHasFixes = sal_True;
            aFixOut.LazyOpen();
            aFixOut << pItem->data_.getStr() << '\n';
        }
    }

    if ( pSourceLine && ( bHasFixes || pSourceLine->IsFixed() ) )
    {
        aFixOut.LazyOpen();
        aFixOut << pSourceLine->data_.getStr() << '\n';
    }
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsicheck checks the syntax of tags in SDF-Files\n" );
    fprintf( stdout, "         checks for inconsistencies and malicious UTF8 encoding\n" );
    fprintf( stdout, "         checks tags in Online Help\n" );
    fprintf( stdout, "         relax GID/LID length to %s\n",
        rtl::OString::valueOf(static_cast<sal_Int32>(MAX_GID_LID_LEN)).getStr() );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: gsicheck [ -c ] [-f] [ -we ] [ -wef ErrorFilename ] [ -wc ]\n" );
    fprintf( stdout, "                 [ -wcf CorrectFilename ] [ -s | -t ] [ -l LanguageID ]\n" );
    fprintf( stdout, "                 [ -r ReferenceFile ] filename\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "-c    Add context to error message (Print the line containing the error)\n" );
    fprintf( stdout, "-f    try to fix errors. See also -wf -wff \n" );
    fprintf( stdout, "-wf   Write File containing all fixed parts\n" );
    fprintf( stdout, "-wff  Same as above but give own filename\n" );
    fprintf( stdout, "-we   Write File containing all errors\n" );
    fprintf( stdout, "-wef  Same as above but give own filename\n" );
    fprintf( stdout, "-wc   Write File containing all correct parts\n" );
    fprintf( stdout, "-wcf  Same as above but give own filename\n" );
    fprintf( stdout, "-s    Check only source language. Should be used before handing out to vendor.\n" );
    fprintf( stdout, "-t    Check only Translation language(s). Should be used before merging.\n" );
    fprintf( stdout, "-e    disable encoding checks. E.g.: double questionmark \'??\' which may be the\n" );
    fprintf( stdout, "      result of false conversions\n" );
    fprintf( stdout, "-l    ISO language code of the source language.\n" );
    fprintf( stdout, "      Default is en-US. Use \"\" (empty string) or 'none'\n" );
    fprintf( stdout, "      to disable source language dependent checks\n" );
    fprintf( stdout, "-r    Reference filename to check that source language entries\n" );
    fprintf( stdout, "      have not been changed\n" );
       fprintf( stdout, "\n" );
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    sal_Bool bError = sal_False;
    sal_Bool bPrintContext = sal_False;
    sal_Bool bCheckSourceLang = sal_False;
    sal_Bool bCheckTranslationLang = sal_False;
    sal_Bool bWriteError = sal_False;
    sal_Bool bWriteCorrect = sal_False;
    sal_Bool bWriteFixed = sal_False;
    sal_Bool bFixTags = sal_False;
    sal_Bool bAllowSuspicious = sal_False;
    rtl::OString aErrorFilename;
    rtl::OString aCorrectFilename;
    rtl::OString aFixedFilename;
    sal_Bool bFileHasError = sal_False;
    rtl::OString aSourceLang( "en-US" );     // English is default
    rtl::OString aFilename;
    rtl::OString aReferenceFilename;
    sal_Bool bReferenceFile = sal_False;
    for ( int i = 1 ; i < argc ; i++ )
    {
        if ( *argv[ i ] == '-' )
        {
            switch (*(argv[ i ]+1))
            {
                case 'c':bPrintContext = sal_True;
                    break;
                case 'w':
                    {
                        if ( (*(argv[ i ]+2)) == 'e' )
                        {
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aErrorFilename = argv[i + 1];
                                    bWriteError = sal_True;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = sal_True;
                                }
                            else
                                   bWriteError = sal_True;
                        }
                        else if ( (*(argv[ i ]+2)) == 'c' )
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aCorrectFilename = argv[i + 1];
                                    bWriteCorrect = sal_True;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = sal_True;
                                }
                            else
                                   bWriteCorrect = sal_True;
                        else if ( (*(argv[ i ]+2)) == 'f' )
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aFixedFilename = argv[i + 1];
                                    bWriteFixed = sal_True;
                                    bFixTags = sal_True;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = sal_True;
                                }
                            else
                            {
                                   bWriteFixed = sal_True;
                                bFixTags = sal_True;
                            }
                        else
                        {
                            fprintf( stderr, "\nERROR: Unknown Switch %s!\n\n", argv[ i ] );
                            bError = sal_True;
                        }
                    }
                    break;
                case 's':bCheckSourceLang = sal_True;
                    break;
                case 't':bCheckTranslationLang = sal_True;
                    break;
                case 'l':
                    {
                        if ( (i+1) < argc )
                        {
                            aSourceLang = argv[ i+1 ];
                            if ( aSourceLang.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("none")) )
                                aSourceLang = rtl::OString();
                            i++;
                        }
                        else
                        {
                            fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                            bError = sal_True;
                        }
                    }
                    break;
                case 'r':
                    {
                        if ( (i+1) < argc )
                        {
                            aReferenceFilename = argv[ i+1 ];
                            bReferenceFile = sal_True;
                            i++;
                        }
                        else
                        {
                            fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                            bError = sal_True;
                        }
                    }
                    break;
                case 'f':
                    {
                        bFixTags = sal_True;
                    }
                    break;
                case 'e':
                    {
                        bAllowSuspicious = sal_True;
                    }
                    break;
                default:
                    fprintf( stderr, "\nERROR: Unknown Switch %s!\n\n", argv[ i ] );
                    bError = sal_True;
            }
        }
        else
        {
            if  (aFilename.isEmpty())
                aFilename = argv[i];
            else
            {
                fprintf( stderr, "\nERROR: Only one filename may be specified!\n\n");
                bError = sal_True;
            }
        }
    }


    if (aFilename.isEmpty() || bError)
    {
        Help();
        exit ( 0 );
    }

    if ( !aSourceLang.isEmpty() && !LanguageOK( aSourceLang ) )
    {
        fprintf( stderr, "\nERROR: The Language '%s' is invalid!\n\n", aSourceLang.getStr() );
        Help();
        exit ( 1 );
    }

    if ( bCheckSourceLang && bCheckTranslationLang )
    {
        fprintf( stderr, "\nERROR: The Options -s and -t are mutually exclusive.\nUse only one of them.\n\n" );
        Help();
        exit ( 1 );
    }



    std::ifstream aGSI(aFilename.getStr());
    if (!aGSI.is_open()) {
        fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", aFilename.getStr() );
        exit ( 3 );
    }

    std::ifstream aReferenceGSI;
    if ( bReferenceFile )
    {
        aReferenceGSI.open(aReferenceFilename.getStr());
        if (!aReferenceGSI.is_open()) {
            fprintf( stderr, "\nERROR: Could not open Input-File %s!\n\n", aFilename.getStr() );
            exit ( 3 );
        }
    }

    LazyStream aOkOut;
    if ( bWriteCorrect )
    {
        if (aCorrectFilename.isEmpty())
        {
            aCorrectFilename = addSuffix(
                aFilename, rtl::OString(RTL_CONSTASCII_STRINGPARAM("_ok")));
        }
        aOkOut.SetFileName(aCorrectFilename);
    }

    LazyStream aErrOut;
    if ( bWriteError )
    {
        if (aErrorFilename.isEmpty())
        {
            aErrorFilename = addSuffix(
                aFilename, rtl::OString(RTL_CONSTASCII_STRINGPARAM("_err")));
        }
        aErrOut.SetFileName(aErrorFilename);
    }

    LazyStream aFixOut;
    if ( bWriteFixed )
    {
        if (aFixedFilename.isEmpty())
        {
            aFixedFilename = addSuffix(
                aFilename, rtl::OString(RTL_CONSTASCII_STRINGPARAM("_fix")));
        }
        aFixOut.SetFileName(aFixedFilename);
    }


    GSILine* pReferenceLine = NULL;
    std::size_t nReferenceLine = 0;

    GSILine* pGSILine = NULL;
    rtl::OString aOldId("No Valid ID");   // just set to something which can never be an ID
    GSIBlock *pBlock = NULL;
    std::size_t nLine = 0;

    while (!aGSI.eof())
    {
        std::string s;
        std::getline(aGSI, s);
        nLine++;
        pGSILine = new GSILine(rtl::OString(s.data(), s.length()), nLine );
        sal_Bool bDelete = sal_True;


        if ( !pGSILine->data_.isEmpty() )
        {
            if ( FORMAT_UNKNOWN == pGSILine->GetLineFormat() )
            {
                PrintError( "Format of line is unknown. Ignoring!", "Line format", pGSILine->data_.copy( 0,40 ), bPrintContext, pGSILine->GetLineNumber() );
                pGSILine->NotOK();
                if ( bWriteError )
                {
                    bFileHasError = sal_True;
                    aErrOut.LazyOpen();
                    aErrOut << pGSILine->data_.getStr();
                }
            }
            else if ( pGSILine->GetLineType().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("res-comment")) )
            {   // ignore comment lines, but write them to Correct Items File
                if ( bWriteCorrect )
                {
                    aOkOut.LazyOpen();
                    aOkOut << pGSILine->data_.getStr() << '\n';
                }
            }
            else
            {
                rtl::OString aId = pGSILine->GetUniqId();
                if ( aId != aOldId )
                {
                    if ( pBlock )
                    {
                        bFileHasError |= pBlock->CheckSyntax( nLine, !aSourceLang.isEmpty(), bFixTags );

                        if ( bWriteError )
                            pBlock->WriteError( aErrOut, !aSourceLang.isEmpty() );
                        if ( bWriteCorrect )
                            pBlock->WriteCorrect( aOkOut, !aSourceLang.isEmpty() );
                        if ( bWriteFixed )
                            pBlock->WriteFixed( aFixOut );

                        delete pBlock;
                    }
                    pBlock = new GSIBlock( bPrintContext, bCheckSourceLang, bCheckTranslationLang, bReferenceFile, bAllowSuspicious );

                    aOldId = aId;


                    // find corresponding line in reference file
                    if ( bReferenceFile )
                    {
                        sal_Bool bContinueSearching = sal_True;
                        while ( ( !aReferenceGSI.eof() || pReferenceLine ) && bContinueSearching )
                        {
                            if ( !pReferenceLine )
                            {
                                std::string s2;
                                std::getline(aReferenceGSI, s2);
                                nReferenceLine++;
                                pReferenceLine = new GSILine(
                                    rtl::OString(s2.data(), s2.length()),
                                    nReferenceLine);
                            }
                            if ( pReferenceLine->GetLineFormat() != FORMAT_UNKNOWN )
                            {
                                if ( pReferenceLine->GetUniqId() == aId && pReferenceLine->GetLanguageId() == aSourceLang )
                                {
                                    pBlock->SetReferenceLine( pReferenceLine );
                                    pReferenceLine = NULL;
                                }
                                else if ( pReferenceLine->GetUniqId() > aId )
                                {
                                    bContinueSearching = sal_False;
                                }
                                else
                                {
                                    if ( pReferenceLine->GetUniqId() < aId  && pReferenceLine->GetLanguageId() == aSourceLang )
                                        PrintError( "No Entry in source file found. Entry has been removed from source file", "File format", "", bPrintContext, pGSILine->GetLineNumber(), pReferenceLine->GetUniqId() );
                                    delete pReferenceLine;
                                    pReferenceLine = NULL;
                                }
                            }
                            else
                            {
                                delete pReferenceLine;
                                pReferenceLine = NULL;
                            }

                        }
                    }

                }

                pBlock->InsertLine( pGSILine, aSourceLang );
                bDelete = sal_False;
            }
        }
        if ( bDelete )
            delete pGSILine;

    }
    if ( pBlock )
    {
        bFileHasError |= pBlock->CheckSyntax( nLine, !aSourceLang.isEmpty(), bFixTags );

        if ( bWriteError )
            pBlock->WriteError( aErrOut, !aSourceLang.isEmpty() );
        if ( bWriteCorrect )
            pBlock->WriteCorrect( aOkOut, !aSourceLang.isEmpty() );
        if ( bWriteFixed )
            pBlock->WriteFixed( aFixOut );

        delete pBlock;
    }
    aGSI.close();

    if ( bWriteError )
        aErrOut.close();
    if ( bWriteCorrect )
        aOkOut.close();
    if ( bWriteFixed )
        aFixOut.close();

    if ( bFileHasError )
        return 55;
    else
        return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
