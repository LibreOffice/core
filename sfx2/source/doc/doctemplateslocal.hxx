/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doctemplateslocal.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 13:27:47 $
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

#ifndef _SFX_DOCTEMPLATESLOCAL_HXX
#define _SFX_DOCTEMPLATESLOCAL_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XDUCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_STRINGPAIR_HPP_
#include <com/sun/star/beans/StringPair.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif


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

