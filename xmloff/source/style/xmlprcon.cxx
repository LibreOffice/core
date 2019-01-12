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

#include <xmloff/xmlprcon.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlprmap.hxx>

using namespace ::com::sun::star;
using namespace ::std;

SvXMLPropertySetContext::SvXMLPropertySetContext(
    SvXMLImport& rImp, sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    sal_uInt32 nFam,
    vector< XMLPropertyState > &rProps,
    const rtl::Reference < SvXMLImportPropertyMapper >  &rMap,
    sal_Int32 nSIdx, sal_Int32 nEIdx )
:   SvXMLImportContext( rImp, nPrfx, rLName )
,   mnStartIdx( nSIdx )
,   mnEndIdx( nEIdx )
,   mnFamily( nFam )
,   mrProperties( rProps )
,   mxMapper( rMap )
{
    mxMapper->importXML( mrProperties, xAttrList,
                        GetImport().GetMM100UnitConverter(),
                        GetImport().GetNamespaceMap(), mnFamily,
                        mnStartIdx, mnEndIdx );
}

SvXMLPropertySetContext::~SvXMLPropertySetContext()
{
}

SvXMLImportContextRef SvXMLPropertySetContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    rtl::Reference< XMLPropertySetMapper > aSetMapper(
            mxMapper->getPropertySetMapper() );
    sal_Int32 nEntryIndex = aSetMapper->GetEntryIndex( nPrefix, rLocalName,
                                                       mnFamily, mnStartIdx );

    if( ( nEntryIndex != -1 ) && (-1 == mnEndIdx || nEntryIndex < mnEndIdx ) &&
        ( 0 != ( aSetMapper->GetEntryFlags( nEntryIndex )
                         & MID_FLAG_ELEMENT_ITEM_IMPORT ) ) )
    {
        XMLPropertyState aProp( nEntryIndex );
        return CreateChildContext( nPrefix, rLocalName, xAttrList,
                                   mrProperties, aProp );
    }
    else
    {
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
}

/** This method is called from this instance implementation of
    CreateChildContext if the element matches an entry in the
    SvXMLImportItemMapper with the mid flag MID_FLAG_ELEMENT
*/
SvXMLImportContextRef SvXMLPropertySetContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >&,
    ::std::vector< XMLPropertyState > &,
    const XMLPropertyState& )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
