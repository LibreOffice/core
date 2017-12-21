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

#ifndef INCLUDED_SVX_LISTCTRL_HXX
#define INCLUDED_SVX_LISTCTRL_HXX

#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>

#include <vector>
#include <limits>

#include <vcl/dllapi.h>

class VCL_DLLPUBLIC ListControl : public Control
{
private:
    std::vector<VclPtr<vcl::Window>> maEntries;
    bool mbHasScrollBar;
    VclPtr<ScrollBar> mpScrollBar;

    void DoScroll(long nDiff);
    void RecalcAll();
public:

    ListControl(vcl::Window* pParent, WinBits nStyle);
    virtual ~ListControl() override;
    virtual void dispose() override;

    void addEntry(VclPtr<vcl::Window> xEntry, sal_uInt32 nPos = std::numeric_limits<sal_uInt16>::max());
    std::vector<VclPtr<vcl::Window>> const & getEntries() const;
    void deleteEntry(sal_uInt32 nPos);

    virtual Size GetOptimalSize() const override;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual void Resize() override;
    virtual bool EventNotify( NotifyEvent& rNEvt ) override;

    DECL_LINK( ScrollHdl, ScrollBar*, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
