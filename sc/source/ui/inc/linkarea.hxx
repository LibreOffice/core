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

#ifndef INCLUDED_SC_SOURCE_UI_INC_LINKAREA_HXX
#define INCLUDED_SC_SOURCE_UI_INC_LINKAREA_HXX

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/inettbc.hxx>

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class ScDocShell;

class ScLinkedAreaDlg : public ModalDialog
{
private:
    VclPtr<SvtURLBox>    m_pCbUrl;
    VclPtr<PushButton>   m_pBtnBrowse;
    VclPtr<ListBox>      m_pLbRanges;
    VclPtr<CheckBox>     m_pBtnReload;
    VclPtr<NumericField> m_pNfDelay;
    VclPtr<FixedText>    m_pFtSeconds;
    VclPtr<OKButton>     m_pBtnOk;

    ScDocShell*             pSourceShell;
    sfx2::DocumentInserter* pDocInserter;

    SfxObjectShellRef   aSourceRef;

    DECL_LINK(FileHdl, void *);
    DECL_LINK_TYPED(BrowseHdl, Button*, void);
    DECL_LINK_TYPED(RangeHdl, ListBox&, void);
    DECL_LINK_TYPED(ReloadHdl, Button*, void);
    DECL_LINK_TYPED( DialogClosedHdl, sfx2::FileDialogHelper*, void );

    void                UpdateSourceRanges();
    void                UpdateEnable();
    void                LoadDocument( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions );

public:
            ScLinkedAreaDlg( vcl::Window* pParent );
            virtual ~ScLinkedAreaDlg();
    virtual void dispose() SAL_OVERRIDE;

    void            InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh );

    virtual short   Execute() SAL_OVERRIDE;          // overwritten to set dialog parent

    OUString       GetURL();
    OUString       GetFilter();        // may be empty
    OUString       GetOptions();       // filter options
    OUString       GetSource();        // separated by ";"
    sal_uLong           GetRefresh();       // 0 if disabled
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
