/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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



