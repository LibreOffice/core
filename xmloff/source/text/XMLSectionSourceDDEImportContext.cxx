/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "XMLSectionSourceDDEImportContext.hxx"
#include "XMLSectionImportContext.hxx"
#include <com/sun/star/text/SectionFileLink.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <tools/debug.hxx>

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XMultiPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

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
        sDdeCommandFile(sAPI_DDECommandFile),
        sDdeCommandType(sAPI_DDECommandType),
       sDdeCommandElement(sAPI_DDECommandElement),
        sIsAutomaticUpdate(sAPI_IsAutomaticUpdate)
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

static const SvXMLTokenMapEntry aSectionSourceDDETokenMap[] =
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
                bool bTmp(false);
                if (::sax::Converter::convertBool(
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
    const Reference<XAttributeList> & )
{
    // ignore -> default context
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
