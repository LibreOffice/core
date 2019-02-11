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
#include "xmlAutoStyle.hxx"
#include "xmlHelper.hxx"
#include "xmlExport.hxx"
#include <xmloff/families.hxx>

namespace rptxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

void OXMLAutoStylePoolP::exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const
{
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties, rPropExp, rUnitConverter, rNamespaceMap );
    if ( nFamily == XML_STYLE_FAMILY_TABLE_CELL )
    {
        rtl::Reference< XMLPropertySetMapper > aPropMapper = rORptExport.GetCellStylePropertyMapper();
        for (const auto& rProp : rProperties)
        {
            sal_Int16 nContextID = aPropMapper->GetEntryContextId(rProp.mnIndex);
            switch (nContextID)
            {
                case CTF_RPT_NUMBERFORMAT :
                {
                    OUString sAttrValue;
                    if ( rProp.maValue >>= sAttrValue )
                    {
                        if ( !sAttrValue.isEmpty() )
                        {
                            rORptExport.AddAttribute(
                                aPropMapper->GetEntryNameSpace(rProp.mnIndex),
                                aPropMapper->GetEntryXMLName(rProp.mnIndex),
                                sAttrValue );
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

OXMLAutoStylePoolP::OXMLAutoStylePoolP(ORptExport& rTempORptExport):
    SvXMLAutoStylePoolP(rTempORptExport),
    rORptExport(rTempORptExport)
{
}

OXMLAutoStylePoolP::~OXMLAutoStylePoolP()
{
}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
