/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_XML_EVENTSDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_XML_EVENTSDOCUMENTHANDLER_HXX_

#ifndef __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_
#include <framework/eventsconfiguration.hxx>
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
#include <threadhelp/threadhelpbase.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>

#include <hash_map>
#include <stdtypes.h>

#include <framework/fwedllapi.h>
//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.

class FWE_DLLPUBLIC OReadEventsDocumentHandler :    private ThreadHelpBase, // Struct for right initalization of lock member! Must be first of baseclasses.
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

class FWE_DLLPUBLIC OWriteEventsDocumentHandler : private ThreadHelpBase // Struct for right initalization of lock member! Must be first of baseclasses.
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
