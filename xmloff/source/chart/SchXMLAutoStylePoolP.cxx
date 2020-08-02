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

#include <SchXMLAutoStylePoolP.hxx>
#include "PropertyMap.hxx"
#include <SchXMLExport.hxx>
#include <xmloff/families.hxx>
#include <xmloff/nmspmap.hxx>


SchXMLAutoStylePoolP::SchXMLAutoStylePoolP( SchXMLExport& rSchXMLExport ) :
        SvXMLAutoStylePoolP( rSchXMLExport ),
        mrSchXMLExport( rSchXMLExport )
{}

SchXMLAutoStylePoolP::~SchXMLAutoStylePoolP()
{}

void SchXMLAutoStylePoolP::exportStyleAttributes(
    SvXMLAttributeList& rAttrList,
    XmlStyleFamily nFamily,
    const ::std::vector< XMLPropertyState >& rProperties,
    const SvXMLExportPropertyMapper& rPropExp
    , const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap
    ) const
{
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties,
                                                rPropExp, rUnitConverter, rNamespaceMap );

    if( nFamily != XmlStyleFamily::SCH_CHART_ID )
        return;

    for( const auto& rProp : rProperties )
    {
        if( rProp.mnIndex == -1 )
            continue;

        rtl::Reference< XMLPropertySetMapper > aPropMapper =
            mrSchXMLExport.GetPropertySetMapper();
        sal_Int16 nContextID = aPropMapper->GetEntryContextId( rProp.mnIndex );
        if( nContextID == XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT )
        {
            sal_Int32 nNumberFormat = -1;
            if( ( rProp.maValue >>= nNumberFormat ) &&
                ( nNumberFormat != -1 ))
            {
                OUString sAttrValue = mrSchXMLExport.getDataStyleName( nNumberFormat );
                if( !sAttrValue.isEmpty() )
                {
                    mrSchXMLExport.AddAttribute(
                        aPropMapper->GetEntryNameSpace( rProp.mnIndex ),
                        aPropMapper->GetEntryXMLName( rProp.mnIndex ),
                        sAttrValue );
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
