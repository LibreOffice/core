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

#ifndef _XMLOFF_EVENTIMPORTHELPER_HXX
#define _XMLOFF_EVENTIMPORTHELPER_HXX


#ifndef _XMLOFF_XMLEVENT_HXX
#include "xmlevent.hxx"
#endif

#include <map>
#include <list>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax {	class XAttributeList; } }
} } }
namespace rtl {	class OUString; }
namespace binfilter {
class XMLEventContextFactory;
class XMLEventsImportContext;
struct XMLEventNameTranslation;

typedef ::std::map< ::rtl::OUString, XMLEventContextFactory* > FactoryMap;
typedef ::std::map< ::rtl::OUString, ::rtl::OUString > NameMap;
typedef ::std::list< NameMap* > NameMapList;


/**
 * Helps the XMLEventsImportContext.
 * 
 * This class stores
 * a) the translation from XML event names to API event names, and
 * b) a mapping from script language names to XMLEventContextFactory objects
 *    (that handle particular languages).
 *
 * Event name translation tables may be added, i.e. they will be joined 
 * together. If different translations are needed (i.e., if the same XML name
 * needs to be translated to different API names in different contexts), then
 * translation tables may be saved on a translation table stack.
 */
class XMLEventImportHelper
{
    /// map of XMLEventContextFactory objects
    FactoryMap aFactoryMap;

    /// map from XML to API names
    NameMap* pEventNameMap;

    /// stack of previous aEventNameMap
    NameMapList aEventNameMapList;

public:
    XMLEventImportHelper();

    ~XMLEventImportHelper();

    /// register a handler for a particular language type
    void RegisterFactory( const ::rtl::OUString& rLanguage,
                          XMLEventContextFactory* aFactory );

    /// add event name translation to the internal table
    void AddTranslationTable( const XMLEventNameTranslation* pTransTable );

    /// save the old translation table on a stack and install an empty table
    void PushTranslationTable();

    /// recover the top-most previously saved translation table
    void PopTranslationTable();

    /// create an appropriate import context for a particular event
    SvXMLImportContext* CreateContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList,
        XMLEventsImportContext* rEvents,
        const ::rtl::OUString& rXmlEventName,
        const ::rtl::OUString& rLanguage);

};

}//end of namespace binfilter
#endif
