/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 **************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

//This file is about the conversion of the UOF v2.0 and ODF document format
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/storagehelper.hxx>
#include <comphelper/attributelist.hxx>
#include <xmloff/attrlist.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>


#include "uof2splithandler.hxx"

using namespace XSLT;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::beans;
using namespace ::rtl;



UOF2SplitHandler::UOF2SplitHandler(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rMultiFactory,
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rStream )
: m_rStream( rStream )
, m_rMultiFactory( rMultiFactory )
, m_bIsStart( sal_False )
{
    _Init();
}

void UOF2SplitHandler::_Init()
{
    if ( m_rMultiFactory.is() && m_rStream.is() )
    {
        try {
            m_pStore = new UOF2Storage( m_rMultiFactory, m_rStream );
            if ( m_pStore != NULL )
                m_pMainStore = m_pStore->getMainStorageRef();

            m_xHandler = Reference< XExtendedDocumentHandler >( m_rMultiFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Writer")), UNO_QUERY );
            Reference< XActiveDataSource > xSource( m_xHandler, UNO_QUERY );
            xSource->setOutputStream( m_rStream->getOutputStream() );
        }
        catch( Exception& e)
        {
            OSL_ENSURE(0, ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        }
    }
}

UOF2SplitHandler::~UOF2SplitHandler()
{
    delete m_pStore;
}

void UOF2SplitHandler::startDocument() throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    m_xHandler->startDocument();
}

void UOF2SplitHandler::endDocument() throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    m_xHandler->endDocument();
}

void UOF2SplitHandler::startElement(const ::rtl::OUString& qName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& AttrList )
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    if ( qName.equalsAscii("pzip:entry") )
    {
        namespaceMap( AttrList );
    }
    else if ( qName.equalsAscii("pzip:target") )
    {
        aOutFileName = AttrList->getValueByName(::rtl::OUString::createFromAscii("pzip:path"));
        m_bIsStart = sal_True;
        Write( aOutFileName , m_bIsStart );
    }
    else
    {
        if ( m_bIsStart )  // start a sub file
        {
            // insert namespace declaration for the root element in each sub file
            ::comphelper::AttributeList * aTmpAtt = new ::comphelper::AttributeList();
            if ( aTmpAtt != NULL )
            {
                typedef ::std::map< OUString, NamespaceValue >::iterator NSItr;
                for( NSItr it = aNamespaceMap.begin(); it != aNamespaceMap.end(); it++ )
                {
                    aTmpAtt->AddAttribute( it->first, it->second.aType, it->second.aValue );
                }
                aTmpAtt->AppendAttributeList( AttrList );
                m_bIsStart = sal_False;
                const Reference< XAttributeList > xnewAttr( aTmpAtt );
                m_xHandler->startElement( qName, xnewAttr );
            }
        }
        else
            m_xHandler->startElement( qName, AttrList );
    }
}

void UOF2SplitHandler::endElement(const ::rtl::OUString& qName )
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    if ( qName.equalsAscii("pzip:entry") )
    {
        m_pMainStore->commit();
    }
    else if ( qName.equalsAscii("pzip:target" ) )
    {
        Write( aOutFileName, sal_False );
    }
    else
    {
        m_xHandler->endElement( qName );
    }
}

void UOF2SplitHandler::characters(const ::rtl::OUString& rStr )
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    m_xHandler->characters( rStr );
}

void UOF2SplitHandler::ignorableWhitespace(const ::rtl::OUString& str)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    m_xHandler->ignorableWhitespace( str );
}

void UOF2SplitHandler::processingInstruction(const ::rtl::OUString& /*str*/, const ::rtl::OUString& /*str2*/)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}

void UOF2SplitHandler::setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >&/* doclocator*/)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}

/* ----
    create substream and write sub file when starting a <pzip:target> element
    close substream when ending a <pzip:target> element
---- */
void UOF2SplitHandler::Write( OUString & rFileName , sal_Bool bStart ) throw( RuntimeException )
{
    if ( bStart )
    {
        try {
            OUString sPath;
            sal_Int32 nIndex = rFileName.lastIndexOf( sal_Unicode('/') );
            if ( nIndex != -1 )
            {
                OUString aTmp = rFileName;
                sPath = aTmp.copy(0, nIndex);
                rFileName = aTmp.copy( nIndex+1 );
                m_pCurStore = m_pMainStore->openSubStorage( sPath, sal_True );

                m_xOutputStream = m_pCurStore->openOutputStream( rFileName );
            }
            else
                m_xOutputStream = m_pMainStore->openOutputStream( rFileName );

            if ( !m_xHandler.is() )
            {
                m_xHandler = Reference< XExtendedDocumentHandler >( m_rMultiFactory->createInstance(
                    OUString::createFromAscii("com.sun.star.xml.sax.Writer")), UNO_QUERY );
            }
            Reference< XActiveDataSource > xSource( m_xHandler, UNO_QUERY );
            xSource->setOutputStream( m_xOutputStream );

            if ( m_xHandler.is() )
                m_xHandler->startDocument();
            else
            {
                SAXException except;
                except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "startElement called before startDocument" ));
                throw except;
            }
        }
        catch( ::com::sun::star::uno::Exception& e )
        {
            OSL_ENSURE(0, ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        }
    }
    else
    {
        if ( m_xHandler.is() )
            m_xHandler->endDocument();
    }
}

/* -----
    collect namesapce declaration of the <pzip:entry> element
---- */
void UOF2SplitHandler::namespaceMap( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& AttrList )
{
    sal_Int16 aAttrCount = AttrList->getLength();
    for ( sal_Int16 i = 0; i< aAttrCount; i++ )
    {
        OUString aName = AttrList->getNameByIndex( i );
        sal_Int32 nToken = aName.indexOf( sal_Unicode(':') );
        OUString aPrefix = aName.copy( 0, nToken );
        if ( aPrefix.equalsAscii("xmlns") )
        {
            if ( aNamespaceMap.find( aName ) == aNamespaceMap.end() )
            {
                OUString sType = AttrList->getTypeByIndex( i );
                OUString sValue = AttrList->getValueByIndex( i );
                aNamespaceMap.insert( ::std::map< OUString, NamespaceValue >::value_type(aName , NamespaceValue( sType , sValue )) );
            }
        }
    }
}
