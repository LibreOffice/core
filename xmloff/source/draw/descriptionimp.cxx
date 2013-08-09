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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>

#include "descriptionimp.hxx"

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

TYPEINIT1( SdXMLDescriptionContext, SvXMLImportContext );

SdXMLDescriptionContext::SdXMLDescriptionContext( SvXMLImport& rImport, sal_uInt16 nPrfx,   const OUString& rLocalName,
        const Reference< XAttributeList>&, const Reference< XShape >& rxShape)
: SvXMLImportContext(rImport, nPrfx, rLocalName), mxShape( rxShape )
{
}

SdXMLDescriptionContext::~SdXMLDescriptionContext()
{
}

void SdXMLDescriptionContext::EndElement()
{
    if( !msText.isEmpty() ) try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY_THROW);
        if(IsXMLToken(GetLocalName(),XML_TITLE))
        {
            xPropSet->setPropertyValue("Title", Any(msText));
        }
        else
        {
            xPropSet->setPropertyValue("Description", Any(msText));
        }
    }
    catch( uno::Exception& )
    {
    }
}

// This method is called for all characters that are contained in the
// current element. The default is to ignore them.
void SdXMLDescriptionContext::Characters( const OUString& rChars )
{
    msText += rChars;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
