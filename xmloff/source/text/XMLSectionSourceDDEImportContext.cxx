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
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <tools/debug.hxx>

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XMultiPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XFastAttributeList;

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

XMLSectionSourceDDEImportContext::XMLSectionSourceDDEImportContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rSectPropSet) :
        SvXMLImportContext(rImport),
        rSectionPropertySet(rSectPropSet)
{
}

XMLSectionSourceDDEImportContext::~XMLSectionSourceDDEImportContext()
{
}

void XMLSectionSourceDDEImportContext::startFastElement(sal_Int32 /*nElement*/,
    const Reference<XFastAttributeList> & xAttrList)
{
    OUString sApplication;
    OUString sTopic;
    OUString sItem;
    bool bAutomaticUpdate = false;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(OFFICE, XML_DDE_APPLICATION):
                sApplication = aIter.toString();
                break;
            case XML_ELEMENT(OFFICE, XML_DDE_TOPIC):
                sTopic = aIter.toString();
                break;
            case XML_ELEMENT(OFFICE, XML_DDE_ITEM):
                sItem = aIter.toString();
                break;
            case XML_ELEMENT(OFFICE, XML_AUTOMATIC_UPDATE):
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, aIter.toView()))
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
    if (!rSectionPropertySet->getPropertySetInfo()->
        hasPropertyByName(u"DDECommandFile"_ustr))
        return;

    // use multi property set to force single update of connection #83654#
    Sequence<OUString> aNames { u"DDECommandFile"_ustr, u"DDECommandType"_ustr, u"DDECommandElement"_ustr, u"IsAutomaticUpdate"_ustr };
    Sequence<Any> aValues { Any(sApplication), Any(sTopic), Any(sItem), Any(bAutomaticUpdate) };

    Reference<XMultiPropertySet> rMultiPropSet(rSectionPropertySet,
                                               UNO_QUERY);
    DBG_ASSERT(rMultiPropSet.is(), "we'd really like a XMultiPropertySet");
    if (rMultiPropSet.is())
        rMultiPropSet->setPropertyValues(aNames, aValues);
    // else: ignore

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
