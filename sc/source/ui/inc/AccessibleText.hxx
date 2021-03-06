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

#include <textuno.hxx>
#include <address.hxx>
#include "viewdata.hxx"
#include <editeng/svxenum.hxx>
#include <svl/SfxBroadcaster.hxx>

#include <memory>

class ScDocShell;
class ScViewForwarder;
class ScEditObjectViewForwarder;
class ScPreviewViewForwarder;
class ScEditViewForwarder;
class ScPreviewShell;
class EditTextObject;
class ScCsvViewForwarder;
class ScAccessibleCell;
class ScTextWnd;

class ScAccessibleTextData : public SfxListener
{
public:
                        ScAccessibleTextData() {}

    virtual ScAccessibleTextData* Clone() const = 0;

    virtual void        Notify( SfxBroadcaster& /* rBC */, const SfxHint& /* rHint */ ) override {}

    virtual SvxTextForwarder* GetTextForwarder() = 0;
    virtual SvxViewForwarder* GetViewForwarder() = 0;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) = 0;
    SfxBroadcaster& GetBroadcaster() const { return maBroadcaster; }

    virtual void                UpdateData() = 0;
    DECL_LINK( NotifyHdl, EENotify&, void );

private:
    mutable SfxBroadcaster maBroadcaster;

};

class ScAccessibleCellBaseTextData : public ScAccessibleTextData,
                                     public ScCellTextData
{
public:
                        ScAccessibleCellBaseTextData(ScDocShell* pDocShellP,
                            const ScAddress& rP)
                            : ScCellTextData(pDocShellP, rP) {}
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override { ScCellTextData::Notify(rBC, rHint); }

    virtual void                UpdateData() override { ScCellTextData::UpdateData(); }
};

//  ScAccessibleCellTextData: shared data between sub objects of an accessible cell text object

class ScAccessibleCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessibleCellTextData(ScTabViewShell* pViewShell,
                            const ScAddress& rP, ScSplitPos eSplitPos, ScAccessibleCell* pAccCell);
    virtual             ~ScAccessibleCellTextData() override;

    virtual ScAccessibleTextData* Clone() const override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual SvxTextForwarder* GetTextForwarder() override;
    virtual SvxViewForwarder* GetViewForwarder() override;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) override;

private:
    std::unique_ptr<ScViewForwarder> mpViewForwarder;
    ScTabViewShell* mpViewShell;
    ScSplitPos meSplitPos;
    ScAccessibleCell* mpAccessibleCell;

    using ScAccessibleCellBaseTextData::GetDocShell;
    static ScDocShell* GetDocShell(ScTabViewShell* pViewShell);
};

class ScAccessibleEditObjectTextData : public ScAccessibleTextData
{
public:
    // Add a para to indicate whether the object is cloned
    ScAccessibleEditObjectTextData(EditView* pEditView, OutputDevice* pWin, bool isClone = false);
    virtual             ~ScAccessibleEditObjectTextData() override;

    virtual ScAccessibleTextData* Clone() const override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual SvxTextForwarder* GetTextForwarder() override;
    virtual SvxViewForwarder* GetViewForwarder() override;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) override;

    virtual void                UpdateData() override {  }

    DECL_LINK( NotifyHdl, EENotify&, void );
protected:
    std::unique_ptr<ScEditObjectViewForwarder> mpViewForwarder;
    std::unique_ptr<ScEditViewForwarder>       mpEditViewForwarder;
    EditView*                  mpEditView;
    EditEngine*                mpEditEngine;
    std::unique_ptr<SvxEditEngineForwarder>    mpForwarder;
    VclPtr<OutputDevice>       mpWindow;
    bool                       mbIsCloned;
};

class ScAccessibleEditLineTextData : public ScAccessibleEditObjectTextData
{
public:
                        ScAccessibleEditLineTextData(EditView* pEditView,
                                                     OutputDevice* pWin,
                                                     ScTextWnd* pTextWnd);
    virtual             ~ScAccessibleEditLineTextData() override;

    virtual ScAccessibleTextData* Clone() const override;

    virtual SvxTextForwarder* GetTextForwarder() override;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) override;

    void Dispose();
    void TextChanged();
    void StartEdit();
    void EndEdit();
private:
    void ResetEditMode();

    ScTextWnd* mpTxtWnd;
    bool mbEditEngineCreated;
};

class ScAccessiblePreviewCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewCellTextData(ScPreviewShell* pViewShell,
                            const ScAddress& rP);
    virtual             ~ScAccessiblePreviewCellTextData() override;

    virtual ScAccessibleTextData* Clone() const override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual SvxTextForwarder* GetTextForwarder() override;
    virtual SvxViewForwarder* GetViewForwarder() override;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool /* bCreate */ ) override { return nullptr; }

private:
    std::unique_ptr<ScPreviewViewForwarder> mpViewForwarder;
    ScPreviewShell* mpViewShell;

    using ScAccessibleCellBaseTextData::GetDocShell;
    static ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};

class ScAccessiblePreviewHeaderCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
                            const OUString& rText, const ScAddress& rP, bool bColHeader, bool bRowHeader);
    virtual             ~ScAccessiblePreviewHeaderCellTextData() override;

    virtual ScAccessibleTextData* Clone() const override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual SvxTextForwarder* GetTextForwarder() override;
    virtual SvxViewForwarder* GetViewForwarder() override;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool /* bCreate */ ) override { return nullptr; }

private:
    std::unique_ptr<ScPreviewViewForwarder> mpViewForwarder;
    ScPreviewShell* mpViewShell;
    OUString        maText;
    bool            mbColHeader;
    bool            mbRowHeader;

    using ScAccessibleCellBaseTextData::GetDocShell;
    static ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};

class ScAccessibleHeaderTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleHeaderTextData(ScPreviewShell* pViewShell,
                            const EditTextObject* pEditObj, SvxAdjust eAdjust);
    virtual             ~ScAccessibleHeaderTextData() override;

    virtual ScAccessibleTextData* Clone() const override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual SvxTextForwarder* GetTextForwarder() override;
    virtual SvxViewForwarder* GetViewForwarder() override;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool /* bCreate */ ) override { return nullptr; }

    virtual void                UpdateData() override {  }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    std::unique_ptr<ScEditEngineDefaulter>  mpEditEngine;
    std::unique_ptr<SvxEditEngineForwarder> mpForwarder;
    ScDocShell*             mpDocSh;
    const EditTextObject*   mpEditObj;
    bool                    mbDataValid;
    SvxAdjust               meAdjust;
};

class ScAccessibleNoteTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleNoteTextData(ScPreviewShell* pViewShell,
                            const OUString& sText, const ScAddress& aCellPos, bool bMarkNote);
    virtual             ~ScAccessibleNoteTextData() override;

    virtual ScAccessibleTextData* Clone() const override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual SvxTextForwarder* GetTextForwarder() override;
    virtual SvxViewForwarder* GetViewForwarder() override;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool /* bCreate */ ) override { return nullptr; }

    virtual void                UpdateData() override {  }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    std::unique_ptr<ScEditEngineDefaulter> mpEditEngine;
    std::unique_ptr<SvxEditEngineForwarder> mpForwarder;
    ScDocShell*             mpDocSh;
    OUString                msText;
    ScAddress               maCellPos;
    bool                    mbMarkNote;
    bool                    mbDataValid;
};

class ScAccessibleCsvTextData : public ScAccessibleTextData
{
private:
    typedef ::std::unique_ptr< SvxTextForwarder > TextForwarderPtr;
    typedef ::std::unique_ptr< ScCsvViewForwarder > ViewForwarderPtr;

    VclPtr<OutputDevice>        mpWindow;
    EditEngine*                 mpEditEngine;
    TextForwarderPtr            mpTextForwarder;
    ViewForwarderPtr            mpViewForwarder;
    OUString                    maCellText;
    Size                        maCellSize;

public:
    explicit                    ScAccessibleCsvTextData(
                                    OutputDevice* pWindow,
                                    EditEngine* pEditEngine,
                                    const OUString& rCellText,
                                    const Size& rCellSize );
    virtual                     ~ScAccessibleCsvTextData() override;

    virtual ScAccessibleTextData* Clone() const override;

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual SvxViewForwarder*   GetViewForwarder() override;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) override;

    virtual void                UpdateData() override {}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
