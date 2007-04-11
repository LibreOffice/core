/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLEventsImportContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:23:02 $
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

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#define _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLEVENT_HXX
#include <xmloff/xmlevent.hxx>
#endif

#include <map>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { struct PropertyValue; }
    namespace container { class XNameReplace; }
    namespace document { class XEventsSupplier; }
} } }
namespace rtl { class OUString; }

typedef ::std::pair<
            ::rtl::OUString,
            ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue> > EventNameValuesPair;

typedef ::std::vector< EventNameValuesPair > EventsVector;

/**
 * Import <script:events> element.
 *
 * The import context usually sets the events immediatly at the event
 * XNameReplace. If none was given on construction, it operates in
 * delayed mode: All events are collected and may then be set
 * with the setEvents() method.
 */
class XMLOFF_DLLPUBLIC XMLEventsImportContext : public SvXMLImportContext
{
protected:
    // the event XNameReplace; may be empty
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameReplace> xEvents;

    // if no XNameReplace is given, use this vector to collect events
    EventsVector aCollectEvents;

public:

    TYPEINFO();

    XMLEventsImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName);

    XMLEventsImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XEventsSupplier> & xEventsSupplier);

    XMLEventsImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameReplace> & xNameRepl);

    ~XMLEventsImportContext();

    void AddEventValues(
        const ::rtl::OUString& rEventName,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rValues);

    /// if the import operates in delayed mode, you can use this method
    /// to set all events that have been read on the XEventsSupplier
    void SetEvents(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XEventsSupplier> & xEventsSupplier);

    /// if the import operates in delayed mode, you can use this method
    /// to set all events that have been read on the XNameReplace
    void SetEvents(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameReplace> & xNameRepl);

    /// if the import operates indelayed mode, you can use this method
    /// to obtain the value sequence for a specific event
    sal_Bool GetEventSequence(
        const ::rtl::OUString& rName,
        ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue> & rSequence );

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif
