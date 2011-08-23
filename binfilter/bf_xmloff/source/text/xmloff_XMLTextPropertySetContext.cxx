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

#ifndef _XMLOFF_XMLTEXTPROPERTYSETCONTEXT_HXX
#include "XMLTextPropertySetContext.hxx"
#endif
#ifndef _XMLTEXTCOLUMNSCONTEXT_HXX
#include "XMLTextColumnsContext.hxx"
#endif
#ifndef _XMLBACKGROUNDIMAGECONTEXT_HXX
#include "XMLBackgroundImageContext.hxx"
#endif
#ifndef _XMLOFF_XMLSECTIONFOOTNOTECONFIGIMPORT_HXX
#include "XMLSectionFootnoteConfigImport.hxx"
#endif

#ifndef _XMLOFF_TXTPRMAP_HXX
#include "txtprmap.hxx"
#endif
#ifndef _XMLOFF_XMLTABI_HXX
#include "xmltabi.hxx"
#endif
#ifndef _XMLOFF_TXTDROPI_HXX
#include "txtdropi.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

XMLTextPropertySetContext::XMLTextPropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap,
                 OUString& rDCTextStyleName ) :
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, rProps, rMap ),
    rDropCapTextStyleName( rDCTextStyleName )
{
}

XMLTextPropertySetContext::~XMLTextPropertySetContext()
{
}
    
SvXMLImportContext *XMLTextPropertySetContext::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContext *pContext = 0;

    switch( xMapper->getPropertySetMapper()
                    ->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_TABSTOP:
        pContext = new SvxXMLTabStopImportContext( GetImport(), nPrefix,
                                                   rLocalName, rProp,
                                                   rProperties );
        break;
    case CTF_TEXTCOLUMNS:
#ifndef SVX_LIGHT
        pContext = new XMLTextColumnsContext( GetImport(), nPrefix,
                                                   rLocalName, xAttrList, rProp,
                                                   rProperties );
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
        break;

    case CTF_DROPCAPFORMAT:
        {
            DBG_ASSERT( rProp.mnIndex >= 2 &&
                        CTF_DROPCAPWHOLEWORD  == xMapper->getPropertySetMapper()
                            ->GetEntryContextId( rProp.mnIndex-2 ),
                        "invalid property map!");
            XMLTextDropCapImportContext *pDCContext =
                new XMLTextDropCapImportContext( GetImport(), nPrefix,
                                                        rLocalName, xAttrList,
                                                        rProp,
                                                        rProp.mnIndex-2,
                                                        rProperties );
            rDropCapTextStyleName = pDCContext->GetStyleName();
        pContext = pDCContext;
        }
        break;

    case CTF_BACKGROUND_URL:
    {
        DBG_ASSERT( rProp.mnIndex >= 2 &&
                    CTF_BACKGROUND_POS  == xMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-2 ) &&
                    CTF_BACKGROUND_FILTER  == xMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-1 ),
                    "invalid property map!");

        // #99657# Transparency might be there as well... but doesn't have
        // to. Thus, this is checked with an if, rather than with an assertion.
        sal_Int32 nTranspIndex = -1;
        if( (rProp.mnIndex >= 3) &&
            ( CTF_BACKGROUND_TRANSPARENCY == 
              xMapper->getPropertySetMapper()->GetEntryContextId( 
                  rProp.mnIndex-3 ) ) )
            nTranspIndex = rProp.mnIndex-3;

        pContext = 
            new XMLBackgroundImageContext( GetImport(), nPrefix,
                                           rLocalName, xAttrList,
                                           rProp,
                                           rProp.mnIndex-2,
                                           rProp.mnIndex-1,
                                           nTranspIndex,
                                           rProperties );
    }
    break;
#ifndef SVX_LIGHT
    case CTF_SECTION_FOOTNOTE_END:
        pContext = new XMLSectionFootnoteConfigImport( 
            GetImport(), nPrefix, rLocalName, rProperties, 
            xMapper->getPropertySetMapper(), rProp.mnIndex);
        break;
    case CTF_SECTION_ENDNOTE_END:
        pContext = new XMLSectionFootnoteConfigImport( 
            GetImport(), nPrefix, rLocalName, rProperties, 
            xMapper->getPropertySetMapper(), rProp.mnIndex);
        break;
#endif // #ifndef SVX_LIGHT
    }
    
    if( !pContext )
        pContext = SvXMLPropertySetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList, 
                                                            rProperties, rProp );
    
    return pContext;
}


}//end of namespace binfilter
