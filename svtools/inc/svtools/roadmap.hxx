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


#ifndef _SVTOOLS_ROADMAP_HXX
#define _SVTOOLS_ROADMAP_HXX

#include "svtools/svtdllapi.h"
#include <vcl/ctrl.hxx>
#include <vcl/imgctrl.hxx>

#ifndef _SVTOOLS_HYPERLABEL_HXX
#include "svtools/hyperlabel.hxx"
#endif




class Bitmap;
//.........................................................................
namespace svt
{
//.........................................................................

    struct RoadmapTypes
    {
    public:
        typedef sal_Int16 ItemId;
        typedef sal_Int32 ItemIndex;
    };

    class RoadmapImpl;
    class RoadmapItem;

    //=====================================================================
    //= Roadmap
    //=====================================================================
    class SVT_DLLPUBLIC ORoadmap : public Control, public RoadmapTypes
    {
    protected:
        RoadmapImpl*    m_pImpl;
        // Window overridables
        void            Paint( const Rectangle& _rRect );
        void            implInit();

    public:
        ORoadmap( Window* _pParent, const ResId& _rId );
        ORoadmap( Window* _pParent, WinBits _nWinStyle = 0 );
        ~ORoadmap( );

        void            SetRoadmapBitmap( const BitmapEx& maBitmap, sal_Bool _bInvalidate = sal_True );
        const BitmapEx& GetRoadmapBitmap( ) const;

        void            EnableRoadmapItem( ItemId _nItemId, sal_Bool _bEnable, ItemIndex _nStartIndex = 0  );
        sal_Bool        IsRoadmapItemEnabled( ItemId _nItemId, ItemIndex _nStartIndex = 0 ) const;

        void            ChangeRoadmapItemLabel( ItemId _nID, const ::rtl::OUString& sLabel, ItemIndex _nStartIndex = 0 );
        ::rtl::OUString GetRoadmapItemLabel( ItemId _nID, ItemIndex _nStartIndex = 0 );
        void            ChangeRoadmapItemID( ItemId _nID, ItemId _NewID, ItemIndex _nStartIndex = 0  );

        void            SetRoadmapInteractive( sal_Bool _bInteractive );
        sal_Bool        IsRoadmapInteractive();

        void            SetRoadmapComplete( sal_Bool _bComplete );
        sal_Bool        IsRoadmapComplete() const;

        ItemIndex       GetItemCount() const;
        ItemId          GetItemID( ItemIndex _nIndex ) const;
        ItemIndex       GetItemIndex( ItemId _nID ) const;

        void            InsertRoadmapItem( ItemIndex _Index, const ::rtl::OUString& _RoadmapItem, ItemId _nUniqueId, sal_Bool _bEnabled = sal_True );
        void            ReplaceRoadmapItem( ItemIndex _Index, const ::rtl::OUString& _RoadmapItem, ItemId _nUniqueId, sal_Bool _bEnabled );
        void            DeleteRoadmapItem( ItemIndex _nIndex );

        ItemId          GetCurrentRoadmapItemID() const;
        sal_Bool        SelectRoadmapItemByID( ItemId _nItemID );

        void            SetItemSelectHdl( const Link& _rHdl );
        Link            GetItemSelectHdl( ) const;
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );
        virtual void    GetFocus();


    protected:
        long            PreNotify( NotifyEvent& rNEvt );

    protected:
        /// called when an item has been selected by any means
        virtual void    Select();

    private:
        DECL_LINK(ImplClickHdl, HyperLabel*);

        RoadmapItem*         GetByIndex( ItemIndex _nItemIndex );
        const RoadmapItem*   GetByIndex( ItemIndex _nItemIndex ) const;

        RoadmapItem*         GetByID( ItemId _nID, ItemIndex _nStartIndex = 0  );
        const RoadmapItem*   GetByID( ItemId _nID, ItemIndex _nStartIndex = 0  ) const;
        RoadmapItem*            GetPreviousHyperLabel( ItemIndex _Index);

        void                        DrawHeadline();
        void                        DeselectOldRoadmapItems();
        ItemId                      GetNextAvailableItemId( ItemIndex _NewIndex );
        ItemId                      GetPreviousAvailableItemId( ItemIndex _NewIndex );
        RoadmapItem*                GetByPointer(Window* pWindow);
        RoadmapItem*                InsertHyperLabel( ItemIndex _Index, const ::rtl::OUString& _aStr, ItemId _RMID, sal_Bool _bEnabled = sal_True  );
        void                        UpdatefollowingHyperLabels( ItemIndex _Index );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif

