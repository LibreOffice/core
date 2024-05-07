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

#include <dp_misc.h>
#include "dp_parceldesc.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace dp_registry::backend::sfwk
{


// XDocumentHandler
void SAL_CALL
ParcelDescDocHandler::startDocument()
{
    m_bIsParsed = false;
}

void SAL_CALL
ParcelDescDocHandler::endDocument()
{
    m_bIsParsed = true;
}

void SAL_CALL
ParcelDescDocHandler::characters( const OUString & )
{
}

void SAL_CALL
ParcelDescDocHandler::ignorableWhitespace( const OUString & )
{
}

void SAL_CALL
ParcelDescDocHandler::processingInstruction(
    const OUString &, const OUString & )
{
}

void SAL_CALL
ParcelDescDocHandler::setDocumentLocator(
    const Reference< xml::sax::XLocator >& )
{
}

void SAL_CALL
ParcelDescDocHandler::startElement( const OUString& aName,
    const Reference< xml::sax::XAttributeList > & xAttribs )
{

    dp_misc::TRACE("ParcelDescDocHandler::startElement() for " +
        aName + "\n");
    if ( !skipIndex )
    {
        if ( aName == "parcel" )
        {
            m_sLang = xAttribs->getValueByName( u"language"_ustr );
        }
        ++skipIndex;
    }
    else
    {
        dp_misc::TRACE("ParcelDescDocHandler::startElement() skipping for "
            + aName + "\n");
    }

}

void SAL_CALL ParcelDescDocHandler::endElement( const OUString & aName )
{
    if ( skipIndex )
    {
        --skipIndex;
        dp_misc::TRACE("ParcelDescDocHandler::endElement() skipping for "
            + aName + "\n");
    }
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
