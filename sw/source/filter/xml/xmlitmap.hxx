/*************************************************************************
 *
 *  $RCSfile: xmlitmap.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2001-07-04 14:15:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <xmloff/xmltoken.hxx>
#endif

namespace rtl { class OUString; }

#define MID_FLAG_MASK                   0x0000ffff

// this flags are used in the item mapper for import and export

#define MID_FLAG_SPECIAL_ITEM_IMPORT    0x80000000
#define MID_FLAG_NO_ITEM_IMPORT         0x40000000
#define MID_FLAG_SPECIAL_ITEM_EXPORT    0x20000000
#define MID_FLAG_NO_ITEM_EXPORT         0x10000000
#define MID_FLAG_SPECIAL_ITEM           0xa0000000 // both import and export
#define MID_FLAG_NO_ITEM                0x50000000 // both import and export
#define MID_FLAG_ELEMENT_ITEM_IMPORT    0x08000000
#define MID_FLAG_ELEMENT_ITEM_EXPORT    0x04000000
#define MID_FLAG_ELEMENT_ITEM           0x0c000000  // both import and export

// ---

struct SvXMLItemMapEntry
{
    sal_uInt16 nNameSpace;      // declares the Namespace in wich this item
                                // exists
    enum ::xmloff::token::XMLTokenEnum eLocalName;
                                // the local name for the item inside
                                // the Namespace (as an XMLTokenEnum)
    sal_uInt32 nWhichId;        // the WichId to identify the item
                                // in the pool
    sal_uInt32 nMemberId;       // the memberid specifies wich part
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


#endif  //  _XMLITMAP_HXX
