/*************************************************************************
 *
 *  $RCSfile: eventsdocumenthandler.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-06-22 13:31:14 $
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

#ifndef __FRAMEWORK_CLASSES_EVENTSDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_CLASSES_EVENTSDOCUMENTHANDLER_HXX_

#ifndef __FRAMEWORK_CLASSES_TOOLBOXCONFIGURATION_HXX_
#include <classes/eventsconfiguration.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef __COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
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

class OReadEventsDocumentHandler :  public ::com::sun::star::xml::sax::XDocumentHandler,
                                    private ThreadHelpBase, // Struct for right initalization of lock member! Must be first of baseclasses.
                                    public ::cppu::OWeakObject
{
    public:
        enum Events_XML_Entry
        {
            EV_ELEMENT_EVENTS,
            EV_ELEMENT_EVENT,
            EV_ATTRIBUTE_TYPE,
            EV_ATTRIBUTE_NAME,
            XL_ATTRIBUTE_HREF,
            XL_ATTRIBUTE_TYPE,
            EV_ATTRIBUTE_MACRONAME,
            EV_ATTRIBUTE_LIBRARY,
            EV_XML_ENTRY_COUNT
        };

        enum Event_XML_Namespace
        {
            EV_NS_EVENT,
            EV_NS_XLINK,
            EV_XML_NAMESPACES_COUNT
        };

        OReadEventsDocumentHandler( EventsConfig& aItems );
        virtual ~OReadEventsDocumentHandler();

        // XInterface
        virtual void SAL_CALL acquire() throw( ::com::sun::star::uno::RuntimeException )
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw( ::com::sun::star::uno::RuntimeException )
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

        class EventsHashMap : public ::std::hash_map<   ::rtl::OUString                 ,
                                                        Events_XML_Entry                ,
                                                        OUStringHashCode                ,
                                                        ::std::equal_to< ::rtl::OUString >  >
        {
            public:
                inline void free()
                {
                    EventsHashMap().swap( *this );
                }
        };

        sal_Bool                                                                    m_bEventsStartFound;
        sal_Bool                                                                    m_bEventsEndFound;
        sal_Bool                                                                    m_bEventStartFound;
        EventsHashMap                                                               m_aEventsMap;
        EventsConfig&                                                               m_aEventItems;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >    m_xLocator;
};

class OWriteEventsDocumentHandler : private ThreadHelpBase // Struct for right initalization of lock member! Must be first of baseclasses.
{
    public:
        OWriteEventsDocumentHandler(
            const EventsConfig& aItems,
            ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > );
        virtual ~OWriteEventsDocumentHandler();

        void WriteEventsDocument() throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

    protected:
        virtual void WriteEvent(
                const ::rtl::OUString& aEventName,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aPropertyValue ) throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        const EventsConfig&                                                                 m_aItems;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xWriteDocumentHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >      m_xEmptyList;
        ::rtl::OUString                                                                     m_aXMLEventNS;
        ::rtl::OUString                                                                     m_aXMLXlinkNS;
        ::rtl::OUString                                                                     m_aAttributeType;
        ::rtl::OUString                                                                     m_aAttributeURL;
        ::rtl::OUString                                                                     m_aAttributeLanguage;
        ::rtl::OUString                                                                     m_aAttributeLinkType;
        ::rtl::OUString                                                                     m_aAttributeMacroName;
        ::rtl::OUString                                                                     m_aAttributeLibrary;
        ::rtl::OUString                                                                     m_aAttributeName;
};

} // namespace framework

#endif
