/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLEventImportHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:48:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_EVENTIMPORTHELPER_HXX
#define _XMLOFF_EVENTIMPORTHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _XMLOFF_XMLEVENT_HXX
#include "xmlevent.hxx"
#endif

#include <map>
#include <list>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace rtl { class OUString; }
class XMLEventContextFactory;
class XMLEventsImportContext;
struct XMLEventNameTranslation;

typedef ::std::map< ::rtl::OUString, XMLEventContextFactory* > FactoryMap;
typedef ::std::map< XMLEventName, ::rtl::OUString > NameMap;
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

#endif
