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

#ifndef __FRAMEWORK_XML_MENUDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_XML_MENUDOCUMENTHANDLER_HXX_

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <threadhelp/threadhelpbase.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <framework/fwedllapi.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

class FWE_DLLPUBLIC ReadMenuDocumentHandlerBase : public ThreadHelpBase,    // Struct for right initalization of mutex member! Must be first of baseclasses.
                                    public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XDocumentHandler >
{
    public:
        ReadMenuDocumentHandlerBase();
        virtual ~ReadMenuDocumentHandlerBase();

        // XDocumentHandler
        virtual void SAL_CALL startDocument(void)
        throw ( ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException ) = 0;

        virtual void SAL_CALL endDocument(void)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException ) = 0;

        virtual void SAL_CALL startElement(
            const rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > &xAttribs)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException ) = 0;

        virtual void SAL_CALL endElement(const rtl::OUString& aName)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException ) = 0;

        virtual void SAL_CALL characters(const rtl::OUString& aChars)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException ) = 0;

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

    protected:
        ::rtl::OUString getErrorLineString();

        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > m_xLocator;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler> m_xReader;
        void initPropertyCommon( com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &rProps,
                                 const rtl::OUString &rCommandURL, const rtl::OUString &rHelpId,
                                 const rtl::OUString &rLabel, sal_Int16 nItemStyleBits );
    private:
        rtl::OUString m_aType;
        rtl::OUString m_aLabel;
        rtl::OUString m_aContainer;
        rtl::OUString m_aHelpURL;
        rtl::OUString m_aCommandURL;
        rtl::OUString m_aStyle;
        ::com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > m_aItemProp;
};


class FWE_DLLPUBLIC OReadMenuDocumentHandler : public ReadMenuDocumentHandlerBase
{
    public:
        OReadMenuDocumentHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >& rItemContainer );
        virtual ~OReadMenuDocumentHandler();

        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& getServiceFactory();

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

    private:
          int       m_nElementDepth;
          sal_Bool  m_bMenuBarMode;
          com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > m_xMenuBarContainer;
          com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory > m_xContainerFactory;
          const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& mxServiceFactory;
};  // OReadMenuDocumentHandler


class FWE_DLLPUBLIC OReadMenuBarHandler : public ReadMenuDocumentHandlerBase
{
    public:
        OReadMenuBarHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rMenuBarContainer,
            const com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >& rContainerFactory );
        virtual ~OReadMenuBarHandler();

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

    private:
        int         m_nElementDepth;
        sal_Bool    m_bMenuMode;
        com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > m_xMenuBarContainer;
        com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory > m_xContainerFactory;

        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& mxServiceFactory;
};  // OReadMenuBarHandler


class FWE_DLLPUBLIC OReadMenuHandler : public ReadMenuDocumentHandlerBase
{
    public:
        OReadMenuHandler( const com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rMenuContainer,
                          const com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >& rContainerFactory );
        virtual ~OReadMenuHandler();

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

    private:
        int                 m_nElementDepth;
        sal_Bool            m_bMenuPopupMode;
        com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > m_xMenuContainer;
        com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory > m_xContainerFactory;
}; // OReadMenuHandler


class FWE_DLLPUBLIC OReadMenuPopupHandler : public ReadMenuDocumentHandlerBase
{
    public:
        OReadMenuPopupHandler( const com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rMenuContainer,
                               const com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >& rContainerFactory );
        virtual ~OReadMenuPopupHandler();

        // XDocumentHandler
        virtual void SAL_CALL startDocument(void)
            throw ( ::com::sun::star::xml::sax::SAXException,
                    ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL endDocument(void)
            throw ( ::com::sun::star::xml::sax::SAXException,
                    ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL startElement(
            const rtl::OUString& aName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > &xAttribs)
            throw ( ::com::sun::star::xml::sax::SAXException,
                    ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL endElement(const rtl::OUString& aName)
            throw ( ::com::sun::star::xml::sax::SAXException,
                    ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL characters(const rtl::OUString& aChars)
            throw ( ::com::sun::star::xml::sax::SAXException,
                    ::com::sun::star::uno::RuntimeException );

    private:
        enum NextElementClose { ELEM_CLOSE_NONE, ELEM_CLOSE_MENUITEM, ELEM_CLOSE_MENUSEPARATOR };

        int                 m_nElementDepth;
        sal_Bool            m_bMenuMode;
        com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > m_xMenuContainer;
        com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory > m_xContainerFactory;
        com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >     m_xComponentContext;
        NextElementClose    m_nNextElementExpected;
}; // OReadMenuPopupHandler


class FWE_DLLPUBLIC OWriteMenuDocumentHandler
{
    public:
        OWriteMenuDocumentHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rMenuBarContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& rDocumentHandler );
        virtual ~OWriteMenuDocumentHandler();

        void WriteMenuDocument() throw
            ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    protected:
        virtual void WriteMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rSubMenuContainer ) throw
            ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

        virtual void WriteMenuItem( const rtl::OUString& aCommandURL, const rtl::OUString& aLabel, const rtl::OUString& aHelpURL, sal_Int16 nStyle = 0 );
        virtual void WriteMenuSeparator();

        com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > m_xMenuBarContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > m_xWriteDocumentHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > m_xEmptyList;
        ::rtl::OUString m_aAttributeType;
};

} // namespace framework

#endif  // #ifndef __FRAMEWORK_XML_MENUDOCUMENTHANDLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
