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

#include <tools/debug.hxx>
#include "XMLTextPropertySetContext.hxx"
#include <XMLTextColumnsContext.hxx>
#include <XMLBackgroundImageContext.hxx>
#include "XMLSectionFootnoteConfigImport.hxx"

#include <xmloff/xmlimppr.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmltabi.hxx>
#include "txtdropi.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

XMLTextPropertySetContext::XMLTextPropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 sal_uInt32 nFamily,
                 ::std::vector< XMLPropertyState > &rProps,
                 const rtl::Reference < SvXMLImportPropertyMapper > &rMap,
                 OUString& rDCTextStyleName ) :
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, nFamily,
                             rProps, rMap ),
    rDropCapTextStyleName( rDCTextStyleName )
{
}

XMLTextPropertySetContext::~XMLTextPropertySetContext()
{
}

SvXMLImportContextRef XMLTextPropertySetContext::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContextRef xContext;

    switch( mxMapper->getPropertySetMapper()
                    ->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_TABSTOP:
        xContext = new SvxXMLTabStopImportContext( GetImport(), nPrefix,
                                                   rLocalName, rProp,
                                                   rProperties );
        break;
    case CTF_TEXTCOLUMNS:
        xContext = new XMLTextColumnsContext( GetImport(), nPrefix,
                                                   rLocalName, xAttrList, rProp,
                                                   rProperties );
        break;

    case CTF_DROPCAPFORMAT:
        {
            DBG_ASSERT( rProp.mnIndex >= 2 &&
                        CTF_DROPCAPWHOLEWORD  == mxMapper->getPropertySetMapper()
                            ->GetEntryContextId( rProp.mnIndex-2 ),
                        "invalid property map!");
            XMLTextDropCapImportContext *pDCContext =
                new XMLTextDropCapImportContext( GetImport(), nPrefix,
                                                        rLocalName, xAttrList,
                                                        rProp,
                                                        rProp.mnIndex-2,
                                                        rProperties );
            rDropCapTextStyleName = pDCContext->GetStyleName();
            xContext = pDCContext;
        }
        break;

    case CTF_BACKGROUND_URL:
    {
        DBG_ASSERT( rProp.mnIndex >= 2 &&
                    CTF_BACKGROUND_POS  == mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-2 ) &&
                    CTF_BACKGROUND_FILTER  == mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-1 ),
                    "invalid property map!");

        // #99657# Transparency might be there as well... but doesn't have
        // to. Thus, this is checked with an if, rather than with an assertion.
        sal_Int32 nTranspIndex = -1;
        if( (rProp.mnIndex >= 3) &&
            ( CTF_BACKGROUND_TRANSPARENCY ==
              mxMapper->getPropertySetMapper()->GetEntryContextId(
                  rProp.mnIndex-3 ) ) )
            nTranspIndex = rProp.mnIndex-3;

        xContext =
            new XMLBackgroundImageContext( GetImport(), nPrefix,
                                           rLocalName, xAttrList,
                                           rProp,
                                           rProp.mnIndex-2,
                                           rProp.mnIndex-1,
                                           nTranspIndex,
                                           -1,
                                           rProperties );
    }
    break;
    case CTF_SECTION_FOOTNOTE_END:
    case CTF_SECTION_ENDNOTE_END:
        xContext = new XMLSectionFootnoteConfigImport(
            GetImport(), nPrefix, rLocalName, rProperties,
            mxMapper->getPropertySetMapper());
        break;
    }

    if (!xContext)
        xContext = SvXMLPropertySetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList,
                                                            rProperties, rProp );

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
