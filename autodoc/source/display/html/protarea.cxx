/*************************************************************************
 *
 *  $RCSfile: protarea.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 13:33:06 $
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

ProtectionArea::ProtectionArea( const char *        i_sLabel,
                                const char *        i_sTitle_class,
                                const char *        i_sTitle_struct,
                                const char *        i_sTitle_union )
    :   pSglTable(0),
        aClassesTables(),
        sLabel(i_sLabel)
{
    aClassesTables[ Index(ary::cpp::CK_class) ]
                = new S_Slot_Table(i_sTitle_class);
    aClassesTables[ Index(ary::cpp::CK_struct) ]
                = new S_Slot_Table(i_sTitle_struct);
    aClassesTables[ Index(ary::cpp::CK_union) ]
                = new S_Slot_Table(i_sTitle_union);
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

bool
ProtectionArea::WasUsed_Table() const
{
    return pSglTable->WasUsed();
}

bool
ProtectionArea::WasUsed_Table( ary::cpp::E_ClassKey i_eClassKey ) const
{
    return aClassesTables[Index(i_eClassKey)]->WasUsed();
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



