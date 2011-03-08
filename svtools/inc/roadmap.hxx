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
#ifndef _SVTOOLS_ROADMAP_HXX
#define _SVTOOLS_ROADMAP_HXX

#include "svtools/svtdllapi.h"
#include <vcl/ctrl.hxx>
#include <vcl/imgctrl.hxx>

#include "svtools/hyperlabel.hxx"




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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
