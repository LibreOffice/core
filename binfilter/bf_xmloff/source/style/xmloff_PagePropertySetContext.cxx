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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_PAGEPROPERTYSETCONTEXT_HXX
#include "PagePropertySetContext.hxx"
#endif
#ifndef _XMLBACKGROUNDIMAGECONTEXT_HXX
#include "XMLBackgroundImageContext.hxx"
#endif
#ifndef _XMLTEXTCOLUMNSCONTEXT_HXX
#include "XMLTextColumnsContext.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif
#ifndef _XMLOFF_XMLFOOTNOTESEPARATORIMPORT_HXX
#include "XMLFootnoteSeparatorImport.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

PagePropertySetContext::PagePropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap,
                 sal_Int32 nStartIndex, sal_Int32 nEndIndex,
                 const PageContextType aTempType ) :
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, rProps, rMap, nStartIndex, nEndIndex )
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
    }
    SvXMLImportContext *pContext = 0;

    switch( xMapper->getPropertySetMapper()
                    ->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_PM_GRAPHICURL:
    case CTF_PM_HEADERGRAPHICURL:
    case CTF_PM_FOOTERGRAPHICURL:
        DBG_ASSERT( rProp.mnIndex >= 2 &&
                    nPos  == xMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-2 ) &&
                    nFil  == xMapper->getPropertySetMapper()
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
            xMapper->getPropertySetMapper(), rProp.mnIndex);
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


}//end of namespace binfilter
