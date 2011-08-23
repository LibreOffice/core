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

#include "xmlimp.hxx"


#include "xmlprcon.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::std;

SvXMLPropertySetContext::SvXMLPropertySetContext(
    SvXMLImport& rImp, USHORT nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    vector< XMLPropertyState > &rProps,
    const UniReference < SvXMLImportPropertyMapper >  &rMap,
    sal_Int32 nSIdx, sal_Int32 nEIdx ):
    SvXMLImportContext( rImp, nPrfx, rLName ),
    rProperties( rProps ),
    xMapper    ( rMap ),
    nStartIdx( nSIdx ),
    nEndIdx( nEIdx )
{
    xMapper->importXML( rProperties, xAttrList,
                        GetImport().GetMM100UnitConverter(),
                        GetImport().GetNamespaceMap(), nStartIdx, nEndIdx );
}

SvXMLPropertySetContext::~SvXMLPropertySetContext()
{
}

SvXMLImportContext *SvXMLPropertySetContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    UniReference< XMLPropertySetMapper > aSetMapper(
            xMapper->getPropertySetMapper() );
    sal_Int32 nEntryIndex = aSetMapper->GetEntryIndex( nPrefix, rLocalName,
                                                       nStartIdx );

    if( ( nEntryIndex != -1 ) && (-1 == nEndIdx || nEntryIndex < nEndIdx ) &&
        ( 0 != ( aSetMapper->GetEntryFlags( nEntryIndex )
                         & MID_FLAG_ELEMENT_ITEM_IMPORT ) ) )
    {
        XMLPropertyState aProp( nEntryIndex );
        return CreateChildContext( nPrefix, rLocalName, xAttrList,
                                   rProperties, aProp );
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
SvXMLImportContext *SvXMLPropertySetContext::CreateChildContext(
    USHORT nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    ::std::vector< XMLPropertyState > &rProperties,
    const XMLPropertyState& rProp )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
