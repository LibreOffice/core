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
#include <dicimp.hxx>
#include <hyphdsp.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/linguistic2/DictionaryType.hpp>
#include <com/sun/star/linguistic2/DictionaryEventFlags.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include "defs.hxx"


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

static const sal_Char*      pVerStr2    = "WBSWG2";
static const sal_Char*      pVerStr5    = "WBSWG5";
static const sal_Char*      pVerStr6    = "WBSWG6";
static const sal_Char*      pVerOOo7    = "OOoUserDict1";

static const sal_Int16 DIC_VERSION_DONTKNOW = -1;
static const sal_Int16 DIC_VERSION_2 = 2;
static const sal_Int16 DIC_VERSION_5 = 5;
static const sal_Int16 DIC_VERSION_6 = 6;
static const sal_Int16 DIC_VERSION_7 = 7;

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


sal_Int16 ReadDicVersion( SvStreamPtr &rpStream, sal_uInt16 &nLng, sal_Bool &bNeg )
{
    // Sniff the header
    sal_Int16 nDicVersion = DIC_VERSION_DONTKNOW;
    sal_Char pMagicHeader[MAX_HEADER_LENGTH];

    nLng = LANGUAGE_NONE;
    bNeg = sal_False;

    if (!rpStream.get() || rpStream->GetError())
        return -1;

    sal_Size nSniffPos = rpStream->Tell();
    static sal_Size nVerOOo7Len = sal::static_int_cast< sal_Size >(strlen( pVerOOo7 ));
    pMagicHeader[ nVerOOo7Len ] = '\0';
    if ((rpStream->Read((void *) pMagicHeader, nVerOOo7Len) == nVerOOo7Len) &&
        !strcmp(pMagicHeader, pVerOOo7))
    {
        sal_Bool bSuccess;
        OString aLine;

        nDicVersion = DIC_VERSION_7;

        // 1st skip magic / header line
        rpStream->ReadLine(aLine);

        // 2nd line: language all | en-US | pt-BR ...
        while (sal_True == (bSuccess = rpStream->ReadLine(aLine)))
        {
            OString aTagValue;

            if (aLine[0] == '#') // skip comments
                continue;

            // lang: field
            if (getTag(aLine, "lang: ", aTagValue))
            {
                if (aTagValue.equalsL(RTL_CONSTASCII_STRINGPARAM("<none>")))
                    nLng = LANGUAGE_NONE;
                else
                    nLng = LanguageTag::convertToLanguageTypeWithFallback(
                            OStringToOUString( aTagValue, RTL_TEXTENCODING_ASCII_US));
            }

            // type: negative / positive
            if (getTag(aLine, "type: ", aTagValue))
            {
                if (aTagValue.equalsL(RTL_CONSTASCII_STRINGPARAM("negative")))
                    bNeg = sal_True;
                else
                    bNeg = sal_False;
            }

            if (aLine.indexOfL(RTL_CONSTASCII_STRINGPARAM("---")) != -1) // end of header
                break;
        }
        if (!bSuccess)
            return -2;
    }
    else
    {
        sal_uInt16 nLen;

        rpStream->Seek (nSniffPos );

        *rpStream >> nLen;
        if (nLen >= MAX_HEADER_LENGTH)
            return -1;

        rpStream->Read(pMagicHeader, nLen);
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
            *rpStream >> nLng;

            if (VERS2_NOLANGUAGE == nLng)
                nLng = LANGUAGE_NONE;

            // Negative Flag
            sal_Char nTmp;
            *rpStream >> nTmp;
            bNeg = (sal_Bool)nTmp;
        }
    }

    return nDicVersion;
}

DictionaryNeo::DictionaryNeo() :
    aDicEvtListeners( GetLinguMutex() ),
    eDicType        (DictionaryType_POSITIVE),
    nLanguage       (LANGUAGE_NONE)
{
    nCount       = 0;
    nDicVersion  = DIC_VERSION_DONTKNOW;
    bNeedEntries = sal_False;
    bIsModified  = bIsActive = sal_False;
    bIsReadonly  = sal_False;
}

DictionaryNeo::DictionaryNeo(const OUString &rName,
                             sal_Int16 nLang, DictionaryType eType,
                             const OUString &rMainURL,
                             sal_Bool bWriteable) :
    aDicEvtListeners( GetLinguMutex() ),
    aDicName        (rName),
    aMainURL        (rMainURL),
    eDicType        (eType),
    nLanguage       (nLang)
{
    nCount       = 0;
    nDicVersion  = DIC_VERSION_DONTKNOW;
    bNeedEntries = sal_True;
    bIsModified  = bIsActive = sal_False;
    bIsReadonly = !bWriteable;

    if( !rMainURL.isEmpty())
    {
        sal_Bool bExists = FileExists( rMainURL );
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
            bNeedEntries = sal_False;
        }
    }
    else
    {
        // non persistent dictionaries (like IgnoreAllList) should always be writable
        bIsReadonly  = sal_False;
        bNeedEntries = sal_False;
    }
}

DictionaryNeo::~DictionaryNeo()
{
}

sal_uLong DictionaryNeo::loadEntries(const OUString &rMainURL)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // counter check that it is safe to set bIsModified to sal_False at
    // the end of the function
    DBG_ASSERT(!bIsModified, "lng : dictionary already modified!");

    // function should only be called once in order to load entries from file
    bNeedEntries = sal_False;

    if (rMainURL.isEmpty())
        return 0;

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
        DBG_ASSERT( 0, "failed to get input stream" );
    }
    if (!xStream.is())
        return static_cast< sal_uLong >(-1);

    SvStreamPtr pStream = SvStreamPtr( utl::UcbStreamHelper::CreateStream( xStream ) );

    sal_uLong nErr = sal::static_int_cast< sal_uLong >(-1);

    // read header
    sal_Bool bNegativ;
    sal_uInt16 nLang;
    nDicVersion = ReadDicVersion(pStream, nLang, bNegativ);
    if (0 != (nErr = pStream->GetError()))
        return nErr;

    nLanguage = nLang;

    eDicType = bNegativ ? DictionaryType_NEGATIVE : DictionaryType_POSITIVE;

    rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
    if (nDicVersion >= DIC_VERSION_6)
        eEnc = RTL_TEXTENCODING_UTF8;
    nCount = 0;

    if (DIC_VERSION_6 == nDicVersion ||
        DIC_VERSION_5 == nDicVersion ||
        DIC_VERSION_2 == nDicVersion)
    {
        sal_uInt16  nLen = 0;
        sal_Char aWordBuf[ BUFSIZE ];

        // Read the first word
        if (!pStream->IsEof())
        {
            *pStream >> nLen;
            if (0 != (nErr = pStream->GetError()))
                return nErr;
            if ( nLen < BUFSIZE )
            {
                pStream->Read(aWordBuf, nLen);
                if (0 != (nErr = pStream->GetError()))
                    return nErr;
                *(aWordBuf + nLen) = 0;
            }
            else
                return SVSTREAM_READ_ERROR;
        }

        while(!pStream->IsEof())
        {
            // Read from file
            // Paste in dictionary without converting
            if(*aWordBuf)
            {
                OUString aText(aWordBuf, rtl_str_getLength(aWordBuf), eEnc);
                uno::Reference< XDictionaryEntry > xEntry =
                        new DicEntry( aText, bNegativ );
                addEntry_Impl( xEntry , sal_True ); //! don't launch events here
            }

            *pStream >> nLen;
            if (pStream->IsEof())
                break;
            if (0 != (nErr = pStream->GetError()))
                return nErr;

            if (nLen < BUFSIZE)
            {
                pStream->Read(aWordBuf, nLen);
                if (0 != (nErr = pStream->GetError()))
                    return nErr;
            }
            else
                return SVSTREAM_READ_ERROR;
            *(aWordBuf + nLen) = 0;
        }
    }
    else if (DIC_VERSION_7 == nDicVersion)
    {
        sal_Bool bSuccess;
        OString aLine;

        // remaining lines - stock strings (a [==] b)
        while (sal_True == (bSuccess = pStream->ReadLine(aLine)))
        {
            if (aLine[0] == '#') // skip comments
                continue;
            OUString aText = OStringToOUString(aLine, RTL_TEXTENCODING_UTF8);
            uno::Reference< XDictionaryEntry > xEntry =
                    new DicEntry( aText, eDicType == DictionaryType_NEGATIVE );
            addEntry_Impl( xEntry , sal_True ); //! don't launch events here
        }
    }

    SAL_WARN_IF(!isSorted(), "linguistic", "dictionary is not sorted");

    // since this routine should be called only initialy (prior to any
    // modification to be saved) we reset the bIsModified flag here that
    // was implicitly set by addEntry_Impl
    bIsModified = sal_False;

    return pStream->GetError();
}

static OString formatForSave(const uno::Reference< XDictionaryEntry > &xEntry,
    rtl_TextEncoding eEnc )
{
   OStringBuffer aStr(OUStringToOString(xEntry->getDictionaryWord(), eEnc));

   if (xEntry->isNegative())
   {
       aStr.append(RTL_CONSTASCII_STRINGPARAM("=="));
       aStr.append(OUStringToOString(xEntry->getReplacementText(), eEnc));
   }
   return aStr.makeStringAndClear();
}

struct TmpDictionary
{
    OUString maURL, maTmpURL;
    uno::Reference< ucb::XSimpleFileAccess3 > mxAccess;

    void cleanTmpFile()
    {
        try
        {
            if (mxAccess.is())
            {
                mxAccess->kill(maTmpURL);
            }
        }
        catch (const uno::Exception &) { }
    }
    TmpDictionary(const OUString &rURL)
        : maURL( rURL )
    {
        maTmpURL = maURL + ".tmp";
    }
    ~TmpDictionary()
    {
        cleanTmpFile();
    }

    uno::Reference< io::XStream > openTmpFile()
    {
        uno::Reference< io::XStream > xStream;

        try
        {
            mxAccess = ucb::SimpleFileAccess::create(
                        comphelper::getProcessComponentContext());
            xStream = mxAccess->openFileReadWrite(maTmpURL);
        } catch (const uno::Exception &) { }

        return xStream;
    }

    sal_uLong renameTmpToURL()
    {
        try
        {
            mxAccess->move(maTmpURL, maURL);
        }
        catch (const uno::Exception &)
        {
            DBG_ASSERT( 0, "failed to overwrite dict" );
            return static_cast< sal_uLong >(-1);
        }
        return 0;
    }
};

sal_uLong DictionaryNeo::saveEntries(const OUString &rURL)
{
    MutexGuard aGuard( GetLinguMutex() );

    if (rURL.isEmpty())
        return 0;
    DBG_ASSERT(!INetURLObject( rURL ).HasError(), "lng : invalid URL");

    // lifecycle manage the .tmp file
    TmpDictionary aTmpDictionary(rURL);
    uno::Reference< io::XStream > xStream = aTmpDictionary.openTmpFile();

    if (!xStream.is())
        return static_cast< sal_uLong >(-1);

    sal_uLong nErr = sal::static_int_cast< sal_uLong >(-1);
    SvStreamPtr pStream = SvStreamPtr( utl::UcbStreamHelper::CreateStream( xStream ) );

    //
    // Always write as the latest version, i.e. DIC_VERSION_7
    //
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_UTF8;
    pStream->WriteLine(OString(pVerOOo7));
    if (0 != (nErr = pStream->GetError()))
        return nErr;
    /* XXX: the <none> case could be differentiated, is it absence or
     * undetermined or multiple? Earlier versions did not know about 'und' and
     * 'mul' and 'zxx' codes. Sync with ReadDicVersion() */
    if (LinguIsUnspecified(nLanguage))
        pStream->WriteLine(OString(RTL_CONSTASCII_STRINGPARAM("lang: <none>")));
    else
    {
        OStringBuffer aLine(RTL_CONSTASCII_STRINGPARAM("lang: "));
        aLine.append(OUStringToOString(LanguageTag::convertToBcp47(nLanguage), eEnc));
        pStream->WriteLine(aLine.makeStringAndClear());
    }
    if (0 != (nErr = pStream->GetError()))
        return nErr;
    if (eDicType == DictionaryType_POSITIVE)
        pStream->WriteLine(OString(RTL_CONSTASCII_STRINGPARAM("type: positive")));
    else
        pStream->WriteLine(OString(RTL_CONSTASCII_STRINGPARAM("type: negative")));
    if (0 != (nErr = pStream->GetError()))
        return nErr;
    pStream->WriteLine(OString(RTL_CONSTASCII_STRINGPARAM("---")));
    if (0 != (nErr = pStream->GetError()))
        return nErr;
    const uno::Reference< XDictionaryEntry > *pEntry = aEntries.getConstArray();
    for (sal_Int32 i = 0;  i < nCount;  i++)
    {
        OString aOutStr = formatForSave(pEntry[i], eEnc);
        pStream->WriteLine (aOutStr);
        if (0 != (nErr = pStream->GetError()))
            break;
    }

    pStream.reset(); // fdo#66420 close streams so Win32 can move the file
    xStream.clear();
    nErr = aTmpDictionary.renameTmpToURL();

    //If we are migrating from an older version, then on first successful
    //write, we're now converted to the latest version, i.e. DIC_VERSION_7
    nDicVersion = DIC_VERSION_7;

    return nErr;
}

void DictionaryNeo::launchEvent(sal_Int16 nEvent,
                                uno::Reference< XDictionaryEntry > xEntry)
{
    MutexGuard  aGuard( GetLinguMutex() );

    DictionaryEvent aEvt;
    aEvt.Source = uno::Reference< XDictionary >( this );
    aEvt.nEvent = nEvent;
    aEvt.xDictionaryEntry = xEntry;

    cppu::OInterfaceIteratorHelper aIt( aDicEvtListeners );
    while (aIt.hasMoreElements())
    {
        uno::Reference< XDictionaryEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processDictionaryEvent( aEvt );
    }
}

int DictionaryNeo::cmpDicEntry(const OUString& rWord1,
                               const OUString &rWord2,
                               sal_Bool bSimilarOnly)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // returns 0 if rWord1 is equal to rWord2
    //   "     a value < 0 if rWord1 is less than rWord2
    //   "     a value > 0 if rWord1 is greater than rWord2

    int nRes = 0;

    OUString    aWord1( rWord1 ),
                aWord2( rWord2 );
    sal_Int32       nLen1 = aWord1.getLength(),
                  nLen2 = aWord2.getLength();
    if (bSimilarOnly)
    {
        const sal_Unicode cChar = '.';
        if (nLen1  &&  cChar == aWord1[ nLen1 - 1 ])
            nLen1--;
        if (nLen2  &&  cChar == aWord2[ nLen2 - 1 ])
            nLen2--;
    }

    const sal_Unicode cIgnChar = '=';
    sal_Int32       nIdx1 = 0,
                  nIdx2 = 0,
                  nNumIgnChar1 = 0,
                  nNumIgnChar2 = 0;

    sal_Int32 nDiff = 0;
    sal_Unicode cChar1 = '\0';
    sal_Unicode cChar2 = '\0';
    do
    {
        // skip chars to be ignored
        while (nIdx1 < nLen1  &&  (cChar1 = aWord1[ nIdx1 ]) == cIgnChar)
        {
            nIdx1++;
            nNumIgnChar1++;
        }
        while (nIdx2 < nLen2  &&  (cChar2 = aWord2[ nIdx2 ]) == cIgnChar)
        {
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
        while (nIdx1 < nLen1 )
        {
            if (aWord1[ nIdx1++ ] == cIgnChar)
                nNumIgnChar1++;
        }
        while (nIdx2 < nLen2 )
        {
            if (aWord2[ nIdx2++ ] == cIgnChar)
                nNumIgnChar2++;
        }

        nRes = ((sal_Int32) nLen1 - nNumIgnChar1) - ((sal_Int32) nLen2 - nNumIgnChar2);
    }

    return nRes;
}

sal_Bool DictionaryNeo::seekEntry(const OUString &rWord,
                              sal_Int32 *pPos, sal_Bool bSimilarOnly)
{
    // look for entry with binary search.
    // return sal_True if found sal_False else.
    // if pPos != NULL it will become the position of the found entry, or
    // if that was not found the position where it has to be inserted
    // to keep the entries sorted

    MutexGuard  aGuard( GetLinguMutex() );

    const uno::Reference< XDictionaryEntry > *pEntry = aEntries.getConstArray();
    sal_Int32 nUpperIdx = getCount(),
          nMidIdx,
          nLowerIdx = 0;
    if( nUpperIdx > 0 )
    {
        nUpperIdx--;
        while( nLowerIdx <= nUpperIdx )
        {
            nMidIdx = (nLowerIdx + nUpperIdx) / 2;
            DBG_ASSERT(pEntry[nMidIdx].is(), "lng : empty entry encountered");

            int nCmp = - cmpDicEntry( pEntry[nMidIdx]->getDictionaryWord(),
                                      rWord, bSimilarOnly );
            if(nCmp == 0)
            {
                if( pPos ) *pPos = nMidIdx;
                return sal_True;
            }
            else if(nCmp > 0)
                nLowerIdx = nMidIdx + 1;
            else if( nMidIdx == 0 )
            {
                if( pPos ) *pPos = nLowerIdx;
                return sal_False;
            }
            else
                nUpperIdx = nMidIdx - 1;
        }
    }
    if( pPos ) *pPos = nLowerIdx;
    return sal_False;
}

bool DictionaryNeo::isSorted()
{
    bool bRes = true;

    const uno::Reference< XDictionaryEntry > *pEntry = aEntries.getConstArray();
    sal_Int32 nEntries = getCount();
    sal_Int32 i;
    for (i = 1;  i < nEntries;  i++)
    {
        if (cmpDicEntry( pEntry[i-1]->getDictionaryWord(),
                         pEntry[i]->getDictionaryWord() ) > 0)
        {
            bRes = false;
            break;
        }
    }
    return bRes;
}

sal_Bool DictionaryNeo::addEntry_Impl(const uno::Reference< XDictionaryEntry > xDicEntry,
        sal_Bool bIsLoadEntries)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;

    if ( bIsLoadEntries || (!bIsReadonly  &&  xDicEntry.is()) )
    {
        sal_Bool bIsNegEntry = xDicEntry->isNegative();
        sal_Bool bAddEntry   = !isFull() &&
                   (   ( eDicType == DictionaryType_POSITIVE && !bIsNegEntry )
                    || ( eDicType == DictionaryType_NEGATIVE &&  bIsNegEntry )
                    || ( eDicType == DictionaryType_MIXED ) );

        // look for position to insert entry at
        // if there is already an entry do not insert the new one
        sal_Int32 nPos = 0;
        sal_Bool bFound = sal_False;
        if (bAddEntry)
        {
            bFound = seekEntry( xDicEntry->getDictionaryWord(), &nPos );
            if (bFound)
                bAddEntry = sal_False;
        }

        if (bAddEntry)
        {
            DBG_ASSERT(!bNeedEntries, "lng : entries still not loaded");

            if (nCount >= aEntries.getLength())
                aEntries.realloc( std::max(2 * nCount, nCount + 32) );
            uno::Reference< XDictionaryEntry > *pEntry = aEntries.getArray();

            // shift old entries right
            sal_Int32 i;
            for (i = nCount - 1; i >= nPos;  i--)
                pEntry[ i+1 ] = pEntry[ i ];
            // insert new entry at specified position
            pEntry[ nPos ] = xDicEntry;
            SAL_WARN_IF(!isSorted(), "linguistic", "dictionary entries unsorted");

            nCount++;

            bIsModified = sal_True;
            bRes = sal_True;

            if (!bIsLoadEntries)
                launchEvent( DictionaryEventFlags::ADD_ENTRY, xDicEntry );
        }
    }

    return bRes;
}


uno::Reference< XInterface > SAL_CALL DictionaryNeo_CreateInstance(
            const uno::Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{
    uno::Reference< XInterface > xService =
            (cppu::OWeakObject*) new DictionaryNeo;
    return xService;
}

OUString SAL_CALL DictionaryNeo::getName(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aDicName;
}

void SAL_CALL DictionaryNeo::setName( const OUString& aName )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (aDicName != aName)
    {
        aDicName = aName;
        launchEvent(DictionaryEventFlags::CHG_NAME, NULL);
    }
}

DictionaryType SAL_CALL DictionaryNeo::getDictionaryType(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return eDicType;
}

void SAL_CALL DictionaryNeo::setActive( sal_Bool bActivate )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bIsActive != bActivate)
    {
        bIsActive = bActivate != 0;
        sal_Int16 nEvent = bIsActive ?
                DictionaryEventFlags::ACTIVATE_DIC : DictionaryEventFlags::DEACTIVATE_DIC;

        // remove entries from memory if dictionary is deactivated
        if (bIsActive == sal_False)
        {
            sal_Bool bIsEmpty = nCount == 0;

            // save entries first if necessary
            if (bIsModified && hasLocation() && !isReadonly())
            {
                store();

                aEntries.realloc( 0 );
                nCount = 0;
                bNeedEntries = !bIsEmpty;
            }
            DBG_ASSERT( !bIsModified || !hasLocation() || isReadonly(),
                    "lng : dictionary is still modified" );
        }

        launchEvent(nEvent, NULL);
    }
}

sal_Bool SAL_CALL DictionaryNeo::isActive(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return bIsActive;
}

sal_Int32 SAL_CALL DictionaryNeo::getCount(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );
    return nCount;
}

Locale SAL_CALL DictionaryNeo::getLocale(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return LanguageTag::convertToLocale( nLanguage );
}

void SAL_CALL DictionaryNeo::setLocale( const Locale& aLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    sal_Int16 nLanguageP = LinguLocaleToLanguage( aLocale );
    if (!bIsReadonly  &&  nLanguage != nLanguageP)
    {
        nLanguage = nLanguageP;
        bIsModified = sal_True; // new language needs to be saved with dictionary

        launchEvent( DictionaryEventFlags::CHG_LANGUAGE, NULL );
    }
}

uno::Reference< XDictionaryEntry > SAL_CALL DictionaryNeo::getEntry(
            const OUString& aWord )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );

    sal_Int32 nPos;
    sal_Bool bFound = seekEntry( aWord, &nPos, sal_True );
    DBG_ASSERT( nCount <= aEntries.getLength(), "lng : wrong number of entries");
    DBG_ASSERT(!bFound || nPos < nCount, "lng : index out of range");

    return bFound ? aEntries.getConstArray()[ nPos ]
                    : uno::Reference< XDictionaryEntry >();
}

sal_Bool SAL_CALL DictionaryNeo::addEntry(
            const uno::Reference< XDictionaryEntry >& xDicEntry )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;

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
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;

    if (!bIsReadonly)
    {
        uno::Reference< XDictionaryEntry > xEntry =
                new DicEntry( rWord, bIsNegative, rRplcText );
        bRes = addEntry_Impl( xEntry );
    }

    return bRes;
}

static void lcl_SequenceRemoveElementAt(
            uno::Sequence< uno::Reference< XDictionaryEntry > >& rEntries, int nPos )
{
    //TODO: helper for SequenceRemoveElementAt available?
    if(nPos >= rEntries.getLength())
        return;
    uno::Sequence< uno::Reference< XDictionaryEntry > > aTmp(rEntries.getLength() - 1);
    uno::Reference< XDictionaryEntry > * pOrig = rEntries.getArray();
    uno::Reference< XDictionaryEntry > * pTemp = aTmp.getArray();
    int nOffset = 0;
    for(int i = 0; i < aTmp.getLength(); i++)
    {
        if(nPos == i)
            nOffset++;
        pTemp[i] = pOrig[i + nOffset];
    }

    rEntries = aTmp;
}

sal_Bool SAL_CALL DictionaryNeo::remove( const OUString& aWord )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRemoved = sal_False;

    if (!bIsReadonly)
    {
        if (bNeedEntries)
            loadEntries( aMainURL );

        sal_Int32 nPos;
        sal_Bool bFound = seekEntry( aWord, &nPos );
        DBG_ASSERT( nCount < aEntries.getLength(),
                "lng : wrong number of entries");
        DBG_ASSERT(!bFound || nPos < nCount, "lng : index out of range");

        // remove element if found
        if (bFound)
        {
            // entry to be removed
            uno::Reference< XDictionaryEntry >
                    xDicEntry( aEntries.getConstArray()[ nPos ] );
            DBG_ASSERT(xDicEntry.is(), "lng : dictionary entry is NULL");

            nCount--;

            //! the following call reduces the length of the sequence by 1 also
            lcl_SequenceRemoveElementAt( aEntries, nPos );
            bRemoved = bIsModified = sal_True;

            launchEvent( DictionaryEventFlags::DEL_ENTRY, xDicEntry );
        }
    }

    return bRemoved;
}

sal_Bool SAL_CALL DictionaryNeo::isFull(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );
    return nCount >= DIC_MAX_ENTRIES;
}

uno::Sequence< uno::Reference< XDictionaryEntry > >
    SAL_CALL DictionaryNeo::getEntries(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );
    //! return sequence with length equal to the number of dictionary entries
    //! (internal used sequence may have additional unused elements.)
    return uno::Sequence< uno::Reference< XDictionaryEntry > >
        (aEntries.getConstArray(), nCount);
}


void SAL_CALL DictionaryNeo::clear(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bIsReadonly && nCount)
    {
        // release all references to old entries and provide space for new ones
        aEntries = uno::Sequence< uno::Reference< XDictionaryEntry > > ( 32 );

        nCount = 0;
        bNeedEntries = sal_False;
        bIsModified = sal_True;

        launchEvent( DictionaryEventFlags::ENTRIES_CLEARED , NULL );
    }
}

sal_Bool SAL_CALL DictionaryNeo::addDictionaryEventListener(
            const uno::Reference< XDictionaryEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (xListener.is())
    {
        sal_Int32   nLen = aDicEvtListeners.getLength();
        bRes = aDicEvtListeners.addInterface( xListener ) != nLen;
    }
    return bRes;
}

sal_Bool SAL_CALL DictionaryNeo::removeDictionaryEventListener(
            const uno::Reference< XDictionaryEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (xListener.is())
    {
        sal_Int32   nLen = aDicEvtListeners.getLength();
        bRes = aDicEvtListeners.removeInterface( xListener ) != nLen;
    }
    return bRes;
}


sal_Bool SAL_CALL DictionaryNeo::hasLocation()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return !aMainURL.isEmpty();
}

OUString SAL_CALL DictionaryNeo::getLocation()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aMainURL;
}

sal_Bool SAL_CALL DictionaryNeo::isReadonly()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return bIsReadonly;
}

void SAL_CALL DictionaryNeo::store()
        throw(io::IOException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bIsModified && hasLocation() && !isReadonly())
    {
        if (!saveEntries( aMainURL ))
            bIsModified = sal_False;
    }
}

void SAL_CALL DictionaryNeo::storeAsURL(
            const OUString& aURL,
            const uno::Sequence< beans::PropertyValue >& /*rArgs*/ )
        throw(io::IOException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!saveEntries( aURL ))
    {
        aMainURL = aURL;
        bIsModified = sal_False;
        bIsReadonly = IsReadOnly( getLocation() );
    }
}

void SAL_CALL DictionaryNeo::storeToURL(
            const OUString& aURL,
            const uno::Sequence< beans::PropertyValue >& /*rArgs*/ )
        throw(io::IOException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    saveEntries(aURL);
}


DicEntry::DicEntry(const OUString &rDicFileWord,
                   sal_Bool bIsNegativWord)
{
    if (!rDicFileWord.isEmpty())
        splitDicFileWord( rDicFileWord, aDicWord, aReplacement );
    bIsNegativ = bIsNegativWord;
}

DicEntry::DicEntry(const OUString &rDicWord, sal_Bool bNegativ,
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

    static const OUString aDelim( "==" );

    sal_Int32 nDelimPos = rDicFileWord.indexOf( aDelim );
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
        rReplacement = OUString();
    }
}

OUString SAL_CALL DicEntry::getDictionaryWord(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aDicWord;
}

sal_Bool SAL_CALL DicEntry::isNegative(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return bIsNegativ;
}

OUString SAL_CALL DicEntry::getReplacementText(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aReplacement;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
