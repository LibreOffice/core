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

#include "XMLTrackedChangesImportContext.hxx"
#include "XMLChangedRegionImportContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>


using ::com::sun::star::uno::Reference;
using namespace ::xmloff::token;


XMLTrackedChangesImportContext::XMLTrackedChangesImportContext(
    SvXMLImport& rImport) :
        SvXMLImportContext(rImport)
{
}

XMLTrackedChangesImportContext::~XMLTrackedChangesImportContext()
{
}

void XMLTrackedChangesImportContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    bool bTrackChanges = true;

    // scan for text:track-changes and text:protection-key attributes
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if (aIter.getToken() == XML_ELEMENT(TEXT, XML_TRACK_CHANGES) )
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bTrackChanges = bTmp;
            }
            break;
        }
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }

    // set tracked changes
    GetImport().GetTextImport()->SetRecordChanges( bTrackChanges );
}


css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTrackedChangesImportContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_CHANGED_REGION) )
    {
        return new XMLChangedRegionImportContext(GetImport());
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
