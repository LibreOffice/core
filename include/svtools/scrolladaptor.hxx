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

#pragma once

#include <svtools/svtdllapi.h>

#include <vcl/InterimItemWindow.hxx>
#include <vcl/scrollable.hxx>

class SVT_DLLPUBLIC ScrollAdaptor : public InterimItemWindow, public Scrollable
{
protected:
    std::unique_ptr<weld::Scrollbar> m_xScrollBar;
    Link<weld::Scrollbar&, void> m_aLink;
    bool m_bHori;

public:
    virtual void SetRange(const Range& rRange) override;
    virtual Range GetRange() const override;

    virtual void SetRangeMin(tools::Long nNewRange) override;
    virtual tools::Long GetRangeMin() const override;

    virtual void SetRangeMax(tools::Long nNewRange) override;
    virtual tools::Long GetRangeMax() const override;

    virtual void SetLineSize(tools::Long nNewSize) override;
    virtual tools::Long GetLineSize() const override;

    virtual void SetPageSize(tools::Long nNewSize) override;
    virtual tools::Long GetPageSize() const override;

    virtual void SetVisibleSize(tools::Long nNewSize) override;
    virtual tools::Long GetVisibleSize() const override;

    virtual void SetThumbPos(tools::Long nThumbPos) override;
    virtual tools::Long GetThumbPos() const override;

    void SetScrollHdl(const Link<weld::Scrollbar&, void>& rLink);
    void SetMouseReleaseHdl(const Link<const MouseEvent&, bool>& rLink);

    // what is it
    bool IsHoriScroll() const { return m_bHori; }

    ScrollType GetScrollType() const;

    virtual void EnableRTL(bool bEnable = true) override;

    virtual tools::Long DoScroll(tools::Long nNewPos) override;

    virtual bool Inactive() const override { return !m_xScrollBar->get_sensitive(); }

    bool HasGrab() const { return m_xScrollBar->has_grab(); }

    void SetThickness(int nThickness);

    void SetSwapArrows(bool bSwap = true);

    ScrollAdaptor(vcl::Window* pParent, bool bHori);
    virtual void dispose() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
