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
    // text of the "use as default" querybox
    String                  m_sUseAsDefaultQuery;
    // shell of the current document
    SwWrtShell*             m_pWrtShell;
    // impl object
    SwCompatibilityOptPage_Impl* m_pImpl;
    // saved options after "Reset"; used in "FillItemSet" for comparison
    sal_uLong                   m_nSavedOptions;

    // handler
    DECL_LINK(              SelectHdl, ListBox* );
    DECL_LINK(              UseAsDefaultHdl, PushButton* );

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
