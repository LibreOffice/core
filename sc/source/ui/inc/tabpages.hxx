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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TABPAGES_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TABPAGES_HXX

#include <vcl/group.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/tabdlg.hxx>

class ScTabPageProtection : public SfxTabPage
{
    friend class VclPtr<ScTabPageProtection>;
    static const sal_uInt16 pProtectionRanges[];
public:
    static  VclPtr<SfxTabPage> Create          ( vcl::Window*               pParent,
                                          const SfxItemSet*     rAttrSet );
    static  const sal_uInt16* GetRanges () { return pProtectionRanges; }
    virtual bool        FillItemSet     ( SfxItemSet* rCoreAttrs ) override;
    virtual void        Reset           ( const SfxItemSet* ) override;

    virtual ~ScTabPageProtection();
    virtual void dispose() override;

protected:
    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:
                ScTabPageProtection( vcl::Window*            pParent,
                                     const SfxItemSet&  rCoreAttrs );
private:
    VclPtr<TriStateBox>    m_pBtnHideCell;
    VclPtr<TriStateBox>    m_pBtnProtect;
    VclPtr<TriStateBox>    m_pBtnHideFormula;
    VclPtr<TriStateBox>    m_pBtnHidePrint;
                                        // current status:
    bool            bTriEnabled;        //  if before - DontCare
    bool            bDontCare;          //  all in  TriState
    bool            bProtect;           //  secure individual settings for TriState
    bool            bHideForm;
    bool            bHideCell;
    bool            bHidePrint;

    // Handler:
    DECL_LINK_TYPED( ButtonClickHdl, Button*, void );
    void        UpdateButtons();
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TABPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
