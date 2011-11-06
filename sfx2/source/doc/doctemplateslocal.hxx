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
 *************************************************************/



#ifndef _SFX_DOCTEMPLATESLOCAL_HXX
#define _SFX_DOCTEMPLATESLOCAL_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XDUCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase1.hxx>


class DocTemplLocaleHelper : public cppu::WeakImplHelper1 < com::sun::star::xml::sax::XDocumentHandler >
{
    // Relations info related strings
    ::rtl::OUString m_aGroupListElement;
    ::rtl::OUString m_aGroupElement;
    ::rtl::OUString m_aNameAttr;
    ::rtl::OUString m_aUINameAttr;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > m_aResultSeq;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aElementsSeq; // stack of elements being parsed

    DocTemplLocaleHelper(); // must not be created directly
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > GetParsingResult();

    static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > SAL_CALL ReadLocalizationSequence_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream, const ::rtl::OUString& aStringID, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
    throw( ::com::sun::star::uno::Exception );

public:
    ~DocTemplLocaleHelper();

    // returns sequence of pairs ( GroupName, GroupUIName )
    static
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >
    ReadGroupLocalizationSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
            throw( ::com::sun::star::uno::Exception );

    // writes sequence of elements ( GroupName, GroupUIName )
    static
    void SAL_CALL WriteGroupLocalizationSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutStream,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aSequence,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
            throw( ::com::sun::star::uno::Exception );

    // XDocumentHandler
    virtual void SAL_CALL startDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};

#endif

