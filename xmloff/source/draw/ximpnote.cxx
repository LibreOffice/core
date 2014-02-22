/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ximpnote.hxx"
#include <com/sun/star/presentation/XPresentationPage.hpp>

using namespace ::com::sun::star;

SdXMLNotesContext::SdXMLNotesContext( SdXMLImport& rImport,
    sal_uInt16 nPrfx, const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLGenericPageContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
    OUString sStyleName;

    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetMasterPageAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_MASTERPAGE_PAGE_MASTER_NAME:
            {
                msPageMasterName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_STYLE_NAME:
            {
                sStyleName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_HEADER_NAME:
            {
                maUseHeaderDeclName =  sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_FOOTER_NAME:
            {
                maUseFooterDeclName =  sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_DATE_TIME_NAME:
            {
                maUseDateTimeDeclName =  sValue;
                break;
            }

        }
    }

    SetStyle( sStyleName );

    
    uno::Reference< drawing::XShape > xShape;
    while(rShapes->getCount())
    {
        rShapes->getByIndex(0L) >>= xShape;
        if(xShape.is())
            rShapes->remove(xShape);
    }

    
    if(!msPageMasterName.isEmpty())
    {
        SetPageMaster( msPageMasterName );
    }
}

SdXMLNotesContext::~SdXMLNotesContext()
{
}

SvXMLImportContext *SdXMLNotesContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    
    SvXMLImportContext *pContext = 0L;

    
    

    
    if(!pContext)
        pContext = SdXMLGenericPageContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

void SdXMLNotesContext::EndElement()
{
    SdXMLGenericPageContext::EndElement();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
