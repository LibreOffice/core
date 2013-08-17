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

#include "xmlTableFilterPattern.hxx"
#include "xmlTableFilterList.hxx"
#include <tools/debug.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLTableFilterPattern)

OXMLTableFilterPattern::OXMLTableFilterPattern( SvXMLImport& rImport,
                sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,sal_Bool _bNameFilter
                ,OXMLTableFilterList& _rParent)
    :SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_rParent(_rParent)
    ,m_bNameFilter(_bNameFilter)
{
    DBG_CTOR(OXMLTableFilterPattern,NULL);

}

OXMLTableFilterPattern::~OXMLTableFilterPattern()
{

    DBG_DTOR(OXMLTableFilterPattern,NULL);
}

void OXMLTableFilterPattern::Characters( const OUString& rChars )
{
    if ( m_bNameFilter )
        m_rParent.pushTableFilterPattern(rChars);
    else
        m_rParent.pushTableTypeFilter(rChars);
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
