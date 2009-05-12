/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: toolboxlayoutdocumenthandler.hxx,v $
 * $Revision: 1.7 $
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

#ifndef __FRAMEWORK_CLASSES_TOOLBOXLAYOUTDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_CLASSES_TOOLBOXLAYOUTDOCUMENTHANDLER_HXX_

#include <classes/toolboxconfiguration.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef __COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.

class OReadToolBoxLayoutDocumentHandler : public ::com::sun::star::xml::sax::XDocumentHandler,
                                          private ThreadHelpBase,   // Struct for right initalization of lock member! Must be first of baseclasses.
                                          public ::cppu::OWeakObject
{
    public:
        enum ToolBoxLayout_XML_Entry
        {
            TBL_ELEMENT_TOOLBARLAYOUTS,
            TBL_ELEMENT_TOOLBARLAYOUT,
            TBL_ELEMENT_TOOLBARCONFIGITEMS,
            TBL_ELEMENT_TOOLBARCONFIGITEM,
            TBL_ATTRIBUTE_ID,
            TBL_ATTRIBUTE_TOOLBARNAME,
            TBL_ATTRIBUTE_CONTEXT,
            TBL_ATTRIBUTE_FLOATINGLINES,
            TBL_ATTRIBUTE_DOCKINGLINES,
            TBL_ATTRIBUTE_ALIGN,
            TBL_ATTRIBUTE_FLOATING,
            TBL_ATTRIBUTE_FLOATINGPOSLEFT,
            TBL_ATTRIBUTE_FLOATINGPOSTOP,
            TBL_ATTRIBUTE_VISIBLE,
            TBL_ATTRIBUTE_STYLE,
            TBL_ATTRIBUTE_USERDEFNAME,
            TBL_XML_ENTRY_COUNT
        };

        enum ToolBox_XML_Namespace
        {
            TBL_NS_TOOLBAR,
            TBL_XML_NAMESPACES_COUNT
        };

        OReadToolBoxLayoutDocumentHandler( ToolBoxLayoutDescriptor& aToolBoxLayoutItems );
        virtual ~OReadToolBoxLayoutDocumentHandler();

        // XInterface
        virtual void SAL_CALL acquire() throw()
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw()
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type & rType ) throw( ::com::sun::star::uno::RuntimeException );

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

        class ToolBoxLayoutHashMap : public ::std::hash_map< ::rtl::OUString                ,
                                                             ToolBoxLayout_XML_Entry        ,
                                                             OUStringHashCode               ,
                                                             ::std::equal_to< ::rtl::OUString > >
        {
            public:
                inline void free()
                {
                    ToolBoxLayoutHashMap().swap( *this );
                }
        };

        sal_Bool                                                                    m_bToolBarLayoutsStartFound;
        sal_Bool                                                                    m_bToolBarLayoutsEndFound;
        sal_Bool                                                                    m_bToolBarLayoutStartFound;
        sal_Bool                                                                    m_bToolBarConfigListStartFound;
        sal_Bool                                                                    m_bToolBarConfigItemStartFound;
        ToolBoxLayoutHashMap                                                        m_aToolBoxMap;
        ToolBoxLayoutDescriptor&                                                    m_aToolBoxItems;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >    m_xLocator;
};

class OWriteToolBoxLayoutDocumentHandler : private ThreadHelpBase // Struct for right initalization of lock member! Must be first of baseclasses.
{
    public:
        OWriteToolBoxLayoutDocumentHandler(
            const ToolBoxLayoutDescriptor& aToolBoxLayoutItems,
            ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > );
        virtual ~OWriteToolBoxLayoutDocumentHandler();

        void WriteToolBoxLayoutDocument() throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

    protected:
        virtual void WriteToolBoxLayoutItem( const ToolBoxLayoutItemDescriptor* ) throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        const ToolBoxLayoutDescriptor&                                                      m_aToolBoxLayoutItems;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xWriteDocumentHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >      m_xEmptyList;
        ::rtl::OUString                                                                     m_aXMLToolbarNS;
        ::rtl::OUString                                                                     m_aAttributeType;
        ::rtl::OUString                                                                     m_aTrueValue;
        ::rtl::OUString                                                                     m_aFalseValue;
};

} // namespace framework

#endif
