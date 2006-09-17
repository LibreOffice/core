/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLAutoTextContainerEventImport.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:01:47 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLAUTOTEXTCONTAINEREVENTIMPORT_HXX
#include "XMLAutoTextContainerEventImport.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
#endif


using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::container::XNameReplace;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_EVENT_LISTENERS;


TYPEINIT1(XMLAutoTextContainerEventImport, SvXMLImportContext);


XMLAutoTextContainerEventImport::XMLAutoTextContainerEventImport(
    SvXMLImport& rImport,
    USHORT nPrfx,
    const OUString& rLName,
    const Reference<XNameReplace> & rEvnts ) :
        SvXMLImportContext(rImport, nPrfx, rLName),
        rEvents(rEvnts)
{
}

XMLAutoTextContainerEventImport::~XMLAutoTextContainerEventImport()
{
}

SvXMLImportContext* XMLAutoTextContainerEventImport::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & )
{
    if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken( rLocalName, XML_EVENT_LISTENERS)   )
    {
        return new XMLEventsImportContext(GetImport(), nPrefix, rLocalName,
                                          rEvents);
    }
    else
        return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}
