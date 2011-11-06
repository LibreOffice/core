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
#include "precompiled_xmloff.hxx"
#include <tools/debug.hxx>
#include "PagePropertySetContext.hxx"
#include "XMLBackgroundImageContext.hxx"
#include "XMLTextColumnsContext.hxx"
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include <xmloff/PageMasterStyleMap.hxx>
#endif
#include "XMLFootnoteSeparatorImport.hxx"

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

PagePropertySetContext::PagePropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 sal_uInt32 nFam,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap,
                 sal_Int32 nStartIndex, sal_Int32 nEndIndex,
                 const PageContextType aTempType ) :
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, nFam,
                             rProps, rMap, nStartIndex, nEndIndex )
{
    aType = aTempType;
}

PagePropertySetContext::~PagePropertySetContext()
{
}

SvXMLImportContext *PagePropertySetContext::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    sal_Int32 nPos = CTF_PM_GRAPHICPOSITION;
    sal_Int32 nFil = CTF_PM_GRAPHICFILTER;
    switch ( aType )
    {
        case Header:
        {
            nPos = CTF_PM_HEADERGRAPHICPOSITION;
            nFil = CTF_PM_HEADERGRAPHICFILTER;
        }
        break;
        case Footer:
        {
            nPos = CTF_PM_FOOTERGRAPHICPOSITION;
            nFil = CTF_PM_FOOTERGRAPHICFILTER;
        }
        break;
        default:
            break;
    }
    SvXMLImportContext *pContext = 0;

    switch( mxMapper->getPropertySetMapper()
                    ->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_PM_GRAPHICURL:
    case CTF_PM_HEADERGRAPHICURL:
    case CTF_PM_FOOTERGRAPHICURL:
        DBG_ASSERT( rProp.mnIndex >= 2 &&
                    nPos  == mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-2 ) &&
                    nFil  == mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-1 ),
                    "invalid property map!");
        pContext =
            new XMLBackgroundImageContext( GetImport(), nPrefix,
                                           rLocalName, xAttrList,
                                           rProp,
                                           rProp.mnIndex-2,
                                           rProp.mnIndex-1,
                                           -1,
                                           rProperties );
        break;

    case CTF_PM_TEXTCOLUMNS:
#ifndef SVX_LIGHT
        pContext = new XMLTextColumnsContext( GetImport(), nPrefix,
                                              rLocalName, xAttrList, rProp,
                                              rProperties );
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
        break;

    case CTF_PM_FTN_LINE_WEIGTH:
#ifndef SVX_LIGHT
        pContext = new XMLFootnoteSeparatorImport(
            GetImport(), nPrefix, rLocalName, rProperties,
            mxMapper->getPropertySetMapper(), rProp.mnIndex);
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName);
#endif // #ifndef SVX_LIGHT
        break;
    }

    if( !pContext )
        pContext = SvXMLPropertySetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList,
                                                            rProperties, rProp );

    return pContext;
}


