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

#ifndef _XMLOFF_XMLTKMAP_HXX
#define _XMLOFF_XMLTKMAP_HXX

#include <sal/types.h>

#include <bf_xmloff/xmltoken.hxx>

namespace rtl { class OUString; }
namespace binfilter {

class SvXMLTokenMap_Impl;
class SvXMLTokenMapEntry_Impl;

#define XML_TOK_UNKNOWN 0xffffU
#define XML_TOKEN_MAP_END { 0xffffU, ::binfilter::xmloff::token::XML_TOKEN_INVALID, 0U }

struct SvXMLTokenMapEntry
{
    sal_uInt16	nPrefixKey;
    enum ::binfilter::xmloff::token::XMLTokenEnum eLocalName;
    sal_uInt16	nToken;
};

class SvXMLTokenMap
{
    SvXMLTokenMap_Impl		*pImpl;

    SvXMLTokenMapEntry_Impl 	*_Find( sal_uInt16 nKind,
                                        const ::rtl::OUString& rName ) const;

public:

    SvXMLTokenMap( SvXMLTokenMapEntry *pMap );
    ~SvXMLTokenMap();

    sal_uInt16 Get( sal_uInt16 nPrefix, const ::rtl::OUString& rLName ) const;
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLTKMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
