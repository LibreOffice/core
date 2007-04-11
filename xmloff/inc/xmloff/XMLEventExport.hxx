/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLEventExport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:22:50 $
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

#ifndef _XMLOFF_XMLEVENTEXPORT_HXX
#define _XMLOFF_XMLEVENTEXPORT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _XMLOFF_XMLEVENT_HXX
#include <xmloff/xmlevent.hxx>
#endif

#include <map>

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace document { class XEventsSupplier; }
    namespace container { class XNameReplace; }
    namespace container { class XNameAccess; }
    namespace beans { struct PropertyValue; }
} } }

typedef ::std::map< ::rtl::OUString, XMLEventExportHandler* > HandlerMap;
typedef ::std::map< ::rtl::OUString, XMLEventName > NameMap;

/**
 * Export instances of EventsSupplier services. To use this class you
 * must fulfill two conditions:
 *
 * 1) provide a translation from the API event names to XML event
 * names
 * 2) Register XMLEventExportHandler instances for all script types
 * that you would like to export.
 *
 * The Export()-Methods all have a bUseWhitespace parameter that
 * causes the exported elements to be surrounded by whitespace, which
 * in turn causes the elements to be indented properly. By default,
 * whitespace is used, but it may not be called for in all cases (e.g
 * events attached to hyperlink within a paragraph.)
 */
class XMLOFF_DLLPUBLIC XMLEventExport
{
    const ::rtl::OUString sEventType;

    SvXMLExport& rExport;

    HandlerMap aHandlerMap;
    NameMap aNameTranslationMap;

public:
    XMLEventExport(SvXMLExport& rExport,
                   const XMLEventNameTranslation* pTranslationTable = NULL);
    ~XMLEventExport();

    /// register an EventExportHandler for a particular script type
    ///
    /// The handlers will be deleted when the object is destroyed, hence
    /// no pointers to a handler registered with AddHandler() should be
    /// held by anyone.
    void AddHandler( const ::rtl::OUString& rName,
                     XMLEventExportHandler* rHandler );

    /// register additional event names
    void AddTranslationTable( const XMLEventNameTranslation* pTransTable );

    /// export the events (calls EventExport::Export(Reference<XNameAcess>) )
    void Export( ::com::sun::star::uno::Reference<
                     ::com::sun::star::document::XEventsSupplier> & xAccess,
                sal_Bool bUseWhitespace = sal_True);

    /// export the events (calls EventExport::Export(Reference<XNameAcess>) )
    void Export( ::com::sun::star::uno::Reference<
                     ::com::sun::star::container::XNameReplace> & xAccess,
                sal_Bool bUseWhitespace = sal_True);

    /// export the events (writes <office:events> element)
    void Export( ::com::sun::star::uno::Reference<
                     ::com::sun::star::container::XNameAccess> & xAccess,
                sal_Bool bUseWhitespace = sal_True);

    /// export a single event (writes <office:events> element)
    void ExportSingleEvent(
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue>& rEventValues,
        const ::rtl::OUString& rApiEventName,
        sal_Bool bUseWhitespace = sal_True );

private:

    /// export one event (start container-element if necessary)
    SAL_DLLPRIVATE void ExportEvent(
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue>& rEventValues,
        const XMLEventName& rXmlEventName,
        sal_Bool bUseWhitespace,
        sal_Bool& rExported);

    /// export the start element
    SAL_DLLPRIVATE void StartElement(sal_Bool bUseWhitespace);

    /// export the end element
    SAL_DLLPRIVATE void EndElement(sal_Bool bUseWhitespace);
};

#endif
