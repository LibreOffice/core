/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLStarBasicContextFactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:55:56 $
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

#ifndef _XMLOFF_XMLSTARBASICCONTEXTFACTORY_HXX
#define _XMLOFF_XMLSTARBASICCONTEXTFACTORY_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _XMLOFF_XMLEVENT_HXX
#include "xmlevent.hxx"
#endif


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace rtl { class OUString; }
class SvXMLImport;
class XMLEventsImportContext;


class XMLStarBasicContextFactory : public XMLEventContextFactory
{
    const ::rtl::OUString sEventType;
    const ::rtl::OUString sLibrary;
    const ::rtl::OUString sMacroName;
    const ::rtl::OUString sStarBasic;

public:
    XMLStarBasicContextFactory();
    virtual ~XMLStarBasicContextFactory();

    virtual SvXMLImportContext* CreateContext(
        SvXMLImport& rImport,               /// import context
        sal_uInt16 nPrefix,                 /// element: namespace prefix
        const ::rtl::OUString& rLocalName,  /// element: local name
        const ::com::sun::star::uno::Reference<     /// attribute list
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList,
        /// the context for the enclosing <script:events> element
        XMLEventsImportContext* rEvents,
        /// the event name (as understood by the API)
        const ::rtl::OUString& rApiEventName,
        /// the event type name (as registered)
        const ::rtl::OUString& rLanguage);
};

#endif
