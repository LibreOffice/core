/*************************************************************************
 *
 *  $RCSfile: XMLEventImportHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:54:52 $
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
