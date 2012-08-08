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

#ifndef __FRAMEWORK_XML_STATUSBARDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_XML_STATUSBARDOCUMENTHANDLER_HXX_

#include <framework/statusbarconfiguration.hxx>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <threadhelp/threadhelpbase.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>

#include <boost/unordered_map.hpp>
#include <stdtypes.h>
#include <framework/fwedllapi.h>

namespace framework{

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.

class FWE_DLLPUBLIC OReadStatusBarDocumentHandler : private ThreadHelpBase, // Struct for right initalization of lock member! Must be first of baseclasses.
                                        public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XDocumentHandler >
{
    public:
        enum StatusBar_XML_Entry
        {
            SB_ELEMENT_STATUSBAR,
            SB_ELEMENT_STATUSBARITEM,
            SB_ATTRIBUTE_URL,
            SB_ATTRIBUTE_ALIGN,
            SB_ATTRIBUTE_STYLE,
            SB_ATTRIBUTE_AUTOSIZE,
            SB_ATTRIBUTE_OWNERDRAW,
            SB_ATTRIBUTE_WIDTH,
            SB_ATTRIBUTE_OFFSET,
            SB_ATTRIBUTE_HELPURL,
            SB_XML_ENTRY_COUNT
        };

        enum StatusBar_XML_Namespace
        {
            SB_NS_STATUSBAR,
            SB_NS_XLINK,
            SB_XML_NAMESPACES_COUNT
        };

        OReadStatusBarDocumentHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& aStatusBarItems );
        virtual ~OReadStatusBarDocumentHandler();

        // XDocumentHandler
        virtual void SAL_CALL startDocument(void)
        throw ( ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL endDocument(void)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL startElement(
            const rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > &xAttribs)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL endElement(const rtl::OUString& aName)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL characters(const rtl::OUString& aChars)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL ignorableWhitespace(const rtl::OUString& aWhitespaces)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL processingInstruction(const rtl::OUString& aTarget,
                                                    const rtl::OUString& aData)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setDocumentLocator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > &xLocator)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

    private:
        ::rtl::OUString getErrorLineString();

        class StatusBarHashMap : public ::boost::unordered_map< ::rtl::OUString             ,
                                                         StatusBar_XML_Entry            ,
                                                         OUStringHashCode               ,
                                                         ::std::equal_to< ::rtl::OUString > >
        {
            public:
                inline void free()
                {
                    StatusBarHashMap().swap( *this );
                }
        };

        sal_Bool                                                                            m_bStatusBarStartFound;
        sal_Bool                                                                            m_bStatusBarEndFound;
        sal_Bool                                                                            m_bStatusBarItemStartFound;
        StatusBarHashMap                                                                    m_aStatusBarMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >    m_aStatusBarItems;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >            m_xLocator;
};

class FWE_DLLPUBLIC OWriteStatusBarDocumentHandler : private ThreadHelpBase // Struct for right initalization of lock member! Must be first of baseclasses.
{
    public:
        OWriteStatusBarDocumentHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rStatusBarItems,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& rWriteDocHandler );
        virtual ~OWriteStatusBarDocumentHandler();

        void WriteStatusBarDocument() throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

    protected:
        virtual void WriteStatusBarItem(
            const rtl::OUString& rCommandURL,
            const rtl::OUString& rHelpURL,
            sal_Int16            nOffset,
            sal_Int16            nStyle,
            sal_Int16            nWidth ) throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >       m_aStatusBarItems;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xWriteDocumentHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >      m_xEmptyList;
        ::rtl::OUString                                                                     m_aXMLStatusBarNS;
        ::rtl::OUString                                                                     m_aXMLXlinkNS;
        ::rtl::OUString                                                                     m_aAttributeType;
        ::rtl::OUString                                                                     m_aAttributeURL;
};

} // namespace framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
