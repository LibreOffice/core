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
//This file is about the conversion of the UOF v2.0 and ODF document format from CS2C 20120610.
#ifndef _UOF2_SPLIT_HELPER_HXX_
#define _UOF2_SPLIT_HELPER_HXX_

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <vector>
#include <map>


#include "uof2storage.hxx"

struct NamespaceValue{
    ::rtl::OUString aType;
    ::rtl::OUString aValue;
    NamespaceValue( ::rtl::OUString & Type, ::rtl::OUString & Value ): aType(Type), aValue(Value) {}
    sal_Bool operator () ( ::rtl::OUString & Type, ::rtl::OUString & Value )
    { return ( aType == Type ) && ( aValue == Value ); }
};

namespace XSLT{

class UOF2SplitHandler : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XDocumentHandler >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler >  m_xHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > & m_rStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& m_rMultiFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >  m_xOutputStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  m_rInputStream;

public :

    UOF2SplitHandler(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rFactory,
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rStream );

    ~UOF2SplitHandler();

    // XDocumentHandler
    virtual void SAL_CALL startDocument() throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endDocument() throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL startElement(const ::rtl::OUString& qName , const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& attriblist)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endElement(const ::rtl::OUString& qName )
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL characters(const ::rtl::OUString& rStr )
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL ignorableWhitespace(const ::rtl::OUString& str)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL processingInstruction(const ::rtl::OUString& str, const ::rtl::OUString& str2)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& doclocator)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

private :
    void _Init();
    void Write( ::rtl::OUString & rOutFile , sal_Bool bStart ) throw( ::com::sun::star::uno::RuntimeException );
    void namespaceMap( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& attriblist );

private :
    UOF2Storage * m_pStore;
    StorageRef  m_pMainStore;
    StorageRef  m_pCurStore;
    ::rtl::OUString aOutFileName;
    ::std::map< ::rtl::OUString, NamespaceValue > aNamespaceMap;
    ::std::vector< ::rtl::OUString > aUsedNamespace;
    sal_Bool    m_bIsStart;  // start a sub file


};
} // end namespace
#endif
