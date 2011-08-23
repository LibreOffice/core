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

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#define _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <bf_xmloff/xmlictxt.hxx>

#include <bf_xmloff/xmlevent.hxx>

#include <map>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax {	class XAttributeList; } }
    namespace beans { struct PropertyValue;	}
    namespace container { class XNameReplace; }
    namespace document { class XEventsSupplier; }
} } }
namespace rtl {	class OUString; }
namespace binfilter {

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
class XMLEventsImportContext : public SvXMLImportContext
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

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
