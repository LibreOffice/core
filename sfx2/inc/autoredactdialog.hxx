/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_INC_AUTOREDACTDIALOG_HXX
#define INCLUDED_SFX2_INC_AUTOREDACTDIALOG_HXX

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/basedlgs.hxx>
#include <sfx2/objsh.hxx>

#include <vcl/idle.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <svtools/simptabl.hxx>

enum RedactionTargetType
{
    REDACTION_TARGET_TEXT,
    REDACTION_TARGET_REGEX,
    REDACTION_TARGET_PREDEFINED,
    REDACTION_TARGET_UNKNOWN
};

/// Keeps information for a single redaction target
struct RedactionTarget
{
    OUString sName;
    RedactionTargetType sType;
    OUString sContent;
    bool bCaseSensitive;
    bool bWholeWords;
    sal_uInt32 nID;
};

/// Used to display the targets list
class TargetsTable : public SvSimpleTable
{
    SvTreeListEntry* GetRowByTargetName(const OUString& sName);

public:
    TargetsTable(SvSimpleTableContainer& rParent);
    void InsertTarget(RedactionTarget* pTarget);
    void InsertTargetAtPos(RedactionTarget* pTarget, const sal_uLong& nPos);
    RedactionTarget* GetTargetByName(const OUString& sName);
    OUString GetNameProposal();
};

namespace sfx2
{
class FileDialogHelper;
}

enum class StartFileDialogType
{
    Open,
    SaveAs
};

class SFX2_DLLPUBLIC SfxAutoRedactDialog : public SfxModalDialog
{
    SfxObjectShellLock m_xDocShell;
    std::unique_ptr<sfx2::FileDialogHelper> m_pFileDlg;
    bool m_bIsValidState;
    bool m_bTargetsCopied;

    VclPtr<SvSimpleTableContainer> m_pTargetsContainer;
    VclPtr<TargetsTable> m_pTargetsBox;
    VclPtr<FixedText> m_pRedactionTargetsLabel;
    VclPtr<PushButton> m_pLoadBtn;
    VclPtr<PushButton> m_pSaveBtn;
    VclPtr<PushButton> m_pAddBtn;
    VclPtr<PushButton> m_pEditBtn;
    VclPtr<PushButton> m_pDeleteBtn;

    DECL_LINK(Load, Button*, void);
    DECL_LINK(Save, Button*, void);
    DECL_LINK(AddHdl, Button*, void);
    DECL_LINK(EditHdl, Button*, void);
    DECL_LINK(DeleteHdl, Button*, void);

    DECL_LINK(LoadHdl, sfx2::FileDialogHelper*, void);
    DECL_LINK(SaveHdl, sfx2::FileDialogHelper*, void);

    void StartFileDialog(StartFileDialogType nType, const OUString& rTitle);
    /// Clear all targets both visually and from the memory
    void clearTargets();

public:
    SfxAutoRedactDialog(vcl::Window* pParent);
    virtual ~SfxAutoRedactDialog() override;
    virtual void dispose() override;

    /** Copies targets vector
     *  Does a shallow copy.
     *  Returns true if successful.
     */
    bool getTargets(std::vector<std::pair<RedactionTarget*, OUString>>& r_aTargets);
};

class SfxAddTargetDialog : public SfxModalDialog
{
private:
    VclPtr<Edit> m_pName;
    VclPtr<ListBox> m_pType;
    VclPtr<FixedText> m_pLabelContent;
    VclPtr<Edit> m_pContent;
    VclPtr<FixedText> m_pLabelPredefContent;
    VclPtr<ListBox> m_pPredefContent;
    VclPtr<CheckBox> m_pCaseSensitive;
    VclPtr<CheckBox> m_pWholeWords;

    DECL_LINK(SelectTypeHdl, ListBox&, void);

public:
    SfxAddTargetDialog(vcl::Window* pWindow, const OUString& rName);
    SfxAddTargetDialog(vcl::Window* pWindow, const OUString& sName,
                       const RedactionTargetType& eTargetType, const OUString& sContent,
                       const bool& bCaseSensitive, const bool& bWholeWords);
    virtual ~SfxAddTargetDialog() override;
    virtual void dispose() override;

    OUString getName() const { return m_pName->GetText(); }
    RedactionTargetType getType() const;
    OUString getContent() const;
    bool isCaseSensitive() const { return m_pCaseSensitive->GetState() == TriState::TRISTATE_TRUE; }
    bool isWholeWords() const { return m_pWholeWords->GetState() == TriState::TRISTATE_TRUE; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
