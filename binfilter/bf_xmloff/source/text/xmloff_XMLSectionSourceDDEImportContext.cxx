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

#include "XMLSectionSourceDDEImportContext.hxx"




#include "xmlimp.hxx"


#include "nmspmap.hxx"

#include "xmlnmspe.hxx"


#include "xmluconv.hxx"

#include <com/sun/star/uno/Reference.h>



#include <tools/debug.hxx>
namespace binfilter {

using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XMultiPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

const sal_Char sAPI_DDECommandFile[] = "DDECommandFile";
const sal_Char sAPI_DDECommandType[] = "DDECommandType";
const sal_Char sAPI_DDECommandElement[] = "DDECommandElement";
const sal_Char sAPI_IsAutomaticUpdate[] = "IsAutomaticUpdate";


TYPEINIT1(XMLSectionSourceDDEImportContext, SvXMLImportContext);

XMLSectionSourceDDEImportContext::XMLSectionSourceDDEImportContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rSectPropSet) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rSectionPropertySet(rSectPropSet),
        sDdeCommandFile(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandFile)),
        sDdeCommandType(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandType)),
       sDdeCommandElement(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandElement)),
        sIsAutomaticUpdate(RTL_CONSTASCII_USTRINGPARAM(sAPI_IsAutomaticUpdate))
{
}

XMLSectionSourceDDEImportContext::~XMLSectionSourceDDEImportContext()
{
}

enum XMLSectionSourceDDEToken 
{
    XML_TOK_SECTION_DDE_APPLICATION,
    XML_TOK_SECTION_DDE_TOPIC,
    XML_TOK_SECTION_DDE_ITEM,
    XML_TOK_SECTION_IS_AUTOMATIC_UPDATE
};

static __FAR_DATA SvXMLTokenMapEntry aSectionSourceDDETokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_DDE_APPLICATION, 
          XML_TOK_SECTION_DDE_APPLICATION },
    { XML_NAMESPACE_OFFICE, XML_DDE_TOPIC, XML_TOK_SECTION_DDE_TOPIC },
    { XML_NAMESPACE_OFFICE, XML_DDE_ITEM, XML_TOK_SECTION_DDE_ITEM },
    { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_UPDATE, 
          XML_TOK_SECTION_IS_AUTOMATIC_UPDATE },
    XML_TOKEN_MAP_END
};


void XMLSectionSourceDDEImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLTokenMap aTokenMap(aSectionSourceDDETokenMap);
    OUString sApplication;
    OUString sTopic;
    OUString sItem;
    sal_Bool bAutomaticUpdate = sal_False;
    
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );

        switch (aTokenMap.Get(nPrefix, sLocalName))
        {
            case XML_TOK_SECTION_DDE_APPLICATION:
                sApplication = xAttrList->getValueByIndex(nAttr);
                break;
            case XML_TOK_SECTION_DDE_TOPIC:
                sTopic = xAttrList->getValueByIndex(nAttr);
                break;
            case XML_TOK_SECTION_DDE_ITEM:
                sItem = xAttrList->getValueByIndex(nAttr);
                break;
            case XML_TOK_SECTION_IS_AUTOMATIC_UPDATE:
            {
                sal_Bool bTmp;
                if (SvXMLUnitConverter::convertBool(
                    bTmp, xAttrList->getValueByIndex(nAttr)))
                {
                    bAutomaticUpdate = bTmp;
                }
                break;
            }
            default:
                ; // ignore
                break;
        }
    }

    // DDE not supported on all platforms; query property first
    if (rSectionPropertySet->getPropertySetInfo()->
        hasPropertyByName(sDdeCommandFile))
    {
        // use multi property set to force single update of connection #83654#
        Sequence<OUString> aNames(4);
        Sequence<Any> aValues(4);

        aValues[0] <<= sApplication;
        aNames[0] = sDdeCommandFile;

        aValues[1] <<= sTopic;
        aNames[1] = sDdeCommandType;

        aValues[2] <<= sItem;
        aNames[2] = sDdeCommandElement;

        aValues[3].setValue(&bAutomaticUpdate, ::getBooleanCppuType());
        aNames[3] = sIsAutomaticUpdate;

        Reference<XMultiPropertySet> rMultiPropSet(rSectionPropertySet, 
                                                   UNO_QUERY);
        DBG_ASSERT(rMultiPropSet.is(), "we'd really like a XMultiPropertySet");
        if (rMultiPropSet.is())
            rMultiPropSet->setPropertyValues(aNames, aValues);
        // else: ignore
    }
}

void XMLSectionSourceDDEImportContext::EndElement()
{
    // nothing to be done!
}

SvXMLImportContext* XMLSectionSourceDDEImportContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    // ignore -> default context
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
