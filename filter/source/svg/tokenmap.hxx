/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_TOKENMAP_HXX
#define INCLUDED_TOKENMAP_HXX

#include "tokens.hxx"

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace svgi
{
    sal_Int32   getTokenId( const char* sIdent, sal_Int32 nLen );
    sal_Int32   getTokenId( const OUString& sIdent );
    const char* getTokenName( sal_Int32 nTokenId );

} // namespace svgi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
