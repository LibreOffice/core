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

#ifndef INCLUDED_SC_SOURCE_UI_INC_HFEDTDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_HFEDTDLG_HXX

#include <sal/config.h>

#include <string_view>

#include <sfx2/tabdlg.hxx>
#include <editeng/svxenum.hxx>

class ScHFEditDlg : public SfxTabDialogController
{
    SvxNumType eNumType;
protected:
    ScHFEditDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle,
        const OUString& rUIXMLDescription, const OString& rID);
public:
    virtual void PageCreated(const OString& rId, SfxTabPage& rPage) override;
};

class ScHFEditHeaderDlg : public ScHFEditDlg
{
public:
    ScHFEditHeaderDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditFooterDlg : public ScHFEditDlg
{
public:
    ScHFEditFooterDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditLeftHeaderDlg : public ScHFEditDlg
{
public:
    ScHFEditLeftHeaderDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditRightHeaderDlg : public ScHFEditDlg
{
public:
    ScHFEditRightHeaderDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditLeftFooterDlg : public ScHFEditDlg
{
public:
    ScHFEditLeftFooterDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditRightFooterDlg : public ScHFEditDlg
{
public:
    ScHFEditRightFooterDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditSharedHeaderDlg : public ScHFEditDlg
{
public:
    ScHFEditSharedHeaderDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditSharedFooterDlg : public ScHFEditDlg
{
public:
    ScHFEditSharedFooterDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditAllDlg : public ScHFEditDlg
{
public:
    ScHFEditAllDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

class ScHFEditActiveDlg : public ScHFEditDlg
{
public:
    ScHFEditActiveDlg(weld::Window* pParent,
        const SfxItemSet& rCoreSet, std::u16string_view rPageStyle);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_HFEDTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
