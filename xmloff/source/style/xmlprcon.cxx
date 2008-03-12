/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlprcon.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:57:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif


#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include <xmloff/xmlprcon.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::std;

SvXMLPropertySetContext::SvXMLPropertySetContext(
    SvXMLImport& rImp, USHORT nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    sal_uInt32 nFam,
    vector< XMLPropertyState > &rProps,
    const UniReference < SvXMLImportPropertyMapper >  &rMap,
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

SvXMLImportContext *SvXMLPropertySetContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    UniReference< XMLPropertySetMapper > aSetMapper(
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
SvXMLImportContext *SvXMLPropertySetContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >&,
    ::std::vector< XMLPropertyState > &,
    const XMLPropertyState& )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}


