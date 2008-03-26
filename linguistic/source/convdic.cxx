/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convdic.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-26 09:04:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
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
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
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

#include <cppuhelper/factory.hxx>   // helper for factories

#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARY_HPP_
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_CONVERSIONDICTIONARYTYPE_HPP_
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONPROPERTYTYPE_HPP_
#include <com/sun/star/linguistic2/XConversionPropertyType.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_CONVERSIONPROPERTYTYPE_HPP_
#include <com/sun/star/linguistic2/ConversionPropertyType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HPP_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHLISTENER_HPP_
#include <com/sun/star/util/XFlushListener.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif


#include "convdic.hxx"
#include "convdicxml.hxx"
#include "misc.hxx"
#include "defs.hxx"

using namespace std;
using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

#define SN_CONV_DICTIONARY      "com.sun.star.linguistic2.ConversionDictionary"
#define SN_HCD_CONV_DICTIONARY  "com.sun.star.linguistic2.HangulHanjaConversionDictionary"


///////////////////////////////////////////////////////////////////////////

void ReadThroughDic( const String &rMainURL, ConvDicXMLImport &rImport )
{
    if (rMainURL.Len() == 0)
        return;

    // get stream to be used
    DBG_ASSERT(!INetURLObject( rMainURL ).HasError(), "invalid URL");
    SfxMedium aMedium( rMainURL, STREAM_READ | STREAM_SHARE_DENYWRITE, FALSE );
    SvStream *pStream = aMedium.GetInStream();
    DBG_ASSERT( pStream, "input stream missing" );
    if (!pStream || pStream->GetError())
        return;

    uno::Reference< lang::XMultiServiceFactory > xServiceFactory(
            utl::getProcessServiceFactory() );
    DBG_ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );
    if (!xServiceFactory.is())
        return;

    uno::Reference< io::XInputStream > xIn = new utl::OInputStreamWrapper( *pStream );
    DBG_ASSERT( xIn.is(), "input stream missing" );

    ULONG nError = sal::static_int_cast< ULONG >(-1);

    // prepare ParserInputSource
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xIn;

    // get parser
    uno::Reference< xml::sax::XParser > xParser;
    try
    {
        xParser = uno::Reference< xml::sax::XParser >( xServiceFactory->createInstance(
            A2OU( "com.sun.star.xml.sax.Parser" ) ), UNO_QUERY );
    }
    catch (uno::Exception &)
    {
    }
    DBG_ASSERT( xParser.is(), "Can't create parser" );
    if (!xParser.is())
        return;

    // get filter
    //ConvDicXMLImport *pImport = new ConvDicXMLImport( this, rMainURL );
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
        if (rImport.GetSuccess())
            nError = 0;
    }
    catch( xml::sax::SAXParseException& )
    {
//        if( bEncrypted )
//            nError = ERRCODE_SFX_WRONGPASSWORD;
    }
    catch( xml::sax::SAXException& )
    {
//        if( bEncrypted )
//            nError = ERRCODE_SFX_WRONGPASSWORD;
    }
    catch( io::IOException& )
    {
    }
}


BOOL IsConvDic( const String &rFileURL, INT16 &nLang, sal_Int16 &nConvType )
{
    BOOL bRes = FALSE;

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
    DBG_ASSERT( bRes, "conversion dictionary corrupted?" )

    if (bRes)
    {
        nLang       = pImport->GetLanguage();
        nConvType   = pImport->GetConversionType();
    }

    return bRes;
}


///////////////////////////////////////////////////////////////////////////

ConvDic::ConvDic(
        const String &rName,
        INT16 nLang,
        sal_Int16 nConvType,
             BOOL bBiDirectional,
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
    bMaxCharCountIsValid = TRUE;

    bNeedEntries = TRUE;
    bIsModified  = bIsActive = FALSE;
    bIsReadonly = FALSE;

    if( rMainURL.Len() > 0 )
    {
        BOOL bExists = FALSE;
        bIsReadonly = IsReadOnly( rMainURL, &bExists );

        if( !bExists )  // new empty dictionary
        {
            bNeedEntries = FALSE;
            //! create physical representation of an **empty** dictionary
            //! that could be found by the dictionary-list implementation
            // (Note: empty dictionaries are not just empty files!)
            Save();
            bIsReadonly = IsReadOnly( rMainURL );   // will be FALSE if Save was succesfull
        }
    }
    else
    {
        bNeedEntries = FALSE;
    }
}


ConvDic::~ConvDic()
{
}


void ConvDic::Load()
{
    DBG_ASSERT( !bIsModified, "dictionary is modified. Really do 'Load'?" );

    //!! prevent function from being called recursively via HasEntry, AddEntry
    bNeedEntries = FALSE;

    ConvDicXMLImport *pImport = new ConvDicXMLImport( this, aMainURL );
    //!! keep a first reference to ensure the lifetime of the object !!
    uno::Reference< XInterface > xRef( (document::XFilter *) pImport, UNO_QUERY );
    ReadThroughDic( aMainURL, *pImport );    // will implicitly add the entries

    bIsModified = FALSE;
}


void ConvDic::Save()
{
    DBG_ASSERT( !bNeedEntries, "saving while entries missing" );
    if (aMainURL.Len() == 0 || bNeedEntries)
        return;

    DBG_ASSERT(!INetURLObject( aMainURL ).HasError(), "invalid URL");
    SfxMedium   aMedium( aMainURL, STREAM_WRITE | STREAM_TRUNC | STREAM_SHARE_DENYALL,
                         FALSE );
    aMedium.CreateTempFile();   // use temp file to write to...

    SvStream *pStream = aMedium.GetOutStream();
    DBG_ASSERT( pStream, "output stream missing" );
    if (!pStream || pStream->GetError())
        return;
    uno::Reference< io::XOutputStream > xOut(
            new utl::OOutputStreamWrapper( *pStream ) );
    DBG_ASSERT( xOut.is(), "output stream missing" );

    // get XML writer
    uno::Reference< lang::XMultiServiceFactory > xServiceFactory(
            utl::getProcessServiceFactory() );
    uno::Reference< io::XActiveDataSource > xSaxWriter;
    if (xServiceFactory.is())
    {
        try
        {
            xSaxWriter = uno::Reference< io::XActiveDataSource >(
                    xServiceFactory->createInstance(
                    OUString::createFromAscii( "com.sun.star.xml.sax.Writer" ) ), UNO_QUERY );
        }
        catch (uno::Exception &)
        {
        }
    }
    DBG_ASSERT( xSaxWriter.is(), "can't instantiate XML writer" );

    if (xSaxWriter.is() && xOut.is())
    {
        // connect XML writer to output stream
        xSaxWriter->setOutputStream( xOut );

        // prepare arguments (prepend doc handler to given arguments)
        uno::Reference< xml::sax::XDocumentHandler > xDocHandler( xSaxWriter, UNO_QUERY );

        ConvDicXMLExport *pExport = new ConvDicXMLExport( *this, aMainURL, xDocHandler );
        //!! keep a first(!) reference until everything is done to
        //!! ensure the proper lifetime of the object
        uno::Reference< document::XFilter > aRef( (document::XFilter *) pExport );
        sal_Bool bRet = pExport->Export( aMedium );     // write entries to file
        DBG_ASSERT( !pStream->GetError(), "I/O error while writing to stream" );
        if (bRet)
        {
            // flush file, close it and release any lock
            aMedium.Close();
            aMedium.Commit();
            bIsModified = FALSE;
        }
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


BOOL ConvDic::HasEntry( const OUString &rLeftText, const OUString &rRightText )
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

    bIsModified = TRUE;
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

    bIsModified = TRUE;
    bMaxCharCountIsValid = FALSE;
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
    bNeedEntries    = FALSE;
    bIsModified     = TRUE;
    nMaxLeftCharCount       = 0;
    nMaxRightCharCount      = 0;
    bMaxCharCountIsValid    = TRUE;
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

    INT32 nCount = 0;
    ConvMap::iterator aIt;
    for (aIt = aRange.first;  aIt != aRange.second;  ++aIt)
        ++nCount;

    uno::Sequence< OUString > aRes( nCount );
    OUString *pRes = aRes.getArray();
    INT32 i = 0;
    for (aIt = aRange.first;  aIt != aRange.second;  ++aIt)
        pRes[i++] = (*aIt).second;

    return aRes;
}


static BOOL lcl_SeqHasEntry(
    const OUString *pSeqStart,  // first element to check
    INT32 nToCheck,             // number of elements to check
    const OUString &rText)
{
    BOOL bRes = FALSE;
    if (pSeqStart && nToCheck > 0)
    {
        const OUString *pDone = pSeqStart + nToCheck;   // one behind last to check
        while (!bRes && pSeqStart != pDone)
        {
            if (*pSeqStart++ == rText)
                bRes = TRUE;
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
    INT32 nIdx = 0;
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

        bMaxCharCountIsValid = TRUE;
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
    if (rServiceName.equalsAscii( SN_CONV_DICTIONARY ))
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
    aSNS.getArray()[0] = A2OU( SN_CONV_DICTIONARY );
    return aSNS;
}

///////////////////////////////////////////////////////////////////////////


