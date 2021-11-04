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
#ifndef INCLUDED_SFX2_NEW_HXX
#define INCLUDED_SFX2_NEW_HXX

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/basedlgs.hxx>
#include <sfx2/objsh.hxx>

#include <vcl/idle.hxx>
#include <sfx2/doctempl.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace weld { class Button; }
namespace weld { class CheckButton; }
namespace weld { class CustomWeld; }
namespace weld { class Expander; }
namespace weld { class Label; }
namespace weld { class TreeView; }
namespace weld { class Window; }

class SfxPreviewWin_Impl;

enum class SfxNewFileDialogMode {
    NONE, Preview, LoadTemplate
};

enum class SfxTemplateFlags
{
    NONE                = 0x00,
    LOAD_TEXT_STYLES    = 0x01,
    LOAD_FRAME_STYLES   = 0x02,
    LOAD_PAGE_STYLES    = 0x04,
    LOAD_NUM_STYLES     = 0x08,
    MERGE_STYLES        = 0x10,
};
namespace o3tl
{
    template<> struct typed_flags<SfxTemplateFlags> : is_typed_flags<SfxTemplateFlags, 0x1f> {};
}

#define RET_TEMPLATE_LOAD       100

class SFX2_DLLPUBLIC SfxNewFileDialog final : public SfxDialogController
{
private:
    Idle m_aPrevIdle;
    SfxNewFileDialogMode m_nFlags;
    SfxDocumentTemplates m_aTemplates;
    SfxObjectShellLock m_xDocShell;

    std::unique_ptr<SfxPreviewWin_Impl> m_xPreviewController;

    std::unique_ptr<weld::TreeView> m_xRegionLb;
    std::unique_ptr<weld::TreeView> m_xTemplateLb;
    std::unique_ptr<weld::CheckButton> m_xTextStyleCB;
    std::unique_ptr<weld::CheckButton> m_xFrameStyleCB;
    std::unique_ptr<weld::CheckButton> m_xPageStyleCB;
    std::unique_ptr<weld::CheckButton> m_xNumStyleCB;
    std::unique_ptr<weld::CheckButton> m_xMergeStyleCB;
    std::unique_ptr<weld::Button> m_xLoadFilePB;
    std::unique_ptr<weld::Expander> m_xMoreBt;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWin;
    std::unique_ptr<weld::Label> m_xAltTitleFt;

    DECL_DLLPRIVATE_LINK( Update, Timer *, void );

    DECL_DLLPRIVATE_LINK(RegionSelect, weld::TreeView&, void);
    DECL_DLLPRIVATE_LINK(TemplateSelect, weld::TreeView&, void);
    DECL_DLLPRIVATE_LINK(DoubleClick, weld::TreeView&, bool);
    DECL_DLLPRIVATE_LINK(Expand, weld::Expander&, void);
    sal_uInt16  GetSelectedTemplatePos() const;

public:
    SfxNewFileDialog(weld::Window *pParent, SfxNewFileDialogMode nFlags);
    virtual ~SfxNewFileDialog() override;

    // Returns false, when '- No -' is set as Template
    // Template names can only be obtained when IsTemplate() returns true.
    bool IsTemplate() const;
    OUString GetTemplateFileName() const;

    // load template methods
    SfxTemplateFlags  GetTemplateFlags() const;
    void              SetTemplateFlags(SfxTemplateFlags nSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
