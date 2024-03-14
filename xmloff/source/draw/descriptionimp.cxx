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

#include "descriptionimp.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;


SdXMLDescriptionContext::SdXMLDescriptionContext( SvXMLImport& rImport, sal_Int32 nElement, const Reference< XShape >& rxShape)
: SvXMLImportContext(rImport), mxShape( rxShape ), mnElement(nElement)
{
}

SdXMLDescriptionContext::~SdXMLDescriptionContext()
{
}

void SdXMLDescriptionContext::endFastElement(sal_Int32 )
{
    if( msText.isEmpty() )
        return;

    try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY_THROW);
        if( (mnElement & TOKEN_MASK) == XML_TITLE)
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
void SdXMLDescriptionContext::characters( const OUString& rChars )
{
    msText += rChars;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
