/*************************************************************************
 *
 *  $RCSfile: dicimp.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-19 14:52:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _LANG_HXX //autogen wg. LANGUAGE_ENGLISH_US
#include <tools/lang.hxx>
#endif

#ifndef _DICIMP_HXX
#include <dicimp.hxx>
#endif
#ifndef _HYPHIMP_HXX
#include <hyphdsp.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif

#include <com/sun/star/linguistic2/DictionaryType.hpp>
#include <com/sun/star/linguistic2/DictionaryEventFlags.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <cppuhelper/factory.hxx>   // helper for factories

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

///////////////////////////////////////////////////////////////////////////

#define BUFSIZE              256
#define VERS2_NOLANGUAGE    1024

static const sal_Char*      pDicExt     = "dic";
static const sal_Char*      pVerStr2    = "WBSWG2";
static const sal_Char*      pVerStr5    = "WBSWG5";
static const sal_Char*      pVerStr6    = "WBSWG6";

int GetDicVersion( const sal_Char *pVerStr )
{
    if (pVerStr)
    {
        if (0 == strcmp( pVerStr, pVerStr6 ))
            return 6;
        if (0 == strcmp( pVerStr, pVerStr5 ))
            return 5;
        if (0 == strcmp( pVerStr, pVerStr2 ))
            return 2;
    }
    return -1;
}

const String GetDicExtension()
{
    return String::CreateFromAscii( pDicExt );
}

///////////////////////////////////////////////////////////////////////////

DictionaryNeo::DictionaryNeo() :
    aDicEvtListeners( GetLinguMutex() ),
    eDicType        (DictionaryType_POSITIVE),
    nLanguage       (LANGUAGE_NONE)
{
    nCount       = 0;
    nDicVersion  = -1;
    bNeedEntries = FALSE;
    bIsModified  = bIsActive = FALSE;
    bIsReadonly  = FALSE;
}

DictionaryNeo::DictionaryNeo(const OUString &rName,
                             INT16 nLang, DictionaryType eType,
                             const OUString &rMainURL) :
    aDicEvtListeners( GetLinguMutex() ),
    aDicName        (rName),
    eDicType        (eType),
    nLanguage       (nLang),
    aMainURL        (rMainURL)
{
    nCount       = 0;
    nDicVersion  = -1;
    bNeedEntries = TRUE;
    bIsModified  = bIsActive = FALSE;

    if (rMainURL.getLength() > 0)
    {
        BOOL bExists = FALSE;
        try
        {
            ::ucb::Content aTestContent( rMainURL ,
                            Reference< ::com::sun::star::ucb::XCommandEnvironment >());
            bExists = aTestContent.isDocument();
        }
        catch( ::com::sun::star::uno::Exception )
        {
            bExists = FALSE;
        }
        if (!bExists)
        {
            // save new dictionaries with in 6.0 Format (uses UTF8)
            nDicVersion  = 6;

            //! create physical representation of an **empty** dictionary
            //! that could be searched for (see DicList::searchForDictionaries)
            // (Note: empty dictionaries are not just empty files!)
            saveEntries( rMainURL );
            bNeedEntries = FALSE;
        }
    }
    else
    {
        bNeedEntries = FALSE;
    }

    //! Creates empty file if there is no one yet!
    //! Thus it must be called after the test for the files existence.
    bIsReadonly = isReadonly_Impl();
}

DictionaryNeo::~DictionaryNeo()
{
}

ULONG DictionaryNeo::loadEntries(const OUString &rMainURL)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // counter check that it is safe to set bIsModified to FALSE at
    // the end of the function
    DBG_ASSERT(!bIsModified, "lng : dictionary already modified!");

    // function should only be called once in order to load entries from file
    bNeedEntries = FALSE;

    if (rMainURL.getLength() == 0)
        return 0;

    ULONG nErr = -1;

    // get stream to use
    SfxMedium aMedium( rMainURL, STREAM_READ | STREAM_SHARE_DENYWRITE, FALSE );
    aMedium.SetTransferPriority( SFX_TFPRIO_SYNCHRON );
    SvStream *pStream = aMedium.GetInStream();
    if (!pStream)
        return nErr;

    // Header einlesen
    BOOL    bSkip = FALSE;
    USHORT  nLen;

    *pStream >> nLen;
    if ((nErr = pStream->GetError()))
        return nErr;

#ifdef NO_MORE
    if( nLen > ICMAX )  // ICMAX = 64 max ICS Wortlänge
    {
        bDirty = TRUE;
        return FALSE;
    }
#endif

    sal_Char aWordBuf[ BUFSIZE ];
    BOOL bNegativ;

    pStream->Read(aWordBuf, nLen);
    if ((nErr = pStream->GetError()))
        return nErr;
    *(aWordBuf + nLen) = 0;

    nDicVersion = GetDicVersion( aWordBuf );

    rtl_TextEncoding eEnc = RTL_TEXTENCODING_MS_1252;
    if (6 == nDicVersion)
        eEnc = RTL_TEXTENCODING_UTF8;

    if (6 == nDicVersion ||
        5 == nDicVersion ||
        2 == nDicVersion)
    {
        bSkip = TRUE;
        // Sprache des Dictionaries
        *pStream >> nLanguage;
        if ((nErr = pStream->GetError()))
            return nErr;

        if ( VERS2_NOLANGUAGE == nLanguage )
            nLanguage = LANGUAGE_NONE;

        // Negativ-Flag
        sal_Char nTmp;
        *pStream >> nTmp;
        if ((nErr = pStream->GetError()))
            return nErr;
        bNegativ = (BOOL) nTmp;
        eDicType = bNegativ ? DictionaryType_NEGATIVE : DictionaryType_POSITIVE;

        // Das erste Wort einlesen
        if (!pStream->IsEof())
        {
            *pStream >> nLen;
            if ((nErr = pStream->GetError()))
                return nErr;
            if ( nLen < BUFSIZE )
            {
                pStream->Read(aWordBuf, nLen);
                if ((nErr = pStream->GetError()))
                    return nErr;
                *(aWordBuf + nLen) = 0;
            }
        }
    }

    nCount = 0;

    while(!pStream->IsEof())
    {
        // Aus dem File einlesen
        // Einfuegen ins Woerterbuch ohne Konvertierung
        if(*aWordBuf)
        {
            ByteString aDummy( aWordBuf );
            String aText( aDummy, eEnc );
            Reference< XDictionaryEntry > xEntry =
                    new DicEntry( aText, bNegativ );
            addEntry_Impl( xEntry , TRUE ); //! don't launch events here
        }

        *pStream >> nLen;
        if (pStream->IsEof())   // #75082# GPF in online-spelling
            break;
        if ((nErr = pStream->GetError()))
            return nErr;
#ifdef LINGU_EXCEPTIONS
        if ( nLen >= BUFSIZE )
            throw  io::IOException() ;
//          break;  // Woerterbuch defekt?
#endif

        if( nLen < BUFSIZE )
        {
            pStream->Read(aWordBuf, nLen);
            if ((nErr = pStream->GetError()))
                return nErr;
        }
        else
            return SVSTREAM_READ_ERROR;
        *(aWordBuf + nLen) = 0;
    }

    DBG_ASSERT(isSorted(), "lng : dictionary is not sorted");

    // since this routine should be called only initialy (prior to any
    // modification to be saved) we reset the bIsModified flag here that
    // was implicitly set by addEntry_Impl
    bIsModified = FALSE;

    return pStream->GetError();
}

ULONG DictionaryNeo::saveEntries(const OUString &rURL)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (rURL.getLength() == 0)
        return 0;

    ULONG nErr = -1;

    DBG_ASSERT(!INetURLObject( rURL ).HasError(), "lng : invalid URL");
    SfxMedium   aMedium( rURL, STREAM_WRITE | STREAM_TRUNC | STREAM_SHARE_DENYALL,
                         FALSE );
    aMedium.CreateTempFile();   // use temp file to write to...
    SvStream *pStream = aMedium.GetOutStream();
    if (!pStream)
        return nErr;

    sal_Char aWordBuf[BUFSIZE];

    // write version
    const sal_Char *pVerStr = NULL;
    if (6 == nDicVersion)
        pVerStr = pVerStr6;
    else
        pVerStr = eDicType == DictionaryType_POSITIVE ? pVerStr2 : pVerStr5;
    strcpy( aWordBuf, pVerStr );
    USHORT nLen = strlen( aWordBuf );
    *pStream << nLen;
    if ((nErr = pStream->GetError()))
        return nErr;
    pStream->Write(aWordBuf, nLen);
    if ((nErr = pStream->GetError()))
        return nErr;

    *pStream << nLanguage;
    if ((nErr = pStream->GetError()))
        return nErr;
    *pStream << (sal_Char) (eDicType == DictionaryType_NEGATIVE ? TRUE : FALSE);
    if ((nErr = pStream->GetError()))
        return nErr;

    rtl_TextEncoding eEnc = GetTextEncoding();
    if (6 == nDicVersion)
        eEnc = RTL_TEXTENCODING_UTF8;

    const Reference< XDictionaryEntry > *pEntry = aEntries.getConstArray();
    for (INT32 i = 0;  i < nCount;  i++)
    {
        BOOL    bIsNegativEntry = pEntry[i]->isNegative();

        ByteString  aTmp1 ( pEntry[i]->getDictionaryWord().getStr(),  eEnc ),
                    aTmp2 ( pEntry[i]->getReplacementText().getStr(), eEnc );
        if (bIsNegativEntry)
            aTmp1 += "==";
        xub_StrLen  nLen1 = aTmp1.Len(),
                    nLen2 = aTmp2.Len();
        if ((nLen = nLen1) < BUFSIZE)
        {
            strncpy( aWordBuf, aTmp1.GetBuffer(), nLen1 );
            if (bIsNegativEntry  &&  (nLen = nLen1 + nLen2) < BUFSIZE)
                strncpy( aWordBuf + nLen1, aTmp2.GetBuffer(), nLen2);
            *pStream << nLen;
            if ((nErr = pStream->GetError()))
                return nErr;
            pStream->Write(aWordBuf, nLen);
            if ((nErr = pStream->GetError()))
                return nErr;
        }
    }

    //! get return value before Stream is destroyed
    ULONG nError = pStream->GetError();

    // flush file, close it and release any lock
    aMedium.Close();
    BOOL bSucc = aMedium.Commit();
    DBG_ASSERT(bSucc == TRUE, "lng : SfxMedium::Commit failed");

    return nError;
}

void DictionaryNeo::launchEvent(INT16 nEvent,
                                Reference< XDictionaryEntry > xEntry)
{
    MutexGuard  aGuard( GetLinguMutex() );

    DictionaryEvent aEvt;
    aEvt.Source = Reference< XDictionary >( this );
    aEvt.nEvent = nEvent;
    aEvt.xDictionaryEntry = xEntry;

    cppu::OInterfaceIteratorHelper aIt( aDicEvtListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XDictionaryEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processDictionaryEvent( aEvt );
    }
}

int DictionaryNeo::cmpDicEntry(const OUString& rWord1,
                               const OUString &rWord2,
                               BOOL bSimilarOnly)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // returns 0 if rWord1 is equal to rWord2
    //   "     a value < 0 if rWord1 is less than rWord2
    //   "     a value > 0 if rWord1 is greater than rWord2

    int nRes = 0;

    OUString    aWord1( rWord1 ),
                aWord2( rWord2 );
    xub_StrLen  nLen1 = aWord1.getLength(),
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
    xub_StrLen  nIdx1 = 0,
                  nIdx2 = 0,
                  nNumIgnChar1 = 0,
                  nNumIgnChar2 = 0;

    sal_Int32 nDiff = 0;
    sal_Unicode cChar1, cChar2;
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
            if (aWord1[ nIdx2++ ] == cIgnChar)
                nNumIgnChar2++;
        }

        nRes = ((INT32) nLen1 - nNumIgnChar1) - ((INT32) nLen2 - nNumIgnChar2);
    }

    return nRes;
}

BOOL DictionaryNeo::seekEntry(const OUString &rWord,
                              INT32 *pPos, BOOL bSimilarOnly)
{
    // look for entry with binary search.
    // return TRUE if found FALSE else.
    // if pPos != NULL it will become the position of the found entry, or
    // if that was not found the position where it has to be inserted
    // to keep the entries sorted

    MutexGuard  aGuard( GetLinguMutex() );

    const Reference< XDictionaryEntry > *pEntry = aEntries.getConstArray();
    INT32 nUpperIdx = getCount(),
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
                return TRUE;
            }
            else if(nCmp > 0)
                nLowerIdx = nMidIdx + 1;
            else if( nMidIdx == 0 )
            {
                if( pPos ) *pPos = nLowerIdx;
                return FALSE;
            }
            else
                nUpperIdx = nMidIdx - 1;
        }
    }
    if( pPos ) *pPos = nLowerIdx;
    return FALSE;
}

BOOL DictionaryNeo::isSorted()
{
    BOOL bRes = TRUE;

    const Reference< XDictionaryEntry > *pEntry = aEntries.getConstArray();
    INT32 nEntries = getCount();
    INT32 i;
    for (i = 1;  i < nEntries;  i++)
    {
        if (cmpDicEntry( pEntry[i-1]->getDictionaryWord(),
                         pEntry[i]->getDictionaryWord() ) > 0)
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

BOOL DictionaryNeo::addEntry_Impl(const Reference< XDictionaryEntry > xDicEntry,
        BOOL bIsLoadEntries)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;

    if ( bIsLoadEntries || (!bIsReadonly  &&  xDicEntry.is()) )
    {
        BOOL bIsNegEntry = xDicEntry->isNegative();
        BOOL bAddEntry   = !isFull() &&
                   (   ( eDicType == DictionaryType_POSITIVE && !bIsNegEntry )
                    || ( eDicType == DictionaryType_NEGATIVE &&  bIsNegEntry )
                    || ( eDicType == DictionaryType_MIXED ) );

        // look for position to insert entry at
        // if there is already an entry do not insert the new one
        INT32 nPos = 0;
        BOOL bFound = FALSE;
        if (bAddEntry)
        {
            bFound = seekEntry( xDicEntry->getDictionaryWord(), &nPos );
            if (bFound)
                bAddEntry = FALSE;
        }

        if (bAddEntry)
        {
            DBG_ASSERT(!bNeedEntries, "lng : entries still not loaded")

            if (nCount >= aEntries.getLength())
                aEntries.realloc( Max(2 * nCount, nCount + 32) );
            Reference< XDictionaryEntry > *pEntry = aEntries.getArray();

            // shift old entries right
            INT32 i;
            for (i = nCount - 1; i >= nPos;  i--)
                pEntry[ i+1 ] = pEntry[ i ];
            // insert new entry at specified position
            pEntry[ nPos ] = xDicEntry;
            DBG_ASSERT(isSorted(), "lng : dictionary entries unsorted");

            nCount++;

            bIsModified = TRUE;
            bRes = TRUE;

            if (!bIsLoadEntries)
                launchEvent( DictionaryEventFlags::ADD_ENTRY, xDicEntry );
        }
    }

    return bRes;
}


Reference< XInterface > SAL_CALL DictionaryNeo_CreateInstance(
            const Reference< XMultiServiceFactory > & rSMgr )
        throw(Exception)
{
    Reference< XInterface > xService =
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
        INT16 nEvent = bIsActive ?
                DictionaryEventFlags::ACTIVATE_DIC : DictionaryEventFlags::DEACTIVATE_DIC;

        // remove entries from memory if dictionary is deactivated
        if (bIsActive == FALSE)
        {
            BOOL bIsEmpty = nCount == 0;

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

sal_Int16 SAL_CALL DictionaryNeo::getCount(  )
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
    Locale aRes;
    return LanguageToLocale( aRes, nLanguage );
}

void SAL_CALL DictionaryNeo::setLocale( const Locale& aLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    setLanguage( LocaleToLanguage( aLocale ) );
}

sal_Int16 SAL_CALL DictionaryNeo::getLanguage(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return nLanguage;
}

void SAL_CALL DictionaryNeo::setLanguage( sal_Int16 nLanguageP )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bIsReadonly  &&  nLanguage != nLanguageP)
    {
        nLanguage = nLanguageP;
        bIsModified = TRUE; // new language needs to be saved with dictionary

        launchEvent( DictionaryEventFlags::CHG_LANGUAGE, NULL );
    }
}


Reference< XDictionaryEntry > SAL_CALL DictionaryNeo::getEntry(
            const OUString& aWord )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );

    INT32 nPos;
    BOOL bFound = seekEntry( aWord, &nPos, TRUE );
    DBG_ASSERT( nCount <= aEntries.getLength(), "lng : wrong number of entries");
    DBG_ASSERT(!bFound || nPos < nCount, "lng : index out of range");

    return bFound ? aEntries.getConstArray()[ nPos ]
                    : Reference< XDictionaryEntry >();
}

sal_Bool SAL_CALL DictionaryNeo::addEntry(
            const Reference< XDictionaryEntry >& xDicEntry )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;

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

    BOOL bRes = FALSE;

    if (!bIsReadonly)
    {
        Reference< XDictionaryEntry > xEntry =
                new DicEntry( rWord, bIsNegative, rRplcText );
        bRes = addEntry_Impl( xEntry );
    }

    return bRes;
}

void lcl_SequenceRemoveElementAt(
            uno::Sequence< Reference< XDictionaryEntry > >& rEntries, int nPos )
{
    //TODO: helper for SequenceRemoveElementAt available?
    if(nPos >= rEntries.getLength())
        return;
    uno::Sequence< Reference< XDictionaryEntry > > aTmp(rEntries.getLength() - 1);
    Reference< XDictionaryEntry > * pOrig = rEntries.getArray();
    Reference< XDictionaryEntry > * pTemp = aTmp.getArray();
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

    BOOL bRemoved = FALSE;

    if (!bIsReadonly)
    {
        if (bNeedEntries)
            loadEntries( aMainURL );

        INT32 nPos;
        BOOL bFound = seekEntry( aWord, &nPos );
        DBG_ASSERT( nCount < aEntries.getLength(),
                "lng : wrong number of entries");
        DBG_ASSERT(!bFound || nPos < nCount, "lng : index out of range");

        // remove element if found
        if (bFound)
        {
            // entry to be removed
            Reference< XDictionaryEntry >
                    xDicEntry( aEntries.getConstArray()[ nPos ] );
            DBG_ASSERT(xDicEntry.is(), "lng : dictionary entry is NULL")

            nCount--;

            //! the following call reduces the length of the sequence by 1 also
            lcl_SequenceRemoveElementAt( aEntries, nPos );
            bRemoved = bIsModified = TRUE;

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

uno::Sequence< Reference< XDictionaryEntry > >
    SAL_CALL DictionaryNeo::getEntries(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bNeedEntries)
        loadEntries( aMainURL );
    //! return sequence with length equal to the number of dictionary entries
    //! (internal used sequence may have additional unused elements.)
    return uno::Sequence< Reference< XDictionaryEntry > >
        (aEntries.getConstArray(), nCount);
}


void SAL_CALL DictionaryNeo::clear(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bIsReadonly && nCount)
    {
        // release all references to old entries and provide space for new ones
        aEntries = uno::Sequence< Reference< XDictionaryEntry > > ( 32 );

        nCount = 0;
        bNeedEntries = FALSE;
        bIsModified = TRUE;

        launchEvent( DictionaryEventFlags::ENTRIES_CLEARED , NULL );
    }
}

sal_Bool SAL_CALL DictionaryNeo::addDictionaryEventListener(
            const Reference< XDictionaryEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;
    if (xListener.is())
    {
        INT32   nCount = aDicEvtListeners.getLength();
        bRes = aDicEvtListeners.addInterface( xListener ) != nCount;
    }
    return bRes;
}

sal_Bool SAL_CALL DictionaryNeo::removeDictionaryEventListener(
            const Reference< XDictionaryEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;
    if (xListener.is())
    {
        INT32   nCount = aDicEvtListeners.getLength();
        bRes = aDicEvtListeners.removeInterface( xListener ) != nCount;
    }
    return bRes;
}


sal_Bool SAL_CALL DictionaryNeo::hasLocation()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aMainURL.getLength() > 0;
}

OUString SAL_CALL DictionaryNeo::getLocation()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aMainURL;
}

BOOL DictionaryNeo::isReadonly_Impl()
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;

    if (hasLocation())
    {
        try
        {
            Reference< com::sun::star::ucb::XCommandEnvironment > xCmdEnv;
            ::ucb::Content aContent( getLocation(), xCmdEnv );
            Any aAny( aContent.getPropertyValue( A2OU( "IsReadOnly" ) ) );
            aAny >>= bRes;
        }
        catch (::com::sun::star::ucb::ContentCreationException &)
        {
            bRes = TRUE;
        }
    }

    return bRes;
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
        if (saveEntries( aMainURL ))
        {
#ifdef LINGU_EXCEPTIONS
            throw io::IOException();
#endif
        }
        else
            bIsModified = FALSE;
    }
}

void SAL_CALL DictionaryNeo::storeAsURL(
            const OUString& aURL,
            const uno::Sequence< beans::PropertyValue >& aArgs )
        throw(io::IOException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (saveEntries( aURL ))
    {
#ifdef LINGU_EXCEPTIONS
        throw io::IOException();
#endif
    }
    else
    {
        aMainURL = aURL;
        bIsModified = FALSE;
        bIsReadonly = isReadonly_Impl();
    }
}

void SAL_CALL DictionaryNeo::storeToURL(
            const OUString& aURL,
            const uno::Sequence< beans::PropertyValue >& aArgs )
        throw(io::IOException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (saveEntries( aURL ))
    {
#ifdef LINGU_EXCEPTIONS
        throw io::IOException();
#endif
    }
}

///////////////////////////////////////////////////////////////////////////

DicEntry::DicEntry()
{
    bIsNegativ = FALSE;
}

DicEntry::DicEntry(const OUString &rDicFileWord,
                   BOOL bIsNegativWord)
{
    if (rDicFileWord.getLength())
        splitDicFileWord( rDicFileWord, aDicWord, aReplacement );
    bIsNegativ = bIsNegativWord;
}

DicEntry::DicEntry(const OUString &rDicWord, BOOL bNegativ,
                   const OUString &rRplcText) :
    aDicWord                (rDicWord),
    bIsNegativ              (bNegativ),
    aReplacement            (rRplcText)
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

    static const OUString aDelim( A2OU( "==" ) );

    sal_Int32 nDelimPos = rDicFileWord.indexOf( aDelim );
    if (-1 != nDelimPos)
    {
        xub_StrLen nTriplePos = nDelimPos + 2;
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


///////////////////////////////////////////////////////////////////////////

