/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ofopxmlhelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 13:51:50 $
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

#ifndef _COMPHELPER_OFOPXMLHELPER_HXX
#define _COMPHELPER_OFOPXMLHELPER_HXX

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

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include <comphelper/comphelperdllapi.h>
#endif


namespace comphelper
{

// this helper class is designed to allow to parse ContentType- and Relationship-related information from OfficeOpenXML format
class COMPHELPER_DLLPUBLIC OFOPXMLHelper : public cppu::WeakImplHelper1 < com::sun::star::xml::sax::XDocumentHandler >
{
    sal_uInt16 m_nFormat; // which format to parse

    // Relations info related strings
    ::rtl::OUString m_aRelListElement;
    ::rtl::OUString m_aRelElement;
    ::rtl::OUString m_aIDAttr;
    ::rtl::OUString m_aTypeAttr;
    ::rtl::OUString m_aTargetModeAttr;
    ::rtl::OUString m_aTargetAttr;

    // ContentType related strings
    ::rtl::OUString m_aTypesElement;
    ::rtl::OUString m_aDefaultElement;
    ::rtl::OUString m_aOverrideElement;
    ::rtl::OUString m_aExtensionAttr;
    ::rtl::OUString m_aPartNameAttr;
    ::rtl::OUString m_aContentTypeAttr;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > m_aResultSeq;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aElementsSeq; // stack of elements being parsed

    OFOPXMLHelper( sal_uInt16 nFormat ); // must not be created directly
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > GetParsingResult();

    static COMPHELPER_DLLPRIVATE ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > SAL_CALL ReadSequence_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream, const ::rtl::OUString& aStringID, sal_uInt16 nFormat, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
    throw( ::com::sun::star::uno::Exception );

public:
    ~OFOPXMLHelper();

    // returns sequence of elements, where each element is described by sequence of tags,
    // where each tag is described by StringPair ( First - name, Second - value )
    // the first tag of each element sequence must be "Id"
    static
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >
    SAL_CALL
    ReadRelationsInfoSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
        const ::rtl::OUString aStreamName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
            throw( ::com::sun::star::uno::Exception );

    // returns sequence containing two entries of type sequence<StringPair>
    // the first sequence describes "Default" elements, where each element is described
    // by StringPair object ( First - Extension, Second - ContentType )
    // the second sequence describes "Override" elements, where each element is described
    // by StringPair object ( First - PartName, Second - ContentType )
    static
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >
    SAL_CALL
    ReadContentTypeSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
            throw( ::com::sun::star::uno::Exception );

    // writes sequence of elements, where each element is described by sequence of tags,
    // where each tag is described by StringPair ( First - name, Second - value )
    // the first tag of each element sequence must be "Id"
    static
    void SAL_CALL WriteRelationsInfoSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutStream,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >& aSequence,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
            throw( ::com::sun::star::uno::Exception );

    // writes two entries of type sequence<StringPair>
    // the first sequence describes "Default" elements, where each element is described
    // by StringPair object ( First - Extension, Second - ContentType )
    // the second sequence describes "Override" elements, where each element is described
    // by StringPair object ( First - PartName, Second - ContentType )
    static
    void SAL_CALL WriteContentSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutStream,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aDefaultsSequence,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aOverridesSequence,
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

} // namespace comphelper

#endif

