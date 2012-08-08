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

#ifndef __FRAMEWORK_XML_TOOLBOXDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_XML_TOOLBOXDOCUMENTHANDLER_HXX_

#include <framework/toolboxconfiguration.hxx>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <threadhelp/threadhelpbase.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <stdtypes.h>
#include <framework/fwedllapi.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.

class FWE_DLLPUBLIC OReadToolBoxDocumentHandler : private ThreadHelpBase,   // Struct for right initalization of lock member! Must be first of baseclasses.
                                    public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XDocumentHandler >
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

        OReadToolBoxDocumentHandler( const ::com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >& rItemContainer );
        virtual ~OReadToolBoxDocumentHandler();

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

        class ToolBoxHashMap : public ::boost::unordered_map<   ::rtl::OUString                 ,
                                                        ToolBox_XML_Entry               ,
                                                        OUStringHashCode                ,
                                                        ::std::equal_to< ::rtl::OUString >  >
        {
            public:
                inline void free()
                {
                    ToolBoxHashMap().swap( *this );
                }
        };

        sal_Bool                                                                        m_bToolBarStartFound : 1;
        sal_Bool                                                                        m_bToolBarEndFound : 1;
        sal_Bool                                                                        m_bToolBarItemStartFound : 1;
        sal_Bool                                                                        m_bToolBarSpaceStartFound : 1;
        sal_Bool                                                                        m_bToolBarBreakStartFound : 1;
        sal_Bool                                                                        m_bToolBarSeparatorStartFound : 1;
        ToolBoxHashMap                                                                  m_aToolBoxMap;
        com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >    m_rItemContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >        m_xLocator;

        sal_Int32                                                                       m_nHashCode_Style_Radio;
        sal_Int32                                                                       m_nHashCode_Style_Auto;
        sal_Int32                                                                       m_nHashCode_Style_Left;
        sal_Int32                                                                       m_nHashCode_Style_AutoSize;
        sal_Int32                                                                       m_nHashCode_Style_DropDown;
        sal_Int32                                                                       m_nHashCode_Style_Repeat;
        sal_Int32                                                                       m_nHashCode_Style_DropDownOnly;
        sal_Int32                                                                       m_nHashCode_Style_Text;
        sal_Int32                                                                       m_nHashCode_Style_Image;
        rtl::OUString                                                                   m_aType;
        rtl::OUString                                                                   m_aLabel;
        rtl::OUString                                                                   m_aStyle;
        rtl::OUString                                                                   m_aHelpURL;
        rtl::OUString                                                                   m_aTooltip;
        rtl::OUString                                                                   m_aIsVisible;
        rtl::OUString                                                                   m_aCommandURL;
};


class FWE_DLLPUBLIC OWriteToolBoxDocumentHandler : private ThreadHelpBase   // Struct for right initalization of lock member! Must be first of baseclasses.
{
    public:
            OWriteToolBoxDocumentHandler(
            const ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& rItemAccess,
            ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& rDocumentHandler );
        virtual ~OWriteToolBoxDocumentHandler();

        void WriteToolBoxDocument() throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

    protected:
        virtual void WriteToolBoxItem( const rtl::OUString& aCommandURL, const rtl::OUString& aLabel, const rtl::OUString& aHelpURL, const rtl::OUString& aTooltip, sal_Int16 nStyle,
                                       sal_Int16 nWidth, sal_Bool bVisible ) throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        virtual void WriteToolBoxSpace() throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        virtual void WriteToolBoxBreak() throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        virtual void WriteToolBoxSeparator() throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xWriteDocumentHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >      m_xEmptyList;
        com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >           m_rItemAccess;
        ::rtl::OUString                                                                     m_aXMLToolbarNS;
        ::rtl::OUString                                                                     m_aXMLXlinkNS;
        ::rtl::OUString                                                                     m_aAttributeType;
        ::rtl::OUString                                                                     m_aAttributeURL;
};

} // namespace framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
