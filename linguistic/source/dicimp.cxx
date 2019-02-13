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


#include <cppuhelper/factory.hxx>
#include "dicimp.hxx"
#include "hyphdsp.hxx"
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/linguistic2/DictionaryType.hpp>
#include <com/sun/star/linguistic2/DictionaryEventFlags.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include "defs.hxx"

#include <algorithm>


using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


#define BUFSIZE             4096
#define VERS2_NOLANGUAGE    1024

#define MAX_HEADER_LENGTH 16

// XML-header to query SPELLML support
// to handle user words with "Grammar By" model words
#define SPELLML_SUPPORT "<?xml?>"

// User dictionaries can contain optional "title:" tags
// to support custom titles with space and other characters.
// (old mechanism stores the title of the user dictionary
// only in its file name, but special characters are
// problem for user dictionaries shipped with LibreOffice).
//
// The following fake file name extension will be
// added to the text of the title: field for correct
// text stripping and dictionary saving.
#define EXTENSION_FOR_TITLE_TEXT "."

static const sal_Char* const pVerStr2    = "WBSWG2";
static const sal_Char* const pVerStr5    = "WBSWG5";
static const sal_Char* const pVerStr6    = "WBSWG6";
static const sal_Char* const pVerOOo7    = "OOoUserDict1";

static const sal_Int16 DIC_VERSION_DONTKNOW = -1;
static const sal_Int16 DIC_VERSION_2 = 2;
static const sal_Int16 DIC_VERSION_5 = 5;
static const sal_Int16 DIC_VERSION_6 = 6;
static const sal_Int16 DIC_VERSION_7 = 7;

static uno::Reference< XLinguServiceManager2 > GetLngSvcMgr_Impl()
{
    uno::Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
    uno::Reference< XLinguServiceManager2 > xRes = LinguServiceManager::create( xContext ) ;
    return xRes;
}

static bool getTag(const OString &rLine, const sal_Char *pTagName,
    OString &rTagValue)
{
    sal_Int32 nPos = rLine.indexOf(pTagName);
    if (nPos == -1)
        return false;

    rTagValue = comphelper::string::strip(rLine.copy(nPos + rtl_str_getLength(pTagName)),
        ' ');
    return true;
}


sal_Int16 ReadDicVersion( SvStreamPtr const &rpStream, LanguageType &nLng, bool &bNeg, OUString &aDicName )
{
    // Sniff the header
    sal_Int16 nDicVersion = DIC_VERSION_DONTKNOW;
    sal_Char pMagicHeader[MAX_HEADER_LENGTH];

    nLng = LANGUAGE_NONE;
    bNeg = false;

    if (!rpStream.get() || rpStream->GetError())
        return -1;

    sal_uInt64 const nSniffPos = rpStream->Tell();
    static std::size_t nVerOOo7Len = sal::static_int_cast< std::size_t >(strlen( pVerOOo7 ));
    pMagicHeader[ nVerOOo7Len ] = '\0';
    if ((rpStream->ReadBytes(static_cast<void *>(pMagicHeader), nVerOOo7Len) == nVerOOo7Len) &&
        !strcmp(pMagicHeader, pVerOOo7))
    {
        bool bSuccess;
        OString aLine;

        nDicVersion = DIC_VERSION_7;

        // 1st skip magic / header line
        rpStream->ReadLine(aLine);

        // 2nd line: language all | en-US | pt-BR ...
        while ((bSuccess = rpStream->ReadLine(aLine)))
        {
            OString aTagValue;

            if (aLine[0] == '#') // skip comments
                continue;

            // lang: field
            if (getTag(aLine, "lang: ", aTagValue))
            {
                if (aTagValue == "<none>")
                    nLng = LANGUAGE_NONE;
                else
                    nLng = LanguageTag::convertToLanguageType(
                            OStringToOUString( aTagValue, RTL_TEXTENCODING_ASCII_US));
            }

            // type: negative / positive
            if (getTag(aLine, "type: ", aTagValue))
            {
                bNeg = aTagValue == "negative";
            }

            // lang: title
            if (getTag(aLine, "title: ", aTagValue))
            {
                aDicName = OStringToOUString( aTagValue, RTL_TEXTENCODING_UTF8) +
                    // recent title text preparation in GetDicInfoStr() waits for an
                    // extension, so we add it to avoid bad stripping at final dot
                    // of the title text
                    EXTENSION_FOR_TITLE_TEXT;
            }

            if (aLine.indexOf("---") != -1) // end of header
                break;
        }
        if (!bSuccess)
            return -2;
    }
    else
    {
        sal_uInt16 nLen;

        rpStream->Seek (nSniffPos );

        rpStream->ReadUInt16( nLen );
        if (nLen >= MAX_HEADER_LENGTH)
            return -1;

        rpStream->ReadBytes(pMagicHeader, nLen);
        pMagicHeader[nLen] = '\0';

        // Check version magic
        if (0 == strcmp( pMagicHeader, pVerStr6 ))
            nDicVersion = DIC_VERSION_6;
        else if (0 == strcmp( pMagicHeader, pVerStr5 ))
            nDicVersion = DIC_VERSION_5;
        else if (0 == strcmp( pMagicHeader, pVerStr2 ))
            nDicVersion = DIC_VERSION_2;
        else
            nDicVersion = DIC_VERSION_DONTKNOW;

        if (DIC_VERSION_2 == nDicVersion ||
            DIC_VERSION_5 == nDicVersion ||
            DIC_VERSION_6 == nDicVersion)
        {
            // The language of the dictionary
            sal_uInt16 nTmp = 0;
            rpStream->ReadUInt16( nTmp );
            nLng = LanguageType(nTmp);
            if (VERS2_NOLANGUAGE == static_cast<sal_uInt16>(nLng))
                nLng = LANGUAGE_NONE;

            // Negative Flag
            rpStream->ReadCharAsBool( bNeg );
        }
    }

    return nDicVersion;
}

DictionaryNeo::DictionaryNeo(const OUString &rName,
                             LanguageType nLang, DictionaryType eType,
                             const OUString &rMainURL,
                             bool bWriteable) :
    aDicEvtListeners( GetLinguMutex() ),
    aDicName        (rName),
    aMainURL        (rMainURL),
    eDicType        (eType),
    nLanguage       (nLang)
{
    nDicVersion  = DIC_VERSION_DONTKNOW;
    bNeedEntries = true;
    bIsModified  = bIsActive = false;
    bIsReadonly = !bWriteable;

    if( !rMainURL.isEmpty())
    {
        bool bExists = FileExists( rMainURL );
        if( !bExists )
        {
            // save new dictionaries with in Format 7 (UTF8 plain text)
            nDicVersion  = DIC_VERSION_7;

            //! create physical representation of an **empty** dictionary
            //! that could be found by the dictionary-list implementation
            // (Note: empty dictionaries are not just empty files!)
            DBG_ASSERT( !bIsReadonly,
                    "DictionaryNeo: dictionaries should be writeable if they are to be saved" );
            if (!bIsReadonly)
                saveEntries( rMainURL );
            bNeedEntries = false;
        }
    }
    else
    {
        // non persistent dictionaries (like IgnoreAllList) should always be writable
        bIsReadonly  = false;
        bNeedEntries = false;
    }
}

DictionaryNeo::~DictionaryNeo()
{
}

ErrCode DictionaryNeo::loadEntries(const OUString &rMainURL)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // counter check that it is safe to set bIsModified to sal_False at
    // the end of the function
    DBG_ASSERT(!bIsModified, "lng : dictionary already modified!");

    // function should only be called once in order to load entries from file
    bNeedEntries = false;

    if (rMainURL.isEmpty())
        return ERRCODE_NONE;

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // get XInputStream stream
    uno::Reference< io::XInputStream > xStream;
    try
    {
        uno::Reference< ucb::XSimpleFileAccess3 > xAccess( ucb::SimpleFileAccess::create(xContext) );
        xStream = xAccess->openFileRead( rMainURL );
    }
    catch (const uno::Exception &)
    {
        SAL_WARN( "linguistic", "failed to get input stream" );
    }
    if (!xStream.is())
        return ErrCode(sal_uInt32(-1));

    SvStreamPtr pStream = SvStreamPtr( utl::UcbStreamHelper::CreateStream( xStream ) );

    // read header
    bool bNegativ;
    LanguageType nLang;
    nDicVersion = ReadDicVersion(pStream, nLang, bNegativ, aDicName);
    ErrCode nErr = pStream->GetError();
    if (nErr != ERRCODE_NONE)
        return nErr;

    nLanguage = nLang;

    eDicType = bNegativ ? DictionaryType_NEGATIVE : DictionaryType_POSITIVE;

    rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
    if (nDicVersion >= DIC_VERSION_6)
        eEnc = RTL_TEXTENCODING_UTF8;
    aEntries.clear();

    if (DIC_VERSION_6 == nDicVersion ||
        DIC_VERSION_5 == nDicVersion ||
        DIC_VERSION_2 == nDicVersion)
    {
        sal_uInt16  nLen = 0;
        sal_Char aWordBuf[ BUFSIZE ];

        // Read the first word
        if (!pStream->eof())
        {
            pStream->ReadUInt16( nLen );
            if (ERRCODE_NONE != (nErr = pStream->GetError()))
                return nErr;
            if ( nLen < BUFSIZE )
            {
                pStream->ReadBytes(aWordBuf, nLen);
                if (ERRCODE_NONE != (nErr = pStream->GetError()))
                    return nErr;
                *(aWordBuf + nLen) = 0;
            }
            else
                return SVSTREAM_READ_ERROR;
        }

        while(!pStream->eof())
        {
            // Read from file
            // Paste in dictionary without converting
            if(*aWordBuf)
            {
                OUString aText(aWordBuf, rtl_str_getLength(aWordBuf), eEnc);
                uno::Reference< XDictionaryEntry > xEntry =
                        new DicEntry( aText, bNegativ );
                addEntry_Impl( xEntry, true ); //! don't launch events here
            }

            pStream->ReadUInt16( nLen );
            if (pStream->eof())
                break;
            if (ERRCODE_NONE != (nErr = pStream->GetError()))
                return nErr;

            if (nLen < BUFSIZE)
            {
                pStream->ReadBytes(aWordBuf, nLen);
                if (ERRCODE_NONE != (nErr = pStream->GetError()))
                    return nErr;
            }
            else
                return SVSTREAM_READ_ERROR;
            *(aWordBuf + nLen) = 0;
        }
    }
    else if (DIC_VERSION_7 == nDicVersion)
    {
        OString aLine;

        // remaining lines - stock strings (a [==] b)
        while (pStream->ReadLine(aLine))
        {
            if (aLine.isEmpty() || aLine[0] == '#') // skip comments
                continue;
            OUString aText = OStringToOUString(aLine, RTL_TEXTENCODING_UTF8);
            uno::Reference< XDictionaryEntry > xEntry =
                    new DicEntry( aText, eDicType == DictionaryType_NEGATIVE );
            addEntry_Impl( xEntry, true ); //! don't launch events here
        }
    }

    SAL_WARN_IF(!isSorted(), "linguistic", "dictionary is not sorted");

    // since this routine should be called only initially (prior to any
    // modification to be saved) we reset the bIsModified flag here that
    // was implicitly set by addEntry_Impl
    bIsModified = false;

    return pStream->GetError();
}

static OString formatForSave(const uno::Reference< XDictionaryEntry > &xEntry,
    rtl_TextEncoding eEnc )
{
   OStringBuffer aStr(OUStringToOString(xEntry->getDictionaryWord(), eEnc));

   if (xEntry->isNegative() || !xEntry->getReplacementText().isEmpty())
   {
       aStr.append("==");
       aStr.append(OUStringToOString(xEntry->getReplacementText(), eEnc));
   }
   return aStr.makeStringAndClear();
}

ErrCode DictionaryNeo::saveEntries(const OUString &rURL)
{
    MutexGuard aGuard( GetLinguMutex() );

    if (rURL.isEmpty())
        return ERRCODE_NONE;
    DBG_ASSERT(!INetURLObject( rURL ).HasError(), "lng : invalid URL");

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // get XOutputStream stream
    uno::Reference<io::XStream> xStream;
    try
    {
        xStream = io::TempFile::create(xContext);
    }
    catch (const uno::Exception &)
    {
        DBG_ASSERT( false, "failed to get input stream" );
    }
    if (!xStream.is())
        return ErrCode(sal_uInt32(-1));

    SvStreamPtr pStream = SvStreamPtr( utl::UcbStreamHelper::CreateStream( xStream ) );

    // Always write as the latest version, i.e. DIC_VERSION_7

    rtl_TextEncoding eEnc = RTL_TEXTENCODING_UTF8;
    pStream->WriteLine(pVerOOo7);
    ErrCode nErr = pStream->GetError();
    if (nErr != ERRCODE_NONE)
        return nErr;
    /* XXX: the <none> case could be differentiated, is it absence or
     * undetermined or multiple? Earlier versions did not know about 'und' and
     * 'mul' and 'zxx' codes. Sync with ReadDicVersion() */
    if (LinguIsUnspecified(nLanguage))
        pStream->WriteLine("lang: <none>");
    else
    {
        OStringBuffer aLine("lang: ");
        aLine.append(OUStringToOString(LanguageTag::convertToBcp47(nLanguage), eEnc));
        pStream->WriteLine(aLine.makeStringAndClear());
    }
    if (ERRCODE_NONE != (nErr = pStream->GetError()))
        return nErr;
    if (eDicType == DictionaryType_POSITIVE)
        pStream->WriteLine("type: positive");
    else
        pStream->WriteLine("type: negative");
    if (aDicName.endsWith(EXTENSION_FOR_TITLE_TEXT))
    {
        pStream->WriteLine(OUStringToOString("title: " +
            // strip EXTENSION_FOR_TITLE_TEXT
            aDicName.copy(0, aDicName.lastIndexOf(EXTENSION_FOR_TITLE_TEXT)), eEnc));
    }
    if (ERRCODE_NONE != (nErr = pStream->GetError()))
        return nErr;
    pStream->WriteLine("---");
    if (ERRCODE_NONE != (nErr = pStream->GetError()))
        return nErr;
    for (Reference<XDictionaryEntry> & aEntrie : aEntries)
    {
        OString aOutStr = formatForSave(aEntrie, eEnc);
        pStream->WriteLine (aOutStr);
        if (ERRCODE_NONE != (nErr = pStream->GetError()))
            return nErr;
    }

    try
    {
        pStream.reset();
        uno::Reference< ucb::XSimpleFileAccess3 > xAccess(ucb::SimpleFileAccess::create(xContext));
        Reference<io::XInputStream> xInputStream(xStream, UNO_QUERY_THROW);
        uno::Reference<io::XSeekable> xSeek(xInputStream, UNO_QUERY_THROW);
        xSeek->seek(0);
        xAccess->writeFile(rURL, xInputStream);
        //If we are migrating from an older version, then on first successful
        //write, we're now converted to the latest version, i.e. DIC_VERSION_7
        nDicVersion = DIC_VERSION_7;
    }
    catch (const uno::Exception &)
    {
        DBG_ASSERT( false, "failed to write stream" );
        return ErrCode(sal_uInt32(-1));
    }

    return nErr;
}

void DictionaryNeo::launchEvent(sal_Int16 nEvent,
                                const uno::Reference< XDictionaryEntry >& xEntry)
{
    MutexGuard  aGuard( GetLinguMutex() );

    DictionaryEvent aEvt;
    aEvt.Source = uno::Reference< XDictionary >( this );
    aEvt.nEvent = nEvent;
    aEvt.xDictionaryEntry = xEntry;

    aDicEvtListeners.notifyEach( &XDictionaryEventListener::processDictionaryEvent, aEvt);
}

int DictionaryNeo::cmpDicEntry(const OUString& rWord1,
                               const OUString &rWord2,
                               bool bSimilarOnly)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // returns 0 if rWord1 is equal to rWord2
    //   "     a value < 0 if rWord1 is less than rWord2
    //   "     a value > 0 if rWord1 is greater than rWord2

    int nRes = 0;

    sal_Int32     nLen1 = rWord1.getLength(),
                  nLen2 = rWord2.getLength();
    if (bSimilarOnly)
    {
        const sal_Unicode cChar = '.';
        if (nLen1  &&  cChar == rWord1[ nLen1 - 1 ])
            nLen1--;
        if (nLen2  &&  cChar == rWord2[ nLen2 - 1 ])
            nLen2--;
    }

    const sal_Unicode cIgnChar = '=';
    const sal_Unicode cIgnBeg = '['; // for alternative hyphenation, eg. Schif[f]fahrt, Zuc[1k]ker
    const sal_Unicode cIgnEnd = ']'; // planned: gee"[1-/e]rfde or ge[-/1e]e"rfde (gee"rfde -> ge=erfde)
    sal_Int32       nIdx1 = 0,
                  nIdx2 = 0,
                  nNumIgnChar1 = 0,
                  nNumIgnChar2 = 0;

    bool IgnState;
    sal_Int32 nDiff = 0;
    sal_Unicode cChar1 = '\0';
    sal_Unicode cChar2 = '\0';
    do
    {
        // skip chars to be ignored
        IgnState = false;
        while (nIdx1 < nLen1  &&  ((cChar1 = rWord1[ nIdx1 ]) == cIgnChar || cChar1 == cIgnBeg || IgnState ))
        {
            if ( cChar1 == cIgnBeg )
                IgnState = true;
            else if (cChar1 == cIgnEnd)
                IgnState = false;
            nIdx1++;
            nNumIgnChar1++;
        }
        IgnState = false;
        while (nIdx2 < nLen2  &&  ((cChar2 = rWord2[ nIdx2 ]) == cIgnChar || cChar2 == cIgnBeg || IgnState ))
        {
            if ( cChar2 == cIgnBeg )
                IgnState = true;
            else if (cChar2 == cIgnEnd)
                IgnState = false;
            nIdx2++;
            nNumIgnChar2++;
        }

        if (nIdx1 < nLen1  &&  nIdx2 < nLen2)
        {
            nDiff = cChar1 - cChar2;
            if (nDiff)
                break;
            nIdx1++;
            nIdx2++;
        }
    } while (nIdx1 < nLen1  &&  nIdx2 < nLen2);


    if (nDiff)
        nRes = nDiff;
    else
    {   // the string with the smallest count of not ignored chars is the
        // shorter one

        // count remaining IgnChars
        IgnState = false;
        while (nIdx1 < nLen1 )
        {
            if (rWord1[ nIdx1 ] == cIgnBeg)
                IgnState = true;
            if (IgnState || rWord1[ nIdx1 ] == cIgnChar)
                nNumIgnChar1++;
            if (rWord1[ nIdx1] == cIgnEnd)
                IgnState = false;
            nIdx1++;
        }
        IgnState = false;
        while (nIdx2 < nLen2 )
        {
            if (rWord2[ nIdx2 ] == cIgnBeg)
                IgnState = true;
            if (IgnState || rWord2[ nIdx2 ] == cIgnChar)
                nNumIgnChar2++;
            if (rWord2[ nIdx2 ] == cIgnEnd)
                IgnState = false;
            nIdx2++;
        }

        nRes = (nLen1 - nNumIgnChar1) - (nLen2 - nNumIgnChar2);
    }

    return nRes;
}

bool DictionaryNeo::seekEntry(const OUString &rWord,
                              sal_Int32 *pPos, bool bSimilarOnly)
{
    // look for entry with binary search.
    // return sal_True if found sal_False else.
    // if pPos != NULL it will become the position of the found entry, or
    // if that was not found the position where it has to be inserted
    // to keep the entries sorted

    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nUpperIdx = getCount(),
          nMidIdx,
          nLowerIdx = 0;
    if( nUpperIdx > 0 )
    {
        nUpperIdx--;
        while( nLowerIdx <= nUpperIdx )
        {
            nMidIdx = (nLowerIdx + nUpperIdx) / 2;
            DBG_ASSERT(aEntries[nMidIdx].is(), "lng : empty entry encountered");

            int nCmp = - cmpDicEntry( aEntries[nMidIdx]->getDictionaryWord(),
                                      rWord, bSimilarOnly );
            if(nCmp == 0)
            {
                if( pPos ) *pPos = nMidIdx;
                return true;
            }
            else if(nCmp > 0)
                nLowerIdx = nMidIdx + 1;
            else if( nMidIdx == 0 )
            {
                if( pPos ) *pPos = nLowerIdx;
                return false;
            }
            else
                nUpperIdx = nMidIdx - 1;
        }
    }
    if( pPos ) *pPos = nLowerIdx;
    return false;
}

bool DictionaryNeo::isSorted()
{
    bool bRes = true;

    sal_Int32 nEntries = getCount();
    sal_Int32 i;
    for (i = 1;  i < nEntries;  i++)
    {
        if (cmpDicEntry( aEntries[i-1]->getDictionaryWord(),
                         aEntries[i]->getDictionaryWord() ) > 0)
        {
            bRes = false;
            break;
        }
    }
    return bRes;
}

bool DictionaryNeo::addEntry_Impl(const uno::Reference< XDictionaryEntry >& xDicEntry,
        bool bIsLoadEntries)
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;

    if ( bIsLoadEntries || (!bIsReadonly  &&  xDicEntry.is()) )
    {
        bool bIsNegEntry = xDicEntry->isNegative();
        bool bAddEntry   = !isFull() &&
                   (   ( eDicType == DictionaryType_POSITIVE && !bIsNegEntry )
                    || ( eDicType == DictionaryType_NEGATIVE &&  bIsNegEntry )
                    || ( eDicType == DictionaryType_MIXED ) );

        // look for position to insert entry at
        // if there is already an entry do not insert the new one
        sal_Int32 nPos = 0;
        if (bAddEntry)
        {
            const bool bFound = seekEntry( xDicEntry->getDictionaryWord(), &nPos );
            if (bFound)
                bAddEntry = false;
        }

        if (bAddEntry)
        {
            DBG_ASSERT(!bNeedEntries, "lng : entries still not loaded");

            // insert new entry at specified position
            aEntries.insert(aEntries.begin() + nPos, xDicEntry);
            SAL_WARN_IF(!isSorted(), "linguistic", "dictionary entries unsorted");

            bIsModified = true;
            bRes = true;

            if (!bIsLoadEntries)
                launchEvent( DictionaryEventFlags::ADD_ENTRY, xDicEntry );
        }
    }

    // add word to the Hunspell dictionary using a sample word for affixation/compounding
    if (xDicEntry.is() && !xDicEntry->isNegative() && !xDicEntry->getReplacementText().isEmpty()) {
        uno::Reference< XLinguServiceManager2 > xLngSvcMgr( GetLngSvcMgr_Impl() );
        uno::Reference< XSpellChecker1 > xSpell;
        Reference< XSpellAlternatives > xTmpRes;
        xSpell.set( xLngSvcMgr->getSpellChecker(), UNO_QUERY );
        Sequence< css::beans::PropertyValue > aEmptySeq;
        if (xSpell.is() && (xSpell->isValid( SPELLML_SUPPORT, static_cast<sal_uInt16>(nLanguage), aEmptySeq )))
        {
            // "Grammar By" sample word is a Hunspell dictionary word?
            if (xSpell->isValid( xDicEntry->getReplacementText(), static_cast<sal_uInt16>(nLanguage), aEmptySeq ))
            {
                xTmpRes = xSpell->spell( "<?xml?><query type='add'><word>" +
                    xDicEntry->getDictionaryWord() + "</word><word>" + xDicEntry->getReplacementText() +
                    "</word></query>", static_cast<sal_uInt16>(nLanguage), aEmptySeq );
                bRes = true;
            } else
                bRes = false;
        }
    }

    return bRes;
}

OUString SAL_CALL DictionaryNeo::getName(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aDicName;
}

void SAL_CALL DictionaryNeo::setName( const OUString& aName )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (aDicName != aName)
    {
        aDicName = aName;
        launchEvent(DictionaryEventFlags::CHG_NAME, nullptr);
    }
}

DictionaryType SAL_CALL DictionaryNeo::getDictionaryType(  )
{
    MutexGuard  aGuard( GetLinguMutex() );

    return eDicType;
}

void SAL_CALL DictionaryNeo::setActive( sal_Bool bActivate )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bIsActive != bool(bActivate))
    {
        bIsActive = bActivate;
        sal_Int16 nEvent = bIsActive ?
                DictionaryEventFlags::ACTIVATE_DIC : DictionaryEventFlags::DEACTIVATE_DIC;

        // remove entries from memory if dictionary is deactivated
        if (!bIsActive)
        {
            bool bIsEmpty = aEntries.empty();

            // save entries first if necessary
            if (bIsModified && hasLocation() && !isReadonly())
            {
                store();

                aEntries.clear();
                bNeedEntries = !bIsEmpty;
            }
            DBG_ASSERT( !bIsModified || !hasLocation() || isReadonly(),
                    "lng : dictionary is still modified" );
        }

        launchEvent(nEvent, nullptr);
    }
}

sal_Bool SAL_CALL DictionaryNeo::isActive(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return bIsActive;
}

sal_Int32 SAL_CALL DictionaryNeo::getCount(  )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );
    return static_cast<sal_Int32>(aEntries.size());
}

Locale SAL_CALL DictionaryNeo::getLocale(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return LanguageTag::convertToLocale( nLanguage );
}

void SAL_CALL DictionaryNeo::setLocale( const Locale& aLocale )
{
    MutexGuard  aGuard( GetLinguMutex() );
    LanguageType nLanguageP = LinguLocaleToLanguage( aLocale );
    if (!bIsReadonly  &&  nLanguage != nLanguageP)
    {
        nLanguage = nLanguageP;
        bIsModified = true; // new language needs to be saved with dictionary

        launchEvent( DictionaryEventFlags::CHG_LANGUAGE, nullptr );
    }
}

uno::Reference< XDictionaryEntry > SAL_CALL DictionaryNeo::getEntry(
            const OUString& aWord )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );

    sal_Int32 nPos;
    bool bFound = seekEntry( aWord, &nPos, true );
    DBG_ASSERT(!bFound || nPos < static_cast<sal_Int32>(aEntries.size()), "lng : index out of range");

    return bFound ? aEntries[ nPos ]
                    : uno::Reference< XDictionaryEntry >();
}

sal_Bool SAL_CALL DictionaryNeo::addEntry(
            const uno::Reference< XDictionaryEntry >& xDicEntry )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;

    if (!bIsReadonly)
    {
        if (bNeedEntries)
            loadEntries( aMainURL );
        bRes = addEntry_Impl( xDicEntry );
    }

    return bRes;
}

sal_Bool SAL_CALL
    DictionaryNeo::add( const OUString& rWord, sal_Bool bIsNegative,
            const OUString& rRplcText )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;

    if (!bIsReadonly)
    {
        uno::Reference< XDictionaryEntry > xEntry =
                new DicEntry( rWord, bIsNegative, rRplcText );
        bRes = addEntry_Impl( xEntry );
    }

    return bRes;
}

sal_Bool SAL_CALL DictionaryNeo::remove( const OUString& aWord )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRemoved = false;

    if (!bIsReadonly)
    {
        if (bNeedEntries)
            loadEntries( aMainURL );

        sal_Int32 nPos;
        bool bFound = seekEntry( aWord, &nPos );
        DBG_ASSERT(!bFound || nPos < static_cast<sal_Int32>(aEntries.size()), "lng : index out of range");

        // remove element if found
        if (bFound)
        {
            // entry to be removed
            uno::Reference< XDictionaryEntry >
                    xDicEntry( aEntries[ nPos ] );
            DBG_ASSERT(xDicEntry.is(), "lng : dictionary entry is NULL");

            aEntries.erase(aEntries.begin() + nPos);

            bRemoved = bIsModified = true;

            launchEvent( DictionaryEventFlags::DEL_ENTRY, xDicEntry );
        }
    }

    return bRemoved;
}

sal_Bool SAL_CALL DictionaryNeo::isFull(  )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );
    return aEntries.size() >= DIC_MAX_ENTRIES;
}

uno::Sequence< uno::Reference< XDictionaryEntry > >
    SAL_CALL DictionaryNeo::getEntries(  )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );
    return comphelper::containerToSequence(aEntries);
}


void SAL_CALL DictionaryNeo::clear(  )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bIsReadonly && !aEntries.empty())
    {
        // release all references to old entries
        aEntries.clear();

        bNeedEntries = false;
        bIsModified = true;

        launchEvent( DictionaryEventFlags::ENTRIES_CLEARED , nullptr );
    }
}

sal_Bool SAL_CALL DictionaryNeo::addDictionaryEventListener(
            const uno::Reference< XDictionaryEventListener >& xListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (xListener.is())
    {
        sal_Int32   nLen = aDicEvtListeners.getLength();
        bRes = aDicEvtListeners.addInterface( xListener ) != nLen;
    }
    return bRes;
}

sal_Bool SAL_CALL DictionaryNeo::removeDictionaryEventListener(
            const uno::Reference< XDictionaryEventListener >& xListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (xListener.is())
    {
        sal_Int32   nLen = aDicEvtListeners.getLength();
        bRes = aDicEvtListeners.removeInterface( xListener ) != nLen;
    }
    return bRes;
}


sal_Bool SAL_CALL DictionaryNeo::hasLocation()
{
    MutexGuard  aGuard( GetLinguMutex() );
    return !aMainURL.isEmpty();
}

OUString SAL_CALL DictionaryNeo::getLocation()
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aMainURL;
}

sal_Bool SAL_CALL DictionaryNeo::isReadonly()
{
    MutexGuard  aGuard( GetLinguMutex() );

    return bIsReadonly;
}

void SAL_CALL DictionaryNeo::store()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bIsModified && hasLocation() && !isReadonly())
    {
        if (!saveEntries( aMainURL ))
            bIsModified = false;
    }
}

void SAL_CALL DictionaryNeo::storeAsURL(
            const OUString& aURL,
            const uno::Sequence< beans::PropertyValue >& /*rArgs*/ )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!saveEntries( aURL ))
    {
        aMainURL = aURL;
        bIsModified = false;
        bIsReadonly = IsReadOnly( getLocation() );
    }
}

void SAL_CALL DictionaryNeo::storeToURL(
            const OUString& aURL,
            const uno::Sequence< beans::PropertyValue >& /*rArgs*/ )
{
    MutexGuard  aGuard( GetLinguMutex() );
    saveEntries(aURL);
}


DicEntry::DicEntry(const OUString &rDicFileWord,
                   bool bIsNegativWord)
{
    if (!rDicFileWord.isEmpty())
        splitDicFileWord( rDicFileWord, aDicWord, aReplacement );
    bIsNegativ = bIsNegativWord;
}

DicEntry::DicEntry(const OUString &rDicWord, bool bNegativ,
                   const OUString &rRplcText) :
    aDicWord                (rDicWord),
    aReplacement            (rRplcText),
    bIsNegativ              (bNegativ)
{
}

DicEntry::~DicEntry()
{
}

void DicEntry::splitDicFileWord(const OUString &rDicFileWord,
                                OUString &rDicWord,
                                OUString &rReplacement)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nDelimPos = rDicFileWord.indexOf( "==" );
    if (-1 != nDelimPos)
    {
        sal_Int32 nTriplePos = nDelimPos + 2;
        if (    nTriplePos < rDicFileWord.getLength()
            &&  rDicFileWord[ nTriplePos ] == '=' )
            ++nDelimPos;
        rDicWord     = rDicFileWord.copy( 0, nDelimPos );
        rReplacement = rDicFileWord.copy( nDelimPos + 2 );
    }
    else
    {
        rDicWord     = rDicFileWord;
        rReplacement.clear();
    }
}

OUString SAL_CALL DicEntry::getDictionaryWord(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aDicWord;
}

sal_Bool SAL_CALL DicEntry::isNegative(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return bIsNegativ;
}

OUString SAL_CALL DicEntry::getReplacementText(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aReplacement;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
