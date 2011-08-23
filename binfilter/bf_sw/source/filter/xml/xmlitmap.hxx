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

#ifndef _XMLITMAP_HXX
#define _XMLITMAP_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif

namespace rtl { class OUString; } 
namespace binfilter {

#define MID_FLAG_MASK					0x0000ffff

// this flags are used in the item mapper for import and export

#define MID_FLAG_SPECIAL_ITEM_IMPORT	0x80000000
#define MID_FLAG_NO_ITEM_IMPORT			0x40000000
#define MID_FLAG_SPECIAL_ITEM_EXPORT	0x20000000
#define MID_FLAG_NO_ITEM_EXPORT			0x10000000
#define MID_FLAG_SPECIAL_ITEM			0xa0000000 // both import and export
#define MID_FLAG_NO_ITEM				0x50000000 // both import and export
#define MID_FLAG_ELEMENT_ITEM_IMPORT	0x08000000
#define MID_FLAG_ELEMENT_ITEM_EXPORT	0x04000000
#define MID_FLAG_ELEMENT_ITEM			0x0c000000  // both import and export 

// ---

struct SvXMLItemMapEntry
{
    sal_uInt16 nNameSpace;		// declares the Namespace in wich this item
                                // exists
    enum ::binfilter::xmloff::token::XMLTokenEnum eLocalName;
                                // the local name for the item inside 
                                // the Namespace (as an XMLTokenEnum)
    sal_uInt32 nWhichId;		// the WichId to identify the item
                                // in the pool
    sal_uInt32 nMemberId;		// the memberid specifies wich part
                                // of the item should be imported or
                                // exported with this Namespace
                                // and localName
};

// ---

class SvXMLItemMapEntries_impl;

/** this class manages an array of SvXMLItemMapEntry. It is
    used for optimizing the static array on startup of import
    or export */
class SvXMLItemMapEntries : public SvRefBase
{
protected:
    SvXMLItemMapEntries_impl* mpImpl;

public:
    SvXMLItemMapEntries( SvXMLItemMapEntry* pEntrys );
    virtual ~SvXMLItemMapEntries();

    SvXMLItemMapEntry* getByName( sal_uInt16 nNameSpace,
                                  const ::rtl::OUString& rString,
                                  SvXMLItemMapEntry* pStartAt = NULL ) const;
    SvXMLItemMapEntry* getByIndex( sal_uInt16 nIndex ) const;

    sal_uInt16 getCount() const;
};

SV_DECL_REF( SvXMLItemMapEntries )
SV_IMPL_REF( SvXMLItemMapEntries )


} //namespace binfilter
#endif	//  _XMLITMAP_HXX
