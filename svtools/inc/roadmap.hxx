/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVTOOLS_INC_ROADMAP_HXX
#define INCLUDED_SVTOOLS_INC_ROADMAP_HXX

#include <svtools/svtdllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/imgctrl.hxx>

#include <svtools/hyperlabel.hxx>

class Bitmap;

namespace svt
{

struct RoadmapTypes
{
public:
    typedef sal_Int16 ItemId;
    typedef sal_Int32 ItemIndex;
};

class RoadmapImpl;
class RoadmapItem;

class SVT_DLLPUBLIC ORoadmap : public Control, public RoadmapTypes
{
protected:
    RoadmapImpl*    m_pImpl;
    // Window overridables
    void            Paint(vcl::RenderContext& rRenderContext, const Rectangle& _rRect) override;
    void            implInit(vcl::RenderContext& rRenderContext);

public:
    ORoadmap( vcl::Window* _pParent, WinBits _nWinStyle = 0 );
    virtual ~ORoadmap( );
    virtual void dispose() override;

    void            SetRoadmapBitmap( const BitmapEx& maBitmap );

    void            EnableRoadmapItem( ItemId _nItemId, bool _bEnable );

    void            ChangeRoadmapItemLabel( ItemId _nID, const OUString& sLabel );
    void            ChangeRoadmapItemID( ItemId _nID, ItemId NewID  );

    void            SetRoadmapInteractive( bool _bInteractive );
    bool            IsRoadmapInteractive();

    void            SetRoadmapComplete( bool _bComplete );
    bool            IsRoadmapComplete() const;

    ItemIndex       GetItemCount() const;
    ItemId          GetItemID( ItemIndex _nIndex ) const;

    void            InsertRoadmapItem( ItemIndex Index, const OUString& RoadmapItem, ItemId _nUniqueId, bool _bEnabled );
    void            ReplaceRoadmapItem( ItemIndex Index, const OUString& RoadmapItem, ItemId _nUniqueId, bool _bEnabled );
    void            DeleteRoadmapItem( ItemIndex _nIndex );

    ItemId          GetCurrentRoadmapItemID() const;
    bool            SelectRoadmapItemByID( ItemId _nItemID );

    void            SetItemSelectHdl( const Link<LinkParamNone*,void>& _rHdl );
    Link<LinkParamNone*,void> GetItemSelectHdl( ) const;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void    GetFocus() override;
    virtual void    ApplySettings( vcl::RenderContext& rRenderContext ) override;

protected:
    bool            PreNotify( NotifyEvent& rNEvt ) override;

protected:
    /// called when an item has been selected by any means
    void    Select();

private:
    DECL_LINK_TYPED(ImplClickHdl, HyperLabel*, void);

    RoadmapItem*         GetByIndex( ItemIndex _nItemIndex );
    const RoadmapItem*   GetByIndex( ItemIndex _nItemIndex ) const;

    RoadmapItem*         GetByID( ItemId _nID  );
    const RoadmapItem*   GetByID( ItemId _nID  ) const;
    RoadmapItem*         GetPreviousHyperLabel( ItemIndex Index);

    void                 DrawHeadline(vcl::RenderContext& rRenderContext);
    void                 DeselectOldRoadmapItems();
    ItemId               GetNextAvailableItemId( ItemIndex NewIndex );
    ItemId               GetPreviousAvailableItemId( ItemIndex NewIndex );
    RoadmapItem*         GetByPointer(vcl::Window* pWindow);
    RoadmapItem*         InsertHyperLabel( ItemIndex Index, const OUString& _aStr, ItemId RMID, bool _bEnabled, bool _bIncomplete  );
    void                 UpdatefollowingHyperLabels( ItemIndex Index );
};

}   // namespace svt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
