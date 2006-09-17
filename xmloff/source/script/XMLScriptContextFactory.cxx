/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLScriptContextFactory.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:40:46 $
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

#ifndef _XMLOFF_XMLSCRIPTCONTEXTFACTORY_HXX
#include "XMLScriptContextFactory.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif


using namespace ::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

XMLScriptContextFactory::XMLScriptContextFactory() :
    sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType")),
    sScript(RTL_CONSTASCII_USTRINGPARAM("Script")),
    sURL(RTL_CONSTASCII_USTRINGPARAM("Script"))
{
}

XMLScriptContextFactory::~XMLScriptContextFactory()
{
}

SvXMLImportContext * XMLScriptContextFactory::CreateContext
(SvXMLImport & rImport,
 sal_uInt16 p_nPrefix,
 const OUString & rLocalName,
 const Reference<XAttributeList> & xAttrList,
 XMLEventsImportContext * rEvents,
 const OUString & rApiEventName,
 const OUString & /*rApiLanguage*/)
{
    OUString sURLVal;

    sal_Int16 nCount = xAttrList->getLength();
    for (sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName(xAttrList->getNameByIndex(nAttr), &sLocalName);

        if (XML_NAMESPACE_XLINK == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_HREF))
                sURLVal = xAttrList->getValueByIndex(nAttr);
            // else: ignore
        }
        // else ignore
    }

    Sequence<PropertyValue> aValues(2);

    // EventType
    aValues[0].Name = sEventType;
    aValues[0].Value <<= sScript;

    // URL
    aValues[1].Name = sURL;
    aValues[1].Value <<= sURLVal;

    // add values for event now
    rEvents->AddEventValues(rApiEventName, aValues);

    // return dummy context
    return new SvXMLImportContext(rImport, p_nPrefix, rLocalName);
}

