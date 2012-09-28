/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <tools/debug.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmlimpit.hxx"
#include "xmlitem.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;

SvXMLItemSetContext::SvXMLItemSetContext( SvXMLImport& rImp, sal_uInt16 nPrfx,
                                          const OUString& rLName,
                                          const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                          SfxItemSet& rISet,
                                          SvXMLImportItemMapper& rIMap,
                                          const SvXMLUnitConverter& rUnitConverter ):
    SvXMLImportContext( rImp, nPrfx, rLName ),
    rItemSet( rISet ),
    rIMapper( rIMap ),
    rUnitConv( rUnitConverter )
{
    rIMap.importXML( rItemSet, xAttrList, rUnitConv,
                           GetImport().GetNamespaceMap() );
}

SvXMLItemSetContext::~SvXMLItemSetContext()
{
}

SvXMLImportContext *SvXMLItemSetContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLocalName,
                                            const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLItemMapEntriesRef xMapEntries = rIMapper.getMapEntries();
    SvXMLItemMapEntry* pEntry = xMapEntries->getByName( nPrefix, rLocalName );

    if( pEntry && 0 != (pEntry->nMemberId & MID_SW_FLAG_ELEMENT_ITEM_IMPORT) )
    {
        return CreateChildContext( nPrefix, rLocalName, xAttrList,
                                   rItemSet, *pEntry, rUnitConv );
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

/** This method is called from this instance implementation of
    CreateChildContext if the element matches an entry in the
    SvXMLImportItemMapper with the mid flag MID_SW_FLAG_ELEMENT
*/
SvXMLImportContext *SvXMLItemSetContext::CreateChildContext( sal_uInt16 nPrefix,
                                   const rtl::OUString& rLocalName,
                                   const uno::Reference< xml::sax::XAttributeList >& /*xAttrList*/,
                                    SfxItemSet&  /*rItemSet*/,
                                   const SvXMLItemMapEntry& /*rEntry*/,
                                   const SvXMLUnitConverter& /*rUnitConv*/ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}


