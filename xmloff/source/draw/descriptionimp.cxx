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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include <tools/debug.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>

#include "descriptionimp.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////

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
    if( msText.getLength() ) try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY_THROW);
        if(IsXMLToken(GetLocalName(),XML_TITLE))
        {
            xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Title")), Any(msText));
        }
        else
        {
            xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Description")), Any(msText));
        }
    }
    catch( uno::Exception& )
    {
    }
}

// This method is called for all characters that are contained in the
// current element. The default is to ignore them.
void SdXMLDescriptionContext::Characters( const ::rtl::OUString& rChars )
{
    msText += rChars;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
