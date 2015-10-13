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

#ifndef INCLUDED_FRAMEWORK_INC_XML_TOOLBOXDOCUMENTHANDLER_HXX
#define INCLUDED_FRAMEWORK_INC_XML_TOOLBOXDOCUMENTHANDLER_HXX

#include <framework/toolboxconfiguration.hxx>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <stdtypes.h>
#include <framework/fwedllapi.h>

namespace framework{

// Hash code function for using in all hash maps of follow implementation.

class FWE_DLLPUBLIC OReadToolBoxDocumentHandler :
                                    public ::cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{
    public:
        enum ToolBox_XML_Entry
        {
            TB_ELEMENT_TOOLBAR,
            TB_ELEMENT_TOOLBARITEM,
            TB_ELEMENT_TOOLBARSPACE,
            TB_ELEMENT_TOOLBARBREAK,
            TB_ELEMENT_TOOLBARSEPARATOR,
            TB_ATTRIBUTE_TEXT,
            TB_ATTRIBUTE_BITMAP,
            TB_ATTRIBUTE_URL,
            TB_ATTRIBUTE_ITEMBITS,
            TB_ATTRIBUTE_VISIBLE,
            TB_ATTRIBUTE_WIDTH,
            TB_ATTRIBUTE_USER,
            TB_ATTRIBUTE_HELPID,
            TB_ATTRIBUTE_STYLE,
            TB_ATTRIBUTE_UINAME,
            TB_ATTRIBUTE_TOOLTIP,
            TB_XML_ENTRY_COUNT
        };

        enum ToolBox_XML_Namespace
        {
            TB_NS_TOOLBAR,
            TB_NS_XLINK,
            TB_XML_NAMESPACES_COUNT
        };

        OReadToolBoxDocumentHandler( const css::uno::Reference< css::container::XIndexContainer >& rItemContainer );
        virtual ~OReadToolBoxDocumentHandler();

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

    private:
        OUString getErrorLineString();

        class ToolBoxHashMap : public std::unordered_map<OUString,
                                                         ToolBox_XML_Entry,
                                                         OUStringHash,
                                                         std::equal_to< OUString >  >
        {
        };

        bool                                                      m_bToolBarStartFound : 1;
        bool                                                      m_bToolBarEndFound : 1;
        bool                                                      m_bToolBarItemStartFound : 1;
        bool                                                      m_bToolBarSpaceStartFound : 1;
        bool                                                      m_bToolBarBreakStartFound : 1;
        bool                                                      m_bToolBarSeparatorStartFound : 1;
        ToolBoxHashMap                                            m_aToolBoxMap;
        css::uno::Reference< css::container::XIndexContainer >    m_rItemContainer;
        css::uno::Reference< css::xml::sax::XLocator >            m_xLocator;

        sal_Int32                                                 m_nHashCode_Style_Radio;
        sal_Int32                                                 m_nHashCode_Style_Auto;
        sal_Int32                                                 m_nHashCode_Style_Left;
        sal_Int32                                                 m_nHashCode_Style_AutoSize;
        sal_Int32                                                 m_nHashCode_Style_DropDown;
        sal_Int32                                                 m_nHashCode_Style_Repeat;
        sal_Int32                                                 m_nHashCode_Style_DropDownOnly;
        sal_Int32                                                 m_nHashCode_Style_Text;
        sal_Int32                                                 m_nHashCode_Style_Image;
        OUString                                                  m_aType;
        OUString                                                  m_aLabel;
        OUString                                                  m_aStyle;
        OUString                                                  m_aHelpURL;
        OUString                                                  m_aTooltip;
        OUString                                                  m_aIsVisible;
        OUString                                                  m_aCommandURL;
};

class FWE_DLLPUBLIC OWriteToolBoxDocumentHandler
{
    public:
            OWriteToolBoxDocumentHandler(
            const css::uno::Reference< css::container::XIndexAccess >& rItemAccess,
            css::uno::Reference< css::xml::sax::XDocumentHandler >& rDocumentHandler );
        virtual ~OWriteToolBoxDocumentHandler();

        void WriteToolBoxDocument() throw
            ( css::xml::sax::SAXException,
              css::uno::RuntimeException );

    protected:
        void WriteToolBoxItem( const OUString& aCommandURL, const OUString& aLabel, const OUString& aHelpURL, const OUString& aTooltip, sal_Int16 nStyle,
                                       sal_Int16 nWidth, bool bVisible ) throw
            ( css::xml::sax::SAXException,
              css::uno::RuntimeException );

        void WriteToolBoxSpace() throw
            ( css::xml::sax::SAXException,
              css::uno::RuntimeException );

        void WriteToolBoxBreak() throw
            ( css::xml::sax::SAXException,
              css::uno::RuntimeException );

        void WriteToolBoxSeparator() throw
            ( css::xml::sax::SAXException,
              css::uno::RuntimeException );

        css::uno::Reference< css::xml::sax::XDocumentHandler >    m_xWriteDocumentHandler;
        css::uno::Reference< css::xml::sax::XAttributeList >      m_xEmptyList;
        css::uno::Reference< css::container::XIndexAccess >           m_rItemAccess;
        OUString                                                                     m_aXMLToolbarNS;
        OUString                                                                     m_aXMLXlinkNS;
        OUString                                                                     m_aAttributeType;
        OUString                                                                     m_aAttributeURL;
};

} // namespace framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
