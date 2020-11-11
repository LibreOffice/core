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
#ifndef INCLUDED_SW_INC_POSTITHELPER_HXX
#define INCLUDED_SW_INC_POSTITHELPER_HXX

#include "swrect.hxx"
#include "fmtfld.hxx"
#include <cstddef>
#include <vcl/vclptr.hxx>
#include <tools/solar.h>
#include "SidebarWindowsTypes.hxx"

class SfxBroadcaster;
class SwRootFrame;
class SwPostItMgr;
class SwEditWin;
class SwFrame;
class IDocumentRedlineAccess;
namespace sw::annotation { class SwAnnotationWin; }
namespace sw::mark { class IMark; }

struct SwPosition;

struct SwLayoutInfo
{
    const SwFrame* mpAnchorFrame;
    SwRect mPosition;

    // optional start of the annotation
    sal_uLong mnStartNodeIdx;
    sal_Int32 mnStartContent;

    SwRect mPageFrame;
    SwRect mPagePrtArea;
    tools::ULong mnPageNumber;

    sw::sidebarwindows::SidebarPosition meSidebarPosition;

    std::size_t mRedlineAuthor;

    /**
     * If true, the overlay arrow points to the comment anchor, otherwise it points to the commented
     * frame.
     */
    bool mPositionFromCommentAnchor;

    SwLayoutInfo()
        : mpAnchorFrame(nullptr)
        , mPosition()
        , mnStartNodeIdx( 0 )
        , mnStartContent( -1 )
        , mPageFrame()
        , mPagePrtArea()
        , mnPageNumber(1)
        , meSidebarPosition(sw::sidebarwindows::SidebarPosition::NONE)
        , mRedlineAuthor(0)
        , mPositionFromCommentAnchor(true)
    {}
};

namespace SwPostItHelper
{
    enum SwLayoutStatus
    {
        INVISIBLE, VISIBLE, INSERTED, DELETED, NONE, HIDDEN
    };

    SwLayoutStatus getLayoutInfos(
        SwLayoutInfo& o_rInfo,
        const SwPosition& rAnchorPos,
        const sw::mark::IMark* pAnnotationMark = nullptr );

    tools::Long getLayoutHeight( const SwRootFrame* pRoot );
    void setSidebarChanged( SwRootFrame* pRoot, bool bBrowseMode );
    tools::ULong getPageInfo( SwRect& rPageFrame, const SwRootFrame* , const Point& );
}

class SAL_DLLPUBLIC_RTTI SwSidebarItem
{
public:
    VclPtr<sw::annotation::SwAnnotationWin> mpPostIt;
    bool mbShow;
    bool mbFocus;
    bool mbPendingLayout;

    SwPostItHelper::SwLayoutStatus mLayoutStatus;
    SwLayoutInfo maLayoutInfo;

    SwSidebarItem( const bool aFocus);

    virtual ~SwSidebarItem();

    SwSidebarItem(SwSidebarItem const &) = default;
    SwSidebarItem(SwSidebarItem &&) = default;
    SwSidebarItem & operator =(SwSidebarItem const &) = default;
    SwSidebarItem & operator =(SwSidebarItem &&) = default;

    virtual SwPosition GetAnchorPosition() const = 0;
    virtual bool UseElement(SwRootFrame const&, IDocumentRedlineAccess const&) = 0;
    virtual const SwFormatField& GetFormatField() const = 0;
    virtual const SfxBroadcaster* GetBroadcaster() const = 0;
    virtual VclPtr<sw::annotation::SwAnnotationWin> GetSidebarWindow( SwEditWin& rEditWin,
                                                                SwPostItMgr& aMgr) = 0;
};

class SwAnnotationItem final : public SwSidebarItem
{
public:
    SwAnnotationItem(
        SwFormatField& rFormatField,
        const bool aFocus)
        : SwSidebarItem( aFocus )
        , mrFormatField( rFormatField )
    {
    }

    virtual SwPosition GetAnchorPosition() const override;
    virtual bool UseElement(SwRootFrame const&, IDocumentRedlineAccess const&) override;
    virtual const SwFormatField& GetFormatField() const override
    {
        return mrFormatField;
    }
    virtual const SfxBroadcaster* GetBroadcaster() const override
    {
        return dynamic_cast<const SfxBroadcaster *> (&mrFormatField);
    }
    virtual VclPtr<sw::annotation::SwAnnotationWin> GetSidebarWindow(
        SwEditWin& rEditWin,
        SwPostItMgr& aMgr ) override;

private:
    SwFormatField& mrFormatField;
};

#endif // INCLUDED_SW_INC_POSTITHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
