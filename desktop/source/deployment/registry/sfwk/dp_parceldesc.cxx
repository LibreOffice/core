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
#include "precompiled_desktop.hxx"
#include "dp_misc.h"
#include "dp_parceldesc.hxx"



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

namespace css = ::com::sun::star;
namespace dp_registry
{
namespace backend
{
namespace sfwk
{


// XDocumentHandler
void SAL_CALL
ParcelDescDocHandler::startDocument()
throw ( xml::sax::SAXException, RuntimeException )
{
    m_bIsParsed = false;
}

void SAL_CALL
ParcelDescDocHandler::endDocument()
throw ( xml::sax::SAXException, RuntimeException )
{
    m_bIsParsed = true;
}

void SAL_CALL
ParcelDescDocHandler::characters( const OUString & )
    throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::ignorableWhitespace( const OUString & )
    throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::processingInstruction(
    const OUString &, const OUString & )
        throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::setDocumentLocator(
    const Reference< xml::sax::XLocator >& )
        throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::startElement( const OUString& aName,
    const Reference< xml::sax::XAttributeList > & xAttribs )
        throw ( xml::sax::SAXException,
            RuntimeException )
{

    dp_misc::TRACE(OUSTR("ParcelDescDocHandler::startElement() for ") +
        aName + OUSTR("\n"));
    if ( !skipIndex )
    {
        if ( aName.equals( OUString(RTL_CONSTASCII_USTRINGPARAM( "parcel" )) ) )
        {
            m_sLang = xAttribs->getValueByName( OUString(RTL_CONSTASCII_USTRINGPARAM( "language" )) );
        }
        ++skipIndex;
    }
    else
    {
        dp_misc::TRACE(OUSTR("ParcelDescDocHandler::startElement() skipping for ")
            + aName + OUSTR("\n"));
    }

}

void SAL_CALL ParcelDescDocHandler::endElement( const OUString & aName )
   throw ( xml::sax::SAXException, RuntimeException )
{
    if ( skipIndex )
    {
        --skipIndex;
        dp_misc::TRACE(OUSTR("ParcelDescDocHandler::endElement() skipping for ")
            + aName + OUSTR("\n"));
    }
}


}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
