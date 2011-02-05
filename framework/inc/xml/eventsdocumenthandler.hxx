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

#ifndef __FRAMEWORK_XML_EVENTSDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_XML_EVENTSDOCUMENTHANDLER_HXX_

#include <xml/eventsconfiguration.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>

#include <boost/unordered_map.hpp>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.

class OReadEventsDocumentHandler :  private ThreadHelpBase, // Struct for right initalization of lock member! Must be first of baseclasses.
                                    public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XDocumentHandler >
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

    protected:
        virtual ~OReadEventsDocumentHandler();

    private:
        ::rtl::OUString getErrorLineString();

        class EventsHashMap : public ::boost::unordered_map<    ::rtl::OUString                 ,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
