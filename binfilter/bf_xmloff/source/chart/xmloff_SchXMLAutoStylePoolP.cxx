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

#include <xmlprmap.hxx>

#include "SchXMLAutoStylePoolP.hxx"
#include "PropertyMap.hxx"

#include "SchXMLExport.hxx"
namespace binfilter {

using ::binfilter::xmloff::token::GetXMLToken;
using ::binfilter::xmloff::token::XML_CDATA;

SchXMLAutoStylePoolP::SchXMLAutoStylePoolP( SchXMLExport& rSchXMLExport ) :
        SvXMLAutoStylePoolP( rSchXMLExport ),
        mrSchXMLExport( rSchXMLExport )
{}

SchXMLAutoStylePoolP::~SchXMLAutoStylePoolP()
{}

void SchXMLAutoStylePoolP::exportStyleAttributes(
    SvXMLAttributeList& rAttrList,
    sal_Int32 nFamily,
    const ::std::vector< XMLPropertyState >& rProperties,
    const SvXMLExportPropertyMapper& rPropExp
    , const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap
    ) const
{
    const ::rtl::OUString sCDATA( GetXMLToken( XML_CDATA ));
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties,
                                                rPropExp, rUnitConverter, rNamespaceMap );

    if( nFamily == XML_STYLE_FAMILY_SCH_CHART_ID )
    {
        ::std::vector< XMLPropertyState >::const_iterator iter = rProperties.begin();
        for( iter; (iter != rProperties.end()); iter++ )
        {
            UniReference< XMLPropertySetMapper > aPropMapper =
                mrSchXMLExport.GetPropertySetMapper();
            sal_Int16 nContextID = aPropMapper->GetEntryContextId( iter->mnIndex );
            if( nContextID == XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT )
            {
                sal_Int32 nNumberFormat = -1;
                if( ( iter->maValue >>= nNumberFormat ) &&
                    ( nNumberFormat != -1 ))
                {
                    ::rtl::OUString sAttrValue = mrSchXMLExport.getDataStyleName( nNumberFormat );
                    if( sAttrValue.getLength() )
                    {
                        mrSchXMLExport.AddAttribute(
                            aPropMapper->GetEntryNameSpace( iter->mnIndex ),
                            aPropMapper->GetEntryXMLName( iter->mnIndex ), 
                            sAttrValue );
                    }
                }
            }
        }
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
