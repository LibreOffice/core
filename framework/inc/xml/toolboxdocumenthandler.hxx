/*************************************************************************
 *
 *  $RCSfile: toolboxdocumenthandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 16:55:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_XML_TOOLBOXDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_XML_TOOLBOXDOCUMENTHANDLER_HXX_

#ifndef __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_
#include <xml/toolboxconfiguration.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.

class OReadToolBoxDocumentHandler : public ::com::sun::star::xml::sax::XDocumentHandler,
                                    private ThreadHelpBase, // Struct for right initalization of lock member! Must be first of baseclasses.
                                    public ::cppu::OWeakObject
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

        class ToolBoxHashMap : public ::std::hash_map<  ::rtl::OUString                 ,
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

        sal_Bool                                                                        m_bToolBarStartFound;
        sal_Bool                                                                        m_bToolBarEndFound;
        sal_Bool                                                                        m_bToolBarItemStartFound;
        sal_Bool                                                                        m_bToolBarSpaceStartFound;
        sal_Bool                                                                        m_bToolBarBreakStartFound;
        sal_Bool                                                                        m_bToolBarSeparatorStartFound;
        ToolBoxHashMap                                                                  m_aToolBoxMap;
        com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >    m_rItemContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >        m_xLocator;

        sal_Int32                                                                       m_nHashCode_Style_Radio;
        sal_Int32                                                                       m_nHashCode_Style_Auto;
        sal_Int32                                                                       m_nHashCode_Style_Left;
        sal_Int32                                                                       m_nHashCode_Style_AutoSize;
        sal_Int32                                                                       m_nHashCode_Style_DropDown;
        sal_Int32                                                                       m_nHashCode_Style_Repeat;
};


class OWriteToolBoxDocumentHandler : private ThreadHelpBase // Struct for right initalization of lock member! Must be first of baseclasses.
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
        virtual void WriteToolBoxItem( const rtl::OUString& aCommandURL, const rtl::OUString& aLabel, const rtl::OUString& aHelpURL,
                                       sal_Bool bVisible ) throw
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
