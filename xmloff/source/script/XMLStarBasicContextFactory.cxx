/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLStarBasicContextFactory.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:41:16 $
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

#ifndef _XMLOFF_XMLSTARBASICCONTEXTFACTORY_HXX
#include "XMLStarBasicContextFactory.hxx"
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


XMLStarBasicContextFactory::XMLStarBasicContextFactory() :
    sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType")),
    sLibrary(RTL_CONSTASCII_USTRINGPARAM("Library")),
    sMacroName(RTL_CONSTASCII_USTRINGPARAM("MacroName")),
    sStarBasic(RTL_CONSTASCII_USTRINGPARAM("StarBasic"))
{
}

XMLStarBasicContextFactory::~XMLStarBasicContextFactory()
{
}

SvXMLImportContext* XMLStarBasicContextFactory::CreateContext(
    SvXMLImport& rImport,
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList,
    XMLEventsImportContext* rEvents,
    const OUString& rApiEventName,
    const OUString& /*rApiLanguage*/)
{
    OUString sLibraryVal;
    OUString sMacroNameVal;

    sal_Int16 nCount = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), &sLocalName );

        if (XML_NAMESPACE_SCRIPT == nPrefix)
        {
//          if (IsXMLToken(sLocalName, XML_LIBRARY))
//          {
//              sLibraryVal = xAttrList->getValueByIndex(nAttr);
//          }
//          if (IsXMLToken(sLocalName, XML_LOCATION))
//          {
//              sLibraryVal = xAttrList->getValueByIndex(nAttr);
//                if ( IsXMLToken( sLibraryVal, XML_APPLICATION ) )
//                    sLibraryVal =
//                        OUString(RTL_CONSTASCII_USTRINGPARAM("StarOffice"));
//          }
//          else
            if (IsXMLToken(sLocalName, XML_MACRO_NAME))
            {
                sMacroNameVal = xAttrList->getValueByIndex(nAttr);
            }
            // else: ingore
        }
        // else: ignore
    }

    const OUString& rApp = GetXMLToken( XML_APPLICATION );
    const OUString& rDoc = GetXMLToken( XML_DOCUMENT );
    if( sMacroNameVal.getLength() > rApp.getLength()+1 &&
        sMacroNameVal.copy(0,rApp.getLength()).equalsIgnoreAsciiCase( rApp ) &&
        ':' == sMacroNameVal[rApp.getLength()] )
    {
        sLibraryVal = OUString(RTL_CONSTASCII_USTRINGPARAM("StarOffice"));
        sMacroNameVal = sMacroNameVal.copy( rApp.getLength()+1 );
    }
    else if( sMacroNameVal.getLength() > rDoc.getLength()+1 &&
        sMacroNameVal.copy(0,rDoc.getLength()).equalsIgnoreAsciiCase( rDoc ) &&
        ':' == sMacroNameVal[rDoc.getLength()] )
    {
        sLibraryVal = rDoc;
        sMacroNameVal = sMacroNameVal.copy( rDoc.getLength()+1 );
    }

    Sequence<PropertyValue> aValues(3);

    // EventType
    aValues[0].Name = sEventType;
    aValues[0].Value <<= sStarBasic;

    // library name
    aValues[1].Name = sLibrary;
    aValues[1].Value <<= sLibraryVal;

    // macro name
    aValues[2].Name = sMacroName;
    aValues[2].Value <<= sMacroNameVal;

    // add values for event now
    rEvents->AddEventValues(rApiEventName, aValues);

    // return dummy context
    return new SvXMLImportContext(rImport, p_nPrefix, rLocalName);
}
