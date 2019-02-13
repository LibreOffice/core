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
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/linguistic2/ConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/util/XFlushListener.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/ElementExistException.hpp>


#include "convdic.hxx"
#include "convdicxml.hxx"
#include <linguistic/misc.hxx>
#include "defs.hxx"

using namespace std;
using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


#define SN_CONV_DICTIONARY      "com.sun.star.linguistic2.ConversionDictionary"

static void ReadThroughDic( const OUString &rMainURL, ConvDicXMLImport &rImport )
{
    if (rMainURL.isEmpty())
        return;
    DBG_ASSERT(!INetURLObject( rMainURL ).HasError(), "invalid URL");

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // get xInputStream stream
    uno::Reference< io::XInputStream > xIn;
    try
    {
        uno::Reference< ucb::XSimpleFileAccess3 > xAccess( ucb::SimpleFileAccess::create(xContext) );
        xIn = xAccess->openFileRead( rMainURL );
    }
    catch (const uno::Exception &)
    {
        SAL_WARN( "linguistic", "failed to get input stream" );
    }
    if (!xIn.is())
        return;

    SvStreamPtr pStream = SvStreamPtr( utl::UcbStreamHelper::CreateStream( xIn ) );

    // prepare ParserInputSource
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xIn;

    // get parser
    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

    //!! keep a reference until everything is done to
    //!! ensure the proper lifetime of the object
    uno::Reference < xml::sax::XDocumentHandler > xFilter(
            static_cast<xml::sax::XExtendedDocumentHandler *>(&rImport), UNO_QUERY );

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    // finally, parser the stream
    try
    {
        xParser->parseStream( aParserInput );   // implicitly calls ConvDicXMLImport::CreateContext
    }
    catch( xml::sax::SAXParseException& )
    {
    }
    catch( xml::sax::SAXException& )
    {
    }
    catch( io::IOException& )
    {
    }
}

bool IsConvDic( const OUString &rFileURL, LanguageType &nLang, sal_Int16 &nConvType )
{
    bool bRes = false;

    if (rFileURL.isEmpty())
        return bRes;

    // check if file extension matches CONV_DIC_EXT
    OUString aExt;
    sal_Int32 nPos = rFileURL.lastIndexOf( '.' );
    if (-1 != nPos)
        aExt = rFileURL.copy( nPos + 1 ).toAsciiLowerCase();
    if (aExt != CONV_DIC_EXT)
        return bRes;

    // first argument being 0 should stop the file from being parsed
    // up to the end (reading all entries) when the required
    // data (language, conversion type) is found.
    rtl::Reference<ConvDicXMLImport> pImport = new ConvDicXMLImport( nullptr );

    ReadThroughDic( rFileURL, *pImport );    // will implicitly add the entries
    bRes =  !LinguIsUnspecified( pImport->GetLanguage()) &&
            pImport->GetConversionType() != -1;
    DBG_ASSERT( bRes, "conversion dictionary corrupted?" );

    if (bRes)
    {
        nLang       = pImport->GetLanguage();
        nConvType   = pImport->GetConversionType();
    }

    return bRes;
}


ConvDic::ConvDic(
        const OUString &rName,
        LanguageType nLang,
        sal_Int16 nConvType,
        bool bBiDirectional,
        const OUString &rMainURL) :
    aFlushListeners( GetLinguMutex() )
{
    aName           = rName;
    nLanguage       = nLang;
    nConversionType = nConvType;
    aMainURL        = rMainURL;

    if (bBiDirectional)
        pFromRight.reset( new ConvMap );
    if (nLang == LANGUAGE_CHINESE_SIMPLIFIED || nLang == LANGUAGE_CHINESE_TRADITIONAL)
        pConvPropType.reset( new PropTypeMap );

    nMaxLeftCharCount = nMaxRightCharCount = 0;
    bMaxCharCountIsValid = true;

    bNeedEntries = true;
    bIsModified  = bIsActive = false;

    if( !rMainURL.isEmpty() )
    {
        bool bExists = false;
        IsReadOnly( rMainURL, &bExists );

        if( !bExists )  // new empty dictionary
        {
            bNeedEntries = false;
            //! create physical representation of an **empty** dictionary
            //! that could be found by the dictionary-list implementation
            // (Note: empty dictionaries are not just empty files!)
            Save();
        }
    }
    else
    {
        bNeedEntries = false;
    }
}


ConvDic::~ConvDic()
{
}


void ConvDic::Load()
{
    DBG_ASSERT( !bIsModified, "dictionary is modified. Really do 'Load'?" );

    //!! prevent function from being called recursively via HasEntry, AddEntry
    bNeedEntries = false;
    rtl::Reference<ConvDicXMLImport> pImport = new ConvDicXMLImport( this );
    ReadThroughDic( aMainURL, *pImport );    // will implicitly add the entries
    bIsModified = false;
}


void ConvDic::Save()
{
    DBG_ASSERT( !bNeedEntries, "saving while entries missing" );
    if (aMainURL.isEmpty() || bNeedEntries)
        return;
    DBG_ASSERT(!INetURLObject( aMainURL ).HasError(), "invalid URL");

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // get XOutputStream stream
    uno::Reference< io::XStream > xStream;
    try
    {
        uno::Reference< ucb::XSimpleFileAccess3 > xAccess( ucb::SimpleFileAccess::create(xContext) );
        xStream = xAccess->openFileReadWrite( aMainURL );
    }
    catch (const uno::Exception &)
    {
        SAL_WARN( "linguistic", "failed to get input stream" );
    }
    if (!xStream.is())
        return;

    SvStreamPtr pStream = SvStreamPtr( utl::UcbStreamHelper::CreateStream( xStream ) );

    // get XML writer
    uno::Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(xContext);

    if (xStream.is())
    {
        // connect XML writer to output stream
        xSaxWriter->setOutputStream( xStream->getOutputStream() );

        // prepare arguments (prepend doc handler to given arguments)
        uno::Reference< xml::sax::XDocumentHandler > xDocHandler( xSaxWriter, UNO_QUERY );
        rtl::Reference<ConvDicXMLExport> pExport = new ConvDicXMLExport( *this, aMainURL, xDocHandler );
        bool bRet = pExport->Export();     // write entries to file
        DBG_ASSERT( !pStream->GetError(), "I/O error while writing to stream" );
        if (bRet)
            bIsModified = false;
    }
    DBG_ASSERT( !bIsModified, "dictionary still modified after save. Save failed?" );
}


ConvMap::iterator ConvDic::GetEntry( ConvMap &rMap, const OUString &rFirstText, const OUString &rSecondText )
{
    pair< ConvMap::iterator, ConvMap::iterator > aRange =
            rMap.equal_range( rFirstText );
    ConvMap::iterator aPos = rMap.end();
    for (ConvMap::iterator aIt = aRange.first;
         aIt != aRange.second  &&  aPos == rMap.end();
         ++aIt)
    {
        if ((*aIt).second == rSecondText)
            aPos = aIt;
    }
    return aPos;
}


bool ConvDic::HasEntry( const OUString &rLeftText, const OUString &rRightText )
{
    if (bNeedEntries)
        Load();
    ConvMap::iterator aIt = GetEntry( aFromLeft, rLeftText, rRightText );
    return aIt != aFromLeft.end();
}


void ConvDic::AddEntry( const OUString &rLeftText, const OUString &rRightText )
{
    if (bNeedEntries)
        Load();

    DBG_ASSERT(!HasEntry( rLeftText, rRightText), "entry already exists" );
    aFromLeft .emplace( rLeftText, rRightText );
    if (pFromRight)
        pFromRight->emplace( rRightText, rLeftText );

    if (bMaxCharCountIsValid)
    {
        if (rLeftText.getLength() > nMaxLeftCharCount)
            nMaxLeftCharCount   = static_cast<sal_Int16>(rLeftText.getLength());
        if (pFromRight.get() && rRightText.getLength() > nMaxRightCharCount)
            nMaxRightCharCount  = static_cast<sal_Int16>(rRightText.getLength());
    }

    bIsModified = true;
}


void ConvDic::RemoveEntry( const OUString &rLeftText, const OUString &rRightText )
{
    if (bNeedEntries)
        Load();

    ConvMap::iterator aLeftIt  = GetEntry( aFromLeft,  rLeftText,  rRightText );
    DBG_ASSERT( aLeftIt  != aFromLeft.end(),  "left map entry missing" );
    aFromLeft .erase( aLeftIt );

    if (pFromRight)
    {
        ConvMap::iterator aRightIt = GetEntry( *pFromRight, rRightText, rLeftText );
        DBG_ASSERT( aRightIt != pFromRight->end(), "right map entry missing" );
        pFromRight->erase( aRightIt );
    }

    bIsModified = true;
    bMaxCharCountIsValid = false;
}


OUString SAL_CALL ConvDic::getName(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aName;
}


Locale SAL_CALL ConvDic::getLocale(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return LanguageTag::convertToLocale( nLanguage );
}


sal_Int16 SAL_CALL ConvDic::getConversionType(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return nConversionType;
}


void SAL_CALL ConvDic::setActive( sal_Bool bActivate )
{
    MutexGuard  aGuard( GetLinguMutex() );
    bIsActive = bActivate;
}


sal_Bool SAL_CALL ConvDic::isActive(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return bIsActive;
}


void SAL_CALL ConvDic::clear(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    aFromLeft .clear();
    if (pFromRight)
        pFromRight->clear();
    bNeedEntries    = false;
    bIsModified     = true;
    nMaxLeftCharCount       = 0;
    nMaxRightCharCount      = 0;
    bMaxCharCountIsValid    = true;
}


uno::Sequence< OUString > SAL_CALL ConvDic::getConversions(
        const OUString& aText,
        sal_Int32 nStartPos,
        sal_Int32 nLength,
        ConversionDirection eDirection,
        sal_Int32 /*nTextConversionOptions*/ )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pFromRight && eDirection == ConversionDirection_FROM_RIGHT)
        return uno::Sequence< OUString >();

    if (bNeedEntries)
        Load();

    OUString aLookUpText( aText.copy(nStartPos, nLength) );
    ConvMap &rConvMap = eDirection == ConversionDirection_FROM_LEFT ?
                                aFromLeft : *pFromRight;
    pair< ConvMap::iterator, ConvMap::iterator > aRange =
            rConvMap.equal_range( aLookUpText );

    sal_Int32 nCount = 0;
    ConvMap::iterator aIt;
    for (aIt = aRange.first;  aIt != aRange.second;  ++aIt)
        ++nCount;

    uno::Sequence< OUString > aRes( nCount );
    OUString *pRes = aRes.getArray();
    sal_Int32 i = 0;
    for (aIt = aRange.first;  aIt != aRange.second;  ++aIt)
        pRes[i++] = (*aIt).second;

    return aRes;
}


static bool lcl_SeqHasEntry(
    const OUString *pSeqStart,  // first element to check
    sal_Int32 nToCheck,             // number of elements to check
    const OUString &rText)
{
    bool bRes = false;
    if (pSeqStart && nToCheck > 0)
    {
        const OUString *pDone = pSeqStart + nToCheck;   // one behind last to check
        while (!bRes && pSeqStart != pDone)
        {
            if (*pSeqStart++ == rText)
                bRes = true;
        }
    }
    return bRes;
}

uno::Sequence< OUString > SAL_CALL ConvDic::getConversionEntries(
        ConversionDirection eDirection )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pFromRight && eDirection == ConversionDirection_FROM_RIGHT)
        return uno::Sequence< OUString >();

    if (bNeedEntries)
        Load();

    ConvMap &rConvMap = eDirection == ConversionDirection_FROM_LEFT ?
                                aFromLeft : *pFromRight;
    uno::Sequence< OUString > aRes( rConvMap.size() );
    OUString *pRes = aRes.getArray();
    sal_Int32 nIdx = 0;
    for (auto const& elem : rConvMap)
    {
        OUString aCurEntry( elem.first );
        // skip duplicate entries ( duplicate = duplicate entries
        // respective to the evaluated side (FROM_LEFT or FROM_RIGHT).
        // Thus if FROM_LEFT is evaluated for pairs (A,B) and (A,C)
        // only one entry for A will be returned in the result)
        if (nIdx == 0 || !lcl_SeqHasEntry( pRes, nIdx, aCurEntry ))
            pRes[ nIdx++ ] = aCurEntry;
    }
    aRes.realloc( nIdx );

    return aRes;
}


void SAL_CALL ConvDic::addEntry(
        const OUString& aLeftText,
        const OUString& aRightText )
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (bNeedEntries)
        Load();
    if (HasEntry( aLeftText, aRightText ))
        throw container::ElementExistException();
    AddEntry( aLeftText, aRightText );
}


void SAL_CALL ConvDic::removeEntry(
        const OUString& aLeftText,
        const OUString& aRightText )
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (bNeedEntries)
        Load();
    if (!HasEntry( aLeftText, aRightText ))
        throw container::NoSuchElementException();
    RemoveEntry( aLeftText, aRightText );
}


sal_Int16 SAL_CALL ConvDic::getMaxCharCount( ConversionDirection eDirection )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pFromRight && eDirection == ConversionDirection_FROM_RIGHT)
    {
        DBG_ASSERT( nMaxRightCharCount == 0, "max right char count should be 0" );
        return 0;
    }

    if (bNeedEntries)
        Load();

    if (!bMaxCharCountIsValid)
    {
        nMaxLeftCharCount   = 0;
        for (auto const& elem : aFromLeft)
        {
            sal_Int16 nTmp = static_cast<sal_Int16>(elem.first.getLength());
            if (nTmp > nMaxLeftCharCount)
                nMaxLeftCharCount = nTmp;
        }

        nMaxRightCharCount  = 0;
        if (pFromRight)
        {
            for (auto const& elem : *pFromRight)
            {
                sal_Int16 nTmp = static_cast<sal_Int16>(elem.first.getLength());
                if (nTmp > nMaxRightCharCount)
                    nMaxRightCharCount = nTmp;
            }
        }

        bMaxCharCountIsValid = true;
    }
    sal_Int16 nRes = eDirection == ConversionDirection_FROM_LEFT ?
            nMaxLeftCharCount : nMaxRightCharCount;
    DBG_ASSERT( nRes >= 0, "invalid MaxCharCount" );
    return nRes;
}


void SAL_CALL ConvDic::setPropertyType(
        const OUString& rLeftText,
        const OUString& rRightText,
        sal_Int16 nPropertyType )
{
    bool bHasElement = HasEntry( rLeftText, rRightText);
    if (!bHasElement)
        throw container::NoSuchElementException();

    // currently we assume that entries with the same left text have the
    // same PropertyType even if the right text is different...
    if (pConvPropType)
        pConvPropType->emplace( rLeftText, nPropertyType );
    bIsModified = true;
}


sal_Int16 SAL_CALL ConvDic::getPropertyType(
        const OUString& rLeftText,
        const OUString& rRightText )
{
    bool bHasElement = HasEntry( rLeftText, rRightText);
    if (!bHasElement)
        throw container::NoSuchElementException();

    sal_Int16 nRes = ConversionPropertyType::NOT_DEFINED;
    if (pConvPropType)
    {
        // still assuming that entries with same left text have same PropertyType
        // even if they have different right text...
        PropTypeMap::iterator aIt = pConvPropType->find( rLeftText );
        if (aIt != pConvPropType->end())
            nRes = (*aIt).second;
    }
    return nRes;
}


void SAL_CALL ConvDic::flush(  )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bIsModified)
        return;

    Save();

    // notify listeners
    EventObject aEvtObj;
    aEvtObj.Source = uno::Reference< XFlushable >( this );
    aFlushListeners.notifyEach( &util::XFlushListener::flushed, aEvtObj );
}


void SAL_CALL ConvDic::addFlushListener(
        const uno::Reference< util::XFlushListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (rxListener.is())
        aFlushListeners.addInterface( rxListener );
}


void SAL_CALL ConvDic::removeFlushListener(
        const uno::Reference< util::XFlushListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (rxListener.is())
        aFlushListeners.removeInterface( rxListener );
}


OUString SAL_CALL ConvDic::getImplementationName(  )
{
    return OUString( "com.sun.star.lingu2.ConvDic" );
}

sal_Bool SAL_CALL ConvDic::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL ConvDic::getSupportedServiceNames(  )
{
    return { SN_CONV_DICTIONARY };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
