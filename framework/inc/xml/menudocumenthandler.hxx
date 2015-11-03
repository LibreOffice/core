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

#ifndef INCLUDED_FRAMEWORK_INC_XML_MENUDOCUMENTHANDLER_HXX
#define INCLUDED_FRAMEWORK_INC_XML_MENUDOCUMENTHANDLER_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <framework/fwedllapi.h>

namespace framework{

class FWE_DLLPUBLIC ReadMenuDocumentHandlerBase :
                                    public ::cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{
    public:
        ReadMenuDocumentHandlerBase();
        virtual ~ReadMenuDocumentHandlerBase();

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override = 0;

        virtual void SAL_CALL endDocument()
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override = 0;

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList > &xAttribs)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override = 0;

        virtual void SAL_CALL endElement(const OUString& aName)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override = 0;

        virtual void SAL_CALL characters(const OUString& aChars)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override = 0;

        virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                    const OUString& aData)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setDocumentLocator(
            const css::uno::Reference< css::xml::sax::XLocator > &xLocator)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

    protected:
        OUString getErrorLineString();

        css::uno::Reference< css::xml::sax::XLocator > m_xLocator;
        css::uno::Reference< css::xml::sax::XDocumentHandler> m_xReader;
        void initPropertyCommon( css::uno::Sequence< css::beans::PropertyValue > &rProps,
                                 const OUString &rCommandURL, const OUString &rHelpId,
                                 const OUString &rLabel, sal_Int16 nItemStyleBits );
    private:
        OUString m_aType;
        OUString m_aLabel;
        OUString m_aContainer;
        OUString m_aHelpURL;
        OUString m_aCommandURL;
        OUString m_aStyle;
};

class FWE_DLLPUBLIC OReadMenuDocumentHandler : public ReadMenuDocumentHandlerBase
{
    public:
        OReadMenuDocumentHandler(
            const css::uno::Reference< css::container::XIndexContainer >& rItemContainer );
        virtual ~OReadMenuDocumentHandler();

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endDocument()
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList > &xAttribs)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endElement(const OUString& aName)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL characters(const OUString& aChars)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

    private:
          int       m_nElementDepth;
          enum class ReaderMode { None, MenuBar, MenuPopup };
          ReaderMode m_eReaderMode;
          css::uno::Reference< css::container::XIndexContainer > m_xMenuBarContainer;
          css::uno::Reference< css::lang::XSingleComponentFactory > m_xContainerFactory;
};  // OReadMenuDocumentHandler

class FWE_DLLPUBLIC OReadMenuBarHandler : public ReadMenuDocumentHandlerBase
{
    public:
        OReadMenuBarHandler(
            const css::uno::Reference< css::container::XIndexContainer >& rMenuBarContainer,
            const css::uno::Reference< css::lang::XSingleComponentFactory >& rContainerFactory );
        virtual ~OReadMenuBarHandler();

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endDocument()
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList > &xAttribs)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endElement(const OUString& aName)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL characters(const OUString& aChars)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

    private:
        int         m_nElementDepth;
        bool    m_bMenuMode;
        css::uno::Reference< css::container::XIndexContainer > m_xMenuBarContainer;
        css::uno::Reference< css::lang::XSingleComponentFactory > m_xContainerFactory;
};  // OReadMenuBarHandler

class FWE_DLLPUBLIC OReadMenuHandler : public ReadMenuDocumentHandlerBase
{
    public:
        OReadMenuHandler( const css::uno::Reference< css::container::XIndexContainer >& rMenuContainer,
                          const css::uno::Reference< css::lang::XSingleComponentFactory >& rContainerFactory );
        virtual ~OReadMenuHandler();

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endDocument()
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList > &xAttribs)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endElement(const OUString& aName)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL characters(const OUString& aChars)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

    private:
        int                 m_nElementDepth;
        bool            m_bMenuPopupMode;
        css::uno::Reference< css::container::XIndexContainer > m_xMenuContainer;
        css::uno::Reference< css::lang::XSingleComponentFactory > m_xContainerFactory;
}; // OReadMenuHandler

class FWE_DLLPUBLIC OReadMenuPopupHandler : public ReadMenuDocumentHandlerBase
{
    public:
        OReadMenuPopupHandler( const css::uno::Reference< css::container::XIndexContainer >& rMenuContainer,
                               const css::uno::Reference< css::lang::XSingleComponentFactory >& rContainerFactory );
        virtual ~OReadMenuPopupHandler();

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw ( css::xml::sax::SAXException,
                    css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endDocument()
            throw ( css::xml::sax::SAXException,
                    css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const css::uno::Reference<
                css::xml::sax::XAttributeList > &xAttribs)
            throw ( css::xml::sax::SAXException,
                    css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endElement(const OUString& aName)
            throw ( css::xml::sax::SAXException,
                    css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL characters(const OUString& aChars)
            throw ( css::xml::sax::SAXException,
                    css::uno::RuntimeException, std::exception ) override;

    private:
        enum NextElementClose { ELEM_CLOSE_NONE, ELEM_CLOSE_MENUITEM, ELEM_CLOSE_MENUSEPARATOR };

        int                                                        m_nElementDepth;
        bool                                                       m_bMenuMode;
        css::uno::Reference< css::container::XIndexContainer >     m_xMenuContainer;
        css::uno::Reference< css::lang::XSingleComponentFactory >  m_xContainerFactory;
        css::uno::Reference< css::uno::XComponentContext >         m_xComponentContext;
        NextElementClose                                           m_nNextElementExpected;
}; // OReadMenuPopupHandler

class FWE_DLLPUBLIC OWriteMenuDocumentHandler
{
    public:
        OWriteMenuDocumentHandler(
            const css::uno::Reference< css::container::XIndexAccess >& rMenuBarContainer,
            const css::uno::Reference< css::xml::sax::XDocumentHandler >& rDocumentHandler,
            bool bIsMenuBar );
        virtual ~OWriteMenuDocumentHandler();

        void WriteMenuDocument() throw
            ( css::xml::sax::SAXException, css::uno::RuntimeException );
    protected:
        void WriteMenu( const css::uno::Reference< css::container::XIndexAccess >& rSubMenuContainer ) throw
            ( css::xml::sax::SAXException, css::uno::RuntimeException );

        void WriteMenuItem( const OUString& aCommandURL, const OUString& aLabel, const OUString& aHelpURL, sal_Int16 nStyle = 0 );
        void WriteMenuSeparator();

        css::uno::Reference< css::container::XIndexAccess > m_xMenuBarContainer;
        css::uno::Reference< css::xml::sax::XDocumentHandler > m_xWriteDocumentHandler;
        css::uno::Reference< css::xml::sax::XAttributeList > m_xEmptyList;
        OUString m_aAttributeType;
        bool m_bIsMenuBar;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_XML_MENUDOCUMENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
