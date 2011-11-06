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



#ifndef _SFX_PICKLIST_HXX_
#define _SFX_PICKLIST_HXX_

#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <vcl/menu.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/util/XStringWidth.hpp>

#include <vector>

#define PICKLIST_MAXSIZE  100

class SfxPickList : public SfxListener
{
    struct PickListEntry
    {
        PickListEntry( const String& _aName, const String& _aFilter, const String& _aTitle ) :
            aName( _aName ), aFilter( _aFilter ), aTitle( _aTitle ) {}

        String      aName;
        String      aFilter;
        String      aTitle;
        String      aOptions;
    };

    static SfxPickList*             pUniqueInstance;
    static osl::Mutex*              pMutex;

    std::vector< PickListEntry* >   m_aPicklistVector;
    sal_uInt32                      m_nAllowedMenuSize;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringWidth > m_xStringLength;

                            SfxPickList( sal_uInt32 nMenuSize );
                            ~SfxPickList();

    static osl::Mutex*      GetOrCreateMutex();

    void                    CreatePicklistMenuTitle( Menu* pMenu, sal_uInt16 nItemId, const String& aURL, sal_uInt32 nNo );
    PickListEntry*          GetPickListEntry( sal_uInt32 nIndex );
    void                    CreatePickListEntries();
    void                    RemovePickListEntries();

    public:
        static SfxPickList* GetOrCreate( const sal_uInt32 nMenuSize );
        static SfxPickList* Get();
        static void         Delete();

        sal_uInt32          GetAllowedMenuSize() { return m_nAllowedMenuSize; }
        sal_uInt32          GetNumOfEntries() const { return m_aPicklistVector.size(); }
        void                CreateMenuEntries( Menu* pMenu );
        void                ExecuteMenuEntry( sal_uInt16 nId );
        void                ExecuteEntry( sal_uInt32 nIndex );
        String              GetMenuEntryTitle( sal_uInt32 nIndex );

        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif // _SFX_PICKLIST_HXX_
