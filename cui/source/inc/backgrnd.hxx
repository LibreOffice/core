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

#include <memory>

#include "cuitabarea.hxx"

class BackgroundPreviewImpl;
class SvxOpenGraphicDialog;
class SvxBrushItem;

/** class SvxBackgroundTabPage --------------------------------------------

    [Description]
    With this TabPage a Brush (e. g. for a frame's background color)
    can be set.
    [Items]
    <SvxBrushItem>:     <SID_ATTR_BRUSH>;
*/

class SvxBkgTabPage : public SvxAreaTabPage
{
    static const sal_uInt16 pPageRanges[];

    std::unique_ptr<weld::ComboBox> m_xTblLBox;
    bool        bHighlighting       : 1;
    bool        bCharBackColor      : 1;
    SfxItemSet maSet;
    std::unique_ptr<SfxItemSet> m_pResetSet;

    sal_Int32 m_nActPos = -1;

    DECL_LINK(TblDestinationHdl_Impl, weld::ComboBox&, void);
public:
    SvxBkgTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~SvxBkgTabPage() override;

    // returns the area of the which-values
    static const sal_uInt16* GetRanges() { return pPageRanges; }

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PageCreated( const SfxAllItemSet& aSet ) override;
    virtual void Reset( const SfxItemSet * ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
