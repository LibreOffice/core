/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmltkmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:36:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLTKMAP_HXX
#define _XMLOFF_XMLTKMAP_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

namespace rtl { class OUString; }

class SvXMLTokenMap_Impl;
class SvXMLTokenMapEntry_Impl;

#define XML_TOK_UNKNOWN 0xffffU
#define XML_TOKEN_MAP_END { 0xffffU, ::xmloff::token::XML_TOKEN_INVALID, 0U }

struct SvXMLTokenMapEntry
{
    sal_uInt16  nPrefixKey;
    enum ::xmloff::token::XMLTokenEnum eLocalName;
    sal_uInt16  nToken;
};

class XMLOFF_DLLPUBLIC SvXMLTokenMap
{
    SvXMLTokenMap_Impl      *pImpl;

    SAL_DLLPRIVATE SvXMLTokenMapEntry_Impl *_Find(
        sal_uInt16 nKind, const ::rtl::OUString& rName ) const;

public:

    SvXMLTokenMap( const SvXMLTokenMapEntry* pMap );
    ~SvXMLTokenMap();

    sal_uInt16 Get( sal_uInt16 nPrefix, const ::rtl::OUString& rLName ) const;
};

#endif  //  _XMLOFF_XMLTKMAP_HXX

