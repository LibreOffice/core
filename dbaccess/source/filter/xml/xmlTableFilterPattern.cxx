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
#include "precompiled_dbaccess.hxx"
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
                ,const ::rtl::OUString& _sLocalName
                ,sal_Bool _bNameFilter
                ,OXMLTableFilterList& _rParent)
    :SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_rParent(_rParent)
    ,m_bNameFilter(_bNameFilter)
{
    DBG_CTOR(OXMLTableFilterPattern,NULL);

}
// -----------------------------------------------------------------------------

OXMLTableFilterPattern::~OXMLTableFilterPattern()
{

    DBG_DTOR(OXMLTableFilterPattern,NULL);
}
// -----------------------------------------------------------------------------
void OXMLTableFilterPattern::Characters( const ::rtl::OUString& rChars )
{
    if ( m_bNameFilter )
        m_rParent.pushTableFilterPattern(rChars);
    else
        m_rParent.pushTableTypeFilter(rChars);
}

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
