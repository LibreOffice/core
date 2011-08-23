/*************************************************************************
 *
 *    OpenOffice.org - a multi-platform office productivity suite
 *
 *    Author:
 *      Fridrich Strba  <fridrich.strba@bluewin.ch>
 *      Thorsten Behrens <tbehrens@novell.com>
 *
 *      Copyright (C) 2008, Novell Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

#ifndef INCLUDED_TOKENMAP_HXX
#define INCLUDED_TOKENMAP_HXX

#include "tokens.hxx"

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace svgi
{
    sal_Int32   getTokenId( const char* sIdent, sal_Int32 nLen );
    sal_Int32   getTokenId( const rtl::OUString& sIdent );
    const char* getTokenName( sal_Int32 nTokenId );

} // namespace svgi

#endif
