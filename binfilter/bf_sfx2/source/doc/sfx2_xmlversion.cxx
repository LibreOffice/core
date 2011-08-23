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

#include <tools/debug.hxx>

#include <unotools/streamwrap.hxx>

#include <xmlversion.hxx>

#include <bf_xmloff/xmlmetae.hxx>

#include <bf_xmloff/xmlkywd.hxx>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <com/sun/star/util/DateTime.hpp>

#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

#include <docfile.hxx>

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 

#include <bf_xmloff/nmspmap.hxx>

#include <bf_xmloff/xmlnmspe.hxx>

namespace binfilter {

using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

using rtl::OUString;

// ------------------------------------------------------------------------

sal_Char __FAR_DATA XMLN_VERSIONSLIST[] = "VersionList.xml";

// ------------------------------------------------------------------------

// #110680#
/*?*/ SfxXMLVersListImport_Impl::SfxXMLVersListImport_Impl( 
/*?*/ 	const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
/*?*/ 	SfxVersionTableDtor *pVersions )
/*?*/ :	SvXMLImport(xServiceFactory),
/*?*/ 	mpVersions( pVersions )
/*?*/ {
/*?*/     GetNamespaceMap().AddAtIndex( XML_NAMESPACE_FRAMEWORK_IDX, sXML_np_versions_list,
/*?*/                                   sXML_n_versions_list, XML_NAMESPACE_FRAMEWORK );
/*?*/ }

// ------------------------------------------------------------------------
/*?*/ SfxXMLVersListImport_Impl::~SfxXMLVersListImport_Impl( void ) throw()
/*?*/ {}

// ------------------------------------------------------------------------
/*?*/ SvXMLImportContext *SfxXMLVersListImport_Impl::CreateContext(
/*?*/         sal_uInt16 nPrefix,
/*?*/         const OUString& rLocalName,
/*?*/         const Reference< XAttributeList > & xAttrList )
/*?*/ {
/*?*/     SvXMLImportContext *pContext = 0;
/*?*/ 
/*?*/     if ( XML_NAMESPACE_FRAMEWORK == nPrefix &&
/*?*/         rLocalName.compareToAscii( sXML_version_list ) == 0 )
/*?*/     {
/*?*/         pContext = new SfxXMLVersListContext_Impl( *this, nPrefix, rLocalName, xAttrList );
/*?*/     }
/*?*/     else
/*?*/     {
/*?*/         pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
/*?*/     }
/*?*/ 
/*?*/     return pContext;
/*?*/ }
/*?*/ 

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

/*?*/ SfxXMLVersListContext_Impl::SfxXMLVersListContext_Impl( SfxXMLVersListImport_Impl& rImport,
/*?*/                                         sal_uInt16 nPrefix,
/*?*/                                         const OUString& rLocalName,
/*?*/                                         const Reference< XAttributeList > & xAttrList )
/*?*/     : rLocalRef( rImport )
/*?*/     , SvXMLImportContext( rImport, nPrefix, rLocalName )
/*?*/ {
/*?*/ }

// ------------------------------------------------------------------------
/*?*/ SfxXMLVersListContext_Impl::~SfxXMLVersListContext_Impl( void )
/*?*/ {}

// ------------------------------------------------------------------------
/*?*/ SvXMLImportContext *SfxXMLVersListContext_Impl::CreateChildContext( sal_uInt16 nPrefix,
/*?*/                                         const OUString& rLocalName,
/*?*/                                         const Reference< XAttributeList > & xAttrList )
/*?*/ {
/*?*/     SvXMLImportContext *pContext = 0;
/*?*/ 
/*?*/     if ( nPrefix == XML_NAMESPACE_FRAMEWORK &&
/*?*/          rLocalName.compareToAscii( sXML_version_entry ) == 0)
/*?*/     {
/*?*/         pContext = new SfxXMLVersionContext_Impl( rLocalRef, nPrefix, rLocalName, xAttrList );
/*?*/     }
/*?*/     else
/*?*/     {
/*?*/         pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName );
/*?*/     }
/*?*/ 
/*?*/     return pContext;
/*?*/ }

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

/*?*/ SfxXMLVersionContext_Impl::SfxXMLVersionContext_Impl( SfxXMLVersListImport_Impl& rImport,
/*?*/                                         sal_uInt16 nPref,
/*?*/                                         const OUString& rLocalName,
/*?*/                                         const Reference< XAttributeList > & xAttrList )
/*?*/     : rLocalRef( rImport )
/*?*/     , SvXMLImportContext( rImport, nPref, rLocalName )
/*?*/ {
/*?*/     sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
/*?*/ 
/*?*/     if ( !nAttrCount )
/*?*/         return;
/*?*/ 
/*?*/     SfxVersionInfo *pInfo = new SfxVersionInfo;
/*?*/ 
/*?*/     for ( sal_Int16 i=0; i < nAttrCount; i++ )
/*?*/     {
/*?*/         OUString        aLocalName;
/*?*/         const OUString& rAttrName   = xAttrList->getNameByIndex( i );
/*?*/         sal_uInt16      nPrefix     = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
/*?*/ 
/*?*/         if ( XML_NAMESPACE_FRAMEWORK == nPrefix )
/*?*/         {
/*?*/             if ( aLocalName.compareToAscii( sXML_title ) == 0 )
/*?*/             {
/*?*/                 const OUString& rAttrValue = xAttrList->getValueByIndex( i );
/*?*/                 pInfo->aName = rAttrValue;
/*?*/             }
/*?*/             else if ( aLocalName.compareToAscii( sXML_comment ) == 0 )
/*?*/             {
/*?*/                 const OUString& rAttrValue = xAttrList->getValueByIndex( i );
/*?*/                 pInfo->aComment = rAttrValue;
/*?*/             }
/*?*/             else if ( aLocalName.compareToAscii( sXML_creator ) == 0 )
/*?*/             {
/*?*/                 const OUString& rAttrValue = xAttrList->getValueByIndex( i );
/*?*/                 pInfo->aCreateStamp.SetName( rAttrValue );
/*?*/             }
/*?*/         }
/*?*/         else if ( ( XML_NAMESPACE_DC == nPrefix ) &&
/*?*/                   ( aLocalName.compareToAscii( sXML_date_time ) == 0 ) )
/*?*/         {
/*?*/             const OUString& rAttrValue = xAttrList->getValueByIndex( i );
/*?*/             DateTime aTime;
/*?*/             if ( ParseISODateTimeString( rAttrValue, aTime ) )
/*?*/                 pInfo->aCreateStamp.SetTime( aTime );
/*?*/         }
/*?*/     }
/*?*/ 
/*?*/     SfxVersionTableDtor* pVersion = rLocalRef.GetList();
/*?*/     pVersion->Insert( pInfo, LIST_APPEND );
/*?*/ }


// ------------------------------------------------------------------------
/*?*/ SfxXMLVersionContext_Impl::~SfxXMLVersionContext_Impl( void )
/*?*/ {}

// ------------------------------------------------------------------------
// static
/*?*/ sal_Bool SfxXMLVersionContext_Impl::ParseISODateTimeString(
/*?*/                                 const ::rtl::OUString& rString,
/*?*/                                 DateTime& rDateTime )
/*?*/ {
/*?*/     sal_Bool bSuccess = sal_True;
/*?*/ 
/*?*/     OUString aDateStr, aTimeStr;
/*?*/     sal_Int32 nPos = rString.indexOf( (sal_Unicode) 'T' );
/*?*/     if ( nPos >= 0 )
/*?*/     {
/*?*/         aDateStr = rString.copy( 0, nPos );
/*?*/         aTimeStr = rString.copy( nPos + 1 );
/*?*/     }
/*?*/     else
/*?*/         aDateStr = rString;         // no separator: only date part
/*?*/ 
/*?*/     sal_Int32 nYear  = 0;
/*?*/     sal_Int32 nMonth = 1;
/*?*/     sal_Int32 nDay   = 1;
/*?*/     sal_Int32 nHour  = 0;
/*?*/     sal_Int32 nMin   = 0;
/*?*/     sal_Int32 nSec   = 0;
/*?*/ 
/*?*/     const sal_Unicode* pStr = aDateStr.getStr();
/*?*/     sal_Int32 nDateTokens = 1;
/*?*/     while ( *pStr )
/*?*/     {
/*?*/         if ( *pStr == '-' )
/*?*/             nDateTokens++;
/*?*/         pStr++;
/*?*/     }
/*?*/     if ( nDateTokens > 3 || aDateStr.getLength() == 0 )
/*?*/         bSuccess = sal_False;
/*?*/     else
/*?*/     {
/*?*/         sal_Int32 n = 0;
/*?*/         nYear = aDateStr.getToken( 0, '-', n ).toInt32();
/*?*/         if ( nYear > 9999 )
/*?*/             bSuccess = sal_False;
/*?*/         else if ( nDateTokens >= 2 )
/*?*/         {
/*?*/             nMonth = aDateStr.getToken( 0, '-', n ).toInt32();
/*?*/             if ( nMonth > 12 )
/*?*/                 bSuccess = sal_False;
/*?*/             else if ( nDateTokens >= 3 )
/*?*/             {
/*?*/                 nDay = aDateStr.getToken( 0, '-', n ).toInt32();
/*?*/                 if ( nDay > 31 )
/*?*/                     bSuccess = sal_False;
/*?*/             }
/*?*/         }
/*?*/     }
/*?*/ 
/*?*/     if ( bSuccess && aTimeStr.getLength() > 0 )         // time is optional
/*?*/     {
/*?*/         pStr = aTimeStr.getStr();
/*?*/         sal_Int32 nTimeTokens = 1;
/*?*/         while ( *pStr )
/*?*/         {
/*?*/             if ( *pStr == ':' )
/*?*/                 nTimeTokens++;
/*?*/             pStr++;
/*?*/         }
/*?*/         if ( nTimeTokens > 3 )
/*?*/             bSuccess = sal_False;
/*?*/         else
/*?*/         {
/*?*/             sal_Int32 n = 0;
/*?*/             nHour = aTimeStr.getToken( 0, ':', n ).toInt32();
/*?*/             if ( nHour > 23 )
/*?*/                 bSuccess = sal_False;
/*?*/             else if ( nTimeTokens >= 2 )
/*?*/             {
/*?*/                 nMin = aTimeStr.getToken( 0, ':', n ).toInt32();
/*?*/                 if ( nMin > 59 )
/*?*/                     bSuccess = sal_False;
/*?*/                 else if ( nTimeTokens >= 3 )
/*?*/                 {
/*?*/                     nSec = aTimeStr.getToken( 0, ':', n ).toInt32();
/*?*/                     if ( nSec > 59 )
/*?*/                         bSuccess = sal_False;
/*?*/                 }
/*?*/             }
/*?*/         }
/*?*/     }
/*?*/ 
/*?*/     if ( bSuccess )
/*?*/         rDateTime = DateTime( Date( (USHORT)nDay, (USHORT)nMonth, (USHORT)nYear ),
/*?*/                               Time( nHour, nMin, nSec ) );
/*?*/ 
/*?*/     return bSuccess;
/*?*/ }

// ------------------------------------------------------------------------
/*N*/ sal_Bool SfxXMLVersList_Impl::ReadInfo( SvStorageRef xRoot, SfxVersionTableDtor *pList )
/*N*/ {
/*N*/     sal_Bool bRet = sal_False;
/*N*/ 
/*N*/     const OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( XMLN_VERSIONSLIST ) );
/*N*/ 
/*N*/     if ( xRoot->IsContained ( sDocName ) )
/*N*/     {
/*?*/         Reference< lang::XMultiServiceFactory > xServiceFactory =
/*?*/                 ::legacy_binfilters::getLegacyProcessServiceFactory();
/*?*/         DBG_ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );
/*?*/ 
/*?*/         InputSource aParserInput;
/*?*/         aParserInput.sSystemId = xRoot->GetName();
/*?*/ 
/*?*/         SvStorageStreamRef xDocStream = xRoot->OpenStream( sDocName, STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE );
/*?*/         xDocStream->Seek( 0L );
/*?*/         xDocStream->SetBufferSize( 16*1024 );
/*?*/         aParserInput.aInputStream = new ::utl::OInputStreamWrapper( *xDocStream );
/*?*/ 
/*?*/         // get parser
/*?*/         Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
/*?*/             OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
/*?*/         DBG_ASSERT( xXMLParser.is(),
/*?*/             "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
/*?*/ 
/*?*/         // get filter
/*?*/ 		// #110680#
/*?*/         // Reference< XDocumentHandler > xFilter = new SfxXMLVersListImport_Impl( pList );
/*?*/         Reference< XDocumentHandler > xFilter = new SfxXMLVersListImport_Impl( xServiceFactory, pList );
/*?*/ 
/*?*/         // connect parser and filter
/*?*/         Reference< XParser > xParser( xXMLParser, UNO_QUERY );
/*?*/         xParser->setDocumentHandler( xFilter );
/*?*/ 
/*?*/         // parse
/*?*/         try
/*?*/         {
/*?*/             xParser->parseStream( aParserInput );
/*?*/             bRet = sal_True;
/*?*/         }
/*?*/         catch( SAXParseException&  ) {}
/*?*/         catch( SAXException&  )      {}
/*?*/         catch( io::IOException& )    {}
/*?*/     }
/*N*/ 
/*N*/     return bRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
