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
#ifndef _OPTCOMP_HXX
#define _OPTCOMP_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <unotools/compatibility.hxx>
#include <svx/checklbx.hxx>

// class SwCompatibilityOptPage ------------------------------------------
class SwWrtShell;
struct SwCompatibilityOptPage_Impl;

class SwCompatibilityOptPage : public SfxTabPage
{
private:
    // controls
    FixedLine               m_aMainFL;
    FixedText               m_aFormattingFT;
    ListBox                 m_aFormattingLB;
    FixedText               m_aOptionsFT;
    SvxCheckListBox         m_aOptionsLB;
    PushButton              m_aResetPB;
    PushButton              m_aDefaultPB;
    // config item
    SvtCompatibilityOptions m_aConfigItem;
    // text of the user entry
    String                  m_sUserEntry;
    // shell of the current document
    SwWrtShell*             m_pWrtShell;
    // impl object
    SwCompatibilityOptPage_Impl* m_pImpl;
    // saved options after "Reset"; used in "FillItemSet" for comparison
    sal_uLong                   m_nSavedOptions;

    // handler
    DECL_LINK(SelectHdl, void *);
    DECL_LINK(UseAsDefaultHdl, void *);

    // private methods
    void                    InitControls( const SfxItemSet& rSet );
    void                    ReplaceFormatName( String& rEntry );
    void                    SetCurrentOptions( sal_uLong nOptions );
    sal_uLong                   GetDocumentOptions() const;
    void                    WriteOptions();

public:
    SwCompatibilityOptPage( Window* pParent, const SfxItemSet& rSet );
    ~SwCompatibilityOptPage();

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
