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


#ifndef _SWDDETBL_HXX
#define _SWDDETBL_HXX

#include "swtable.hxx"

class SwDDEFieldType;

class SwDDETable : public SwTable
{
    SwDepend aDepend;
public:
    // Constructor movet alle Lines/Boxen aus der SwTable zu sich.
    // Die SwTable ist danach Leer und muss geloescht werden.
    SwDDETable( SwTable& rTable, SwDDEFieldType* pDDEType,
                sal_Bool bUpdate = sal_True );
    ~SwDDETable();

    void ChangeContent();
    sal_Bool  NoDDETable();

    SwDDEFieldType* GetDDEFldType();
    inline const SwDDEFieldType* GetDDEFldType() const;
protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );
    virtual void SwClientNotify( const SwModify&, const SfxHint& );
};


// ----------- Inlines -----------------------------

inline const SwDDEFieldType* SwDDETable::GetDDEFldType() const
{
    return ((SwDDETable*)this)->GetDDEFldType();
}

#endif
