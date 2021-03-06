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

#include <sfx2/objsh.hxx>
#include <tools/solar.h>
#include <vcl/weld.hxx>

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class ScDocShell;
class SvtURLBox;

class ScLinkedAreaDlg : public weld::GenericDialogController
{
private:
    ScDocShell*             m_pSourceShell;
    std::unique_ptr<sfx2::DocumentInserter> m_xDocInserter;
    SfxObjectShellRef   aSourceRef;

    std::unique_ptr<SvtURLBox> m_xCbUrl;
    std::unique_ptr<weld::Button> m_xBtnBrowse;
    std::unique_ptr<weld::TreeView> m_xLbRanges;
    std::unique_ptr<weld::CheckButton> m_xBtnReload;
    std::unique_ptr<weld::SpinButton> m_xNfDelay;
    std::unique_ptr<weld::Label> m_xFtSeconds;
    std::unique_ptr<weld::Button> m_xBtnOk;

    DECL_LINK(FileHdl, weld::ComboBox&, bool);
    DECL_LINK(BrowseHdl, weld::Button&, void);
    DECL_LINK(RangeHdl, weld::TreeView&, void);
    DECL_LINK(ReloadHdl, weld::Button&, void);
    DECL_LINK(DialogClosedHdl, sfx2::FileDialogHelper*, void);

    void                UpdateSourceRanges();
    void                UpdateEnable();
    void                LoadDocument( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions );

public:
    ScLinkedAreaDlg(weld::Widget* pParent);
    virtual ~ScLinkedAreaDlg() override;

    void            InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh );

    OUString       GetURL() const;
    OUString       GetFilter() const;        // may be empty
    OUString       GetOptions() const;       // filter options
    OUString       GetSource() const;        // separated by ";"
    sal_uLong      GetRefresh() const;       // 0 if disabled
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
