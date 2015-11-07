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
#include <com/sun/star/uno/Sequence.h>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>


using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::xml::sax::XAttributeList;
using namespace ::xmloff::token;




XMLTrackedChangesImportContext::XMLTrackedChangesImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName)
{
}

XMLTrackedChangesImportContext::~XMLTrackedChangesImportContext()
{
}

void XMLTrackedChangesImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList )
{
    bool bTrackChanges = true;

    // scan for text:track-changes and text:protection-key attributes
    sal_Int16 nLength = xAttrList->getLength();
    for( sal_Int16 i = 0; i < nLength; i++ )
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        if ( XML_NAMESPACE_TEXT == nPrefix )
        {
            if ( IsXMLToken( sLocalName, XML_TRACK_CHANGES ) )
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(
                    bTmp, xAttrList->getValueByIndex(i)) )
                {
                    bTrackChanges = bTmp;
                }
            }
        }
    }

    // set tracked changes
    GetImport().GetTextImport()->SetRecordChanges( bTrackChanges );
}


SvXMLImportContext* XMLTrackedChangesImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         IsXMLToken( rLocalName, XML_CHANGED_REGION ) )
    {
        pContext = new XMLChangedRegionImportContext(GetImport(),
                                                     nPrefix, rLocalName);
    }

    if (NULL == pContext)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
