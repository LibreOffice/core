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

#include <precomp.h>
#include "protarea.hxx"


// NOT FULLY DEFINED SERVICES
#include "hdimpl.hxx"


inline UINT8
ProtectionArea::Index( ary::cpp::E_ClassKey i_eClassKey ) const
{
     return i_eClassKey == ary::cpp::CK_class
                ?   0
                :   i_eClassKey == ary::cpp::CK_struct
                        ?   1
                        :   2;
}



ProtectionArea::ProtectionArea( const char *        i_sLabel,
                                const char *        i_sTitle )
    :   pSglTable( new S_Slot_Table(i_sTitle) ),
        aClassesTables(),
        sLabel(i_sLabel)
{
}

ProtectionArea::~ProtectionArea()
{

}

csi::html::Table &
ProtectionArea::GetTable()
{
    csv_assert(pSglTable);

    return pSglTable->GetTable();
}

csi::html::Table &
ProtectionArea::GetTable( ary::cpp::E_ClassKey i_eClassKey )
{
    csv_assert(aClassesTables[Index(i_eClassKey)]);
    return aClassesTables[Index(i_eClassKey)]->GetTable();
}

DYN csi::html::Table *
ProtectionArea::ReleaseTable()
{
    csv_assert(pSglTable);
    return pSglTable->ReleaseTable();
}

DYN csi::html::Table *
ProtectionArea::ReleaseTable( ary::cpp::E_ClassKey i_eClassKey )
{
    csv_assert(aClassesTables[Index(i_eClassKey)]);
    return aClassesTables[Index(i_eClassKey)]->ReleaseTable();
}

const char *
ProtectionArea::Label() const
{
    return sLabel;
}


bool
ProtectionArea::WasUsed_Area() const
{
    if ( pSglTable )
    {
         return pSglTable->WasUsed();
    }

    typedef const Dyn<ProtectionArea::S_Slot_Table> cdyntab;

    // Workaround a maybe compiler bug in Solaris5-CC ?
    //   should normally work without the cast,
    //   because that is exactly the genuine type, given:
    return static_cast< cdyntab& >(aClassesTables[0])->WasUsed()
           OR static_cast< cdyntab& >(aClassesTables[1])->WasUsed()
           OR static_cast< cdyntab& >(aClassesTables[2])->WasUsed();
}

//*******************        S_Slot_Table        **********************//

ProtectionArea::
S_Slot_Table::S_Slot_Table(const char * i_sTitle)
    :   sTableTitle(i_sTitle)
{
}

ProtectionArea::
S_Slot_Table::~S_Slot_Table()
{
}

csi::html::Table &
ProtectionArea::
S_Slot_Table::GetTable()
{
    return pTable
                ?   *pTable
                :   *( pTable = &Create_ChildListTable(sTableTitle) );
}



