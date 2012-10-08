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
#include <i18npool/lang.h>
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/XConversionPropertyType.hpp>
#include <com/sun/star/linguistic2/ConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/util/XFlushListener.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>


#include "convdic.hxx"
#include "convdicxml.hxx"
#include "linguistic/misc.hxx"
#include "defs.hxx"

using namespace std;
using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;

#define SN_CONV_DICTIONARY      "com.sun.star.linguistic2.ConversionDictionary"

void ReadThroughDic( const String &rMainURL, ConvDicXMLImport &rImport )
{
    if (rMainURL.Len() == 0)
        return;
    DBG_ASSERT(!INetURLObject( rMainURL ).HasError(), "invalid URL");

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // get xInputStream stream
    uno::Reference< io::XInputStream > xIn;
    try
    {
        uno::Reference< ucb::XSimpleFileAccess2 > xAccess( ucb::SimpleFileAccess::create(xContext) );
        xIn = xAccess->openFileRead( rMainURL );
    }
    catch (const uno::Exception &)
    {
        DBG_ASSERT( 0, "failed to get input stream" );
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
            (xml::sax::XExtendedDocumentHandler *) &rImport, UNO_QUERY );

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

sal_Bool IsConvDic( const String &rFileURL, sal_Int16 &nLang, sal_Int16 &nConvType )
{
    sal_Bool bRes = sal_False;

    if (rFileURL.Len() == 0)
        return bRes;

    // check if file extension matches CONV_DIC_EXT
    String aExt;
    xub_StrLen nPos = rFileURL.SearchBackward( '.' );
    if (STRING_NOTFOUND != nPos)
        aExt = rFileURL.Copy( nPos + 1 );
    aExt.ToLowerAscii();
    if (!aExt.EqualsAscii( CONV_DIC_EXT ))
        return bRes;

    // first argument being 0 should stop the file from being parsed
    // up to the end (reading all entries) when the required
    // data (language, conversion type) is found.
    ConvDicXMLImport *pImport = new ConvDicXMLImport( 0, rFileURL );

    //!! keep a first reference to ensure the lifetime of the object !!
    uno::Reference< XInterface > xRef( (document::XFilter *) pImport, UNO_QUERY );

    ReadThroughDic( rFileURL, *pImport );    // will implicitly add the entries
    bRes =  pImport->GetLanguage() != LANGUAGE_NONE &&
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
        const String &rName,
        sal_Int16 nLang,
        sal_Int16 nConvType,
             sal_Bool bBiDirectional,
        const String &rMainURL) :
    aFlushListeners( GetLinguMutex() )
{
    aName           = rName;
    nLanguage       = nLang;
    nConversionType = nConvType;
    aMainURL        = rMainURL;

    if (bBiDirectional)
        pFromRight = std::auto_ptr< ConvMap >( new ConvMap );
    if (nLang == LANGUAGE_CHINESE_SIMPLIFIED || nLang == LANGUAGE_CHINESE_TRADITIONAL)
        pConvPropType = std::auto_ptr< PropTypeMap >( new PropTypeMap );

    nMaxLeftCharCount = nMaxRightCharCount = 0;
    bMaxCharCountIsValid = sal_True;

    bNeedEntries = sal_True;
    bIsModified  = bIsActive = sal_False;
    bIsReadonly = sal_False;

    if( rMainURL.Len() > 0 )
    {
        sal_Bool bExists = sal_False;
        bIsReadonly = IsReadOnly( rMainURL, &bExists );

        if( !bExists )  // new empty dictionary
        {
            bNeedEntries = sal_False;
            //! create physical representation of an **empty** dictionary
            //! that could be found by the dictionary-list implementation
            // (Note: empty dictionaries are not just empty files!)
            Save();
            bIsReadonly = IsReadOnly( rMainURL );   // will be sal_False if Save was successful
        }
    }
    else
    {
        bNeedEntries = sal_False;
    }
}


ConvDic::~ConvDic()
{
}


void ConvDic::Load()
{
    DBG_ASSERT( !bIsModified, "dictionary is modified. Really do 'Load'?" );

    //!! prevent function from being called recursively via HasEntry, AddEntry
    bNeedEntries = sal_False;
    ConvDicXMLImport *pImport = new ConvDicXMLImport( this, aMainURL );
    //!! keep a first reference to ensure the lifetime of the object !!
    uno::Reference< XInterface > xRef( (document::XFilter *) pImport, UNO_QUERY );
    ReadThroughDic( aMainURL, *pImport );    // will implicitly add the entries
    bIsModified = sal_False;
}


void ConvDic::Save()
{
    DBG_ASSERT( !bNeedEntries, "saving while entries missing" );
    if (aMainURL.Len() == 0 || bNeedEntries)
        return;
    DBG_ASSERT(!INetURLObject( aMainURL ).HasError(), "invalid URL");

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    uno::Reference< lang::XMultiServiceFactory > xServiceFactory( comphelper::getProcessServiceFactory() );

    // get XOutputStream stream
    uno::Reference< io::XStream > xStream;
    try
    {
        uno::Reference< ucb::XSimpleFileAccess2 > xAccess( ucb::SimpleFileAccess::create(xContext) );
        xStream = xAccess->openFileReadWrite( aMainURL );
    }
    catch (const uno::Exception &)
    {
        DBG_ASSERT( 0, "failed to get input stream" );
    }
    if (!xStream.is())
        return;

    SvStreamPtr pStream = SvStreamPtr( utl::UcbStreamHelper::CreateStream( xStream ) );

    // get XML writer
    uno::Reference< io::XActiveDataSource > xSaxWriter;
    if (xServiceFactory.is())
    {
        try
        {
            xSaxWriter = uno::Reference< io::XActiveDataSource >(
                    xServiceFactory->createInstance( "com.sun.star.xml.sax.Writer" ), UNO_QUERY );
        }
        catch (uno::Exception &)
        {
        }
    }
    DBG_ASSERT( xSaxWriter.is(), "can't instantiate XML writer" );

    if (xSaxWriter.is() && xStream.is())
    {
        // connect XML writer to output stream
        xSaxWriter->setOutputStream( xStream->getOutputStream() );

        // prepare arguments (prepend doc handler to given arguments)
        uno::Reference< xml::sax::XDocumentHandler > xDocHandler( xSaxWriter, UNO_QUERY );
        ConvDicXMLExport *pExport = new ConvDicXMLExport( *this, aMainURL, xDocHandler );
        //!! keep a first(!) reference until everything is done to
        //!! ensure the proper lifetime of the object
        uno::Reference< document::XFilter > aRef( (document::XFilter *) pExport );
        sal_Bool bRet = pExport->Export();     // write entries to file
        DBG_ASSERT( !pStream->GetError(), "I/O error while writing to stream" );
        if (bRet)
            bIsModified = sal_False;
    }
    DBG_ASSERT( !bIsModified, "dictionary still modified after save. Save failed?" );
}


ConvMap::iterator ConvDic::GetEntry( ConvMap &rMap, const rtl::OUString &rFirstText, const rtl::OUString &rSecondText )
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


sal_Bool ConvDic::HasEntry( const OUString &rLeftText, const OUString &rRightText )
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
    aFromLeft .insert( ConvMap::value_type( rLeftText, rRightText ) );
    if (pFromRight.get())
        pFromRight->insert( ConvMap::value_type( rRightText, rLeftText ) );

    if (bMaxCharCountIsValid)
    {
        if (rLeftText.getLength() > nMaxLeftCharCount)
            nMaxLeftCharCount   = (sal_Int16) rLeftText.getLength();
        if (pFromRight.get() && rRightText.getLength() > nMaxRightCharCount)
            nMaxRightCharCount  = (sal_Int16) rRightText.getLength();
    }

    bIsModified = sal_True;
}


void ConvDic::RemoveEntry( const OUString &rLeftText, const OUString &rRightText )
{
    if (bNeedEntries)
        Load();

    ConvMap::iterator aLeftIt  = GetEntry( aFromLeft,  rLeftText,  rRightText );
    DBG_ASSERT( aLeftIt  != aFromLeft.end(),  "left map entry missing" );
    aFromLeft .erase( aLeftIt );

    if (pFromRight.get())
    {
        ConvMap::iterator aRightIt = GetEntry( *pFromRight, rRightText, rLeftText );
        DBG_ASSERT( aRightIt != pFromRight->end(), "right map entry missing" );
        pFromRight->erase( aRightIt );
    }

    bIsModified = sal_True;
    bMaxCharCountIsValid = sal_False;
}


OUString SAL_CALL ConvDic::getName(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aName;
}


Locale SAL_CALL ConvDic::getLocale(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return CreateLocale( nLanguage );
}


sal_Int16 SAL_CALL ConvDic::getConversionType(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return nConversionType;
}


void SAL_CALL ConvDic::setActive( sal_Bool bActivate )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    bIsActive = bActivate;
}


sal_Bool SAL_CALL ConvDic::isActive(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return bIsActive;
}


void SAL_CALL ConvDic::clear(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    aFromLeft .clear();
    if (pFromRight.get())
        pFromRight->clear();
    bNeedEntries    = sal_False;
    bIsModified     = sal_True;
    nMaxLeftCharCount       = 0;
    nMaxRightCharCount      = 0;
    bMaxCharCountIsValid    = sal_True;
}


uno::Sequence< OUString > SAL_CALL ConvDic::getConversions(
        const OUString& aText,
        sal_Int32 nStartPos,
        sal_Int32 nLength,
        ConversionDirection eDirection,
        sal_Int32 /*nTextConversionOptions*/ )
    throw (IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pFromRight.get() && eDirection == ConversionDirection_FROM_RIGHT)
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


static sal_Bool lcl_SeqHasEntry(
    const OUString *pSeqStart,  // first element to check
    sal_Int32 nToCheck,             // number of elements to check
    const OUString &rText)
{
    sal_Bool bRes = sal_False;
    if (pSeqStart && nToCheck > 0)
    {
        const OUString *pDone = pSeqStart + nToCheck;   // one behind last to check
        while (!bRes && pSeqStart != pDone)
        {
            if (*pSeqStart++ == rText)
                bRes = sal_True;
        }
    }
    return bRes;
}

uno::Sequence< OUString > SAL_CALL ConvDic::getConversionEntries(
        ConversionDirection eDirection )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pFromRight.get() && eDirection == ConversionDirection_FROM_RIGHT)
        return uno::Sequence< OUString >();

    if (bNeedEntries)
        Load();

    ConvMap &rConvMap = eDirection == ConversionDirection_FROM_LEFT ?
                                aFromLeft : *pFromRight;
    uno::Sequence< OUString > aRes( rConvMap.size() );
    OUString *pRes = aRes.getArray();
    ConvMap::iterator aIt = rConvMap.begin();
    sal_Int32 nIdx = 0;
    while (aIt != rConvMap.end())
    {
        OUString aCurEntry( (*aIt).first );
        // skip duplicate entries ( duplicate = duplicate entries
        // respective to the evaluated side (FROM_LEFT or FROM_RIGHT).
        // Thus if FROM_LEFT is evaluated for pairs (A,B) and (A,C)
        // only one entry for A will be returned in the result)
        if (nIdx == 0 || !lcl_SeqHasEntry( pRes, nIdx, aCurEntry ))
            pRes[ nIdx++ ] = aCurEntry;
        ++aIt;
    }
    aRes.realloc( nIdx );

    return aRes;
}


void SAL_CALL ConvDic::addEntry(
        const OUString& aLeftText,
        const OUString& aRightText )
    throw (IllegalArgumentException, container::ElementExistException, RuntimeException)
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
    throw (container::NoSuchElementException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (bNeedEntries)
        Load();
    if (!HasEntry( aLeftText, aRightText ))
        throw container::NoSuchElementException();
    RemoveEntry( aLeftText, aRightText );
}


sal_Int16 SAL_CALL ConvDic::getMaxCharCount( ConversionDirection eDirection )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pFromRight.get() && eDirection == ConversionDirection_FROM_RIGHT)
    {
        DBG_ASSERT( nMaxRightCharCount == 0, "max right char count should be 0" );
        return 0;
    }

    if (bNeedEntries)
        Load();

    if (!bMaxCharCountIsValid)
    {
        nMaxLeftCharCount   = 0;
        ConvMap::iterator aIt = aFromLeft.begin();
        while (aIt != aFromLeft.end())
        {
            sal_Int16 nTmp = (sal_Int16) (*aIt).first.getLength();
            if (nTmp > nMaxLeftCharCount)
                nMaxLeftCharCount = nTmp;
            ++aIt;
        }

        nMaxRightCharCount  = 0;
        if (pFromRight.get())
        {
            aIt = pFromRight->begin();
            while (aIt != pFromRight->end())
            {
                sal_Int16 nTmp = (sal_Int16) (*aIt).first.getLength();
                if (nTmp > nMaxRightCharCount)
                    nMaxRightCharCount = nTmp;
                ++aIt;
            }
        }

        bMaxCharCountIsValid = sal_True;
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
    throw (container::NoSuchElementException, IllegalArgumentException, RuntimeException)
{
    sal_Bool bHasElement = HasEntry( rLeftText, rRightText);
    if (!bHasElement)
        throw container::NoSuchElementException();

    // currently we assume that entries with the same left text have the
    // same PropertyType even if the right text is different...
    if (pConvPropType.get())
        pConvPropType->insert( PropTypeMap::value_type( rLeftText, nPropertyType ) );
    bIsModified = sal_True;
}


sal_Int16 SAL_CALL ConvDic::getPropertyType(
        const OUString& rLeftText,
        const OUString& rRightText )
    throw (container::NoSuchElementException, RuntimeException)
{
    sal_Bool bHasElement = HasEntry( rLeftText, rRightText);
    if (!bHasElement)
        throw container::NoSuchElementException();

    sal_Int16 nRes = ConversionPropertyType::NOT_DEFINED;
    if (pConvPropType.get())
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
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bIsModified)
        return;

    Save();

    // notify listeners
    EventObject aEvtObj;
    aEvtObj.Source = uno::Reference< XFlushable >( this );
    cppu::OInterfaceIteratorHelper aIt( aFlushListeners );
    while (aIt.hasMoreElements())
    {
        uno::Reference< util::XFlushListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->flushed( aEvtObj );
    }
}


void SAL_CALL ConvDic::addFlushListener(
        const uno::Reference< util::XFlushListener >& rxListener )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (rxListener.is())
        aFlushListeners.addInterface( rxListener );
}


void SAL_CALL ConvDic::removeFlushListener(
        const uno::Reference< util::XFlushListener >& rxListener )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (rxListener.is())
        aFlushListeners.removeInterface( rxListener );
}


OUString SAL_CALL ConvDic::getImplementationName(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}


sal_Bool SAL_CALL ConvDic::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    sal_Bool bRes = sal_False;
    if ( rServiceName == SN_CONV_DICTIONARY )
        bRes = sal_True;
    return bRes;
}


uno::Sequence< OUString > SAL_CALL ConvDic::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}


uno::Sequence< OUString > ConvDic::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = SN_CONV_DICTIONARY ;
    return aSNS;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
