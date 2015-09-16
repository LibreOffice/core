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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLETEXT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLETEXT_HXX

#include "textuno.hxx"
#include "global.hxx"
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

class ScAccessibleTextData : public SfxListener
{
public:
                        ScAccessibleTextData() {}
    virtual             ~ScAccessibleTextData() {}

    virtual ScAccessibleTextData* Clone() const = 0;

    virtual void        Notify( SfxBroadcaster& /* rBC */, const SfxHint& /* rHint */ ) SAL_OVERRIDE {}

    virtual SvxTextForwarder* GetTextForwarder() = 0;
    virtual SvxViewForwarder* GetViewForwarder() = 0;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) = 0;
    SfxBroadcaster& GetBroadcaster() const { return maBroadcaster; }

    virtual void                UpdateData() = 0;
    DECL_LINK_TYPED( NotifyHdl, EENotify&, void );

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
    virtual             ~ScAccessibleCellBaseTextData() {}
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE { ScCellTextData::Notify(rBC, rHint); }

    virtual void                UpdateData() SAL_OVERRIDE { ScCellTextData::UpdateData(); }
};

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

class ScAccessibleCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessibleCellTextData(ScTabViewShell* pViewShell,
                            const ScAddress& rP, ScSplitPos eSplitPos, ScAccessibleCell* pAccCell);
    virtual             ~ScAccessibleCellTextData();

    virtual ScAccessibleTextData* Clone() const SAL_OVERRIDE;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    virtual SvxTextForwarder* GetTextForwarder() SAL_OVERRIDE;
    virtual SvxViewForwarder* GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) SAL_OVERRIDE;

protected:
    virtual void            GetCellText(const ScAddress& rCellPos, OUString& rText) SAL_OVERRIDE;
private:
    ScViewForwarder* mpViewForwarder;
    ScEditViewForwarder* mpEditViewForwarder;
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
    ScAccessibleEditObjectTextData(EditView* pEditView, vcl::Window* pWin, bool isClone = false);
    virtual             ~ScAccessibleEditObjectTextData();

    virtual ScAccessibleTextData* Clone() const SAL_OVERRIDE;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    virtual SvxTextForwarder* GetTextForwarder() SAL_OVERRIDE;
    virtual SvxViewForwarder* GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) SAL_OVERRIDE;

    virtual void                UpdateData() SAL_OVERRIDE {  }

    DECL_LINK_TYPED( NotifyHdl, EENotify&, void );
protected:
    ScEditObjectViewForwarder* mpViewForwarder;
    ScEditViewForwarder*       mpEditViewForwarder;
    EditView*                  mpEditView;
    EditEngine*                mpEditEngine;
    SvxEditEngineForwarder*    mpForwarder;
    VclPtr<vcl::Window>        mpWindow;
    bool                       mbIsCloned;
};

class ScAccessibleEditLineTextData : public ScAccessibleEditObjectTextData
{
public:
                        ScAccessibleEditLineTextData(EditView* pEditView, vcl::Window* pWin);
    virtual             ~ScAccessibleEditLineTextData();

    virtual ScAccessibleTextData* Clone() const SAL_OVERRIDE;

    virtual SvxTextForwarder* GetTextForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) SAL_OVERRIDE;

    void Dispose();
    void TextChanged();
    void StartEdit();
    void EndEdit();
private:
    void ResetEditMode();

    bool mbEditEngineCreated;
};

class ScAccessiblePreviewCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewCellTextData(ScPreviewShell* pViewShell,
                            const ScAddress& rP);
    virtual             ~ScAccessiblePreviewCellTextData();

    virtual ScAccessibleTextData* Clone() const SAL_OVERRIDE;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    virtual SvxTextForwarder* GetTextForwarder() SAL_OVERRIDE;
    virtual SvxViewForwarder* GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool /* bCreate */ ) SAL_OVERRIDE { return NULL; }

private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell* mpViewShell;

    using ScAccessibleCellBaseTextData::GetDocShell;
    static ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};

class ScAccessiblePreviewHeaderCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
                            const OUString& rText, const ScAddress& rP, bool bColHeader, bool bRowHeader);
    virtual             ~ScAccessiblePreviewHeaderCellTextData();

    virtual ScAccessibleTextData* Clone() const SAL_OVERRIDE;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    virtual SvxTextForwarder* GetTextForwarder() SAL_OVERRIDE;
    virtual SvxViewForwarder* GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool /* bCreate */ ) SAL_OVERRIDE { return NULL; }

private:
    ScPreviewViewForwarder* mpViewForwarder;
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
                            const EditTextObject* pEditObj, bool bHeader, SvxAdjust eAdjust);
    virtual             ~ScAccessibleHeaderTextData();

    virtual ScAccessibleTextData* Clone() const SAL_OVERRIDE;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    virtual SvxTextForwarder* GetTextForwarder() SAL_OVERRIDE;
    virtual SvxViewForwarder* GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool /* bCreate */ ) SAL_OVERRIDE { return NULL; }

    virtual void                UpdateData() SAL_OVERRIDE {  }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    ScEditEngineDefaulter*  mpEditEngine;
    SvxEditEngineForwarder* mpForwarder;
    ScDocShell*             mpDocSh;
    const EditTextObject*   mpEditObj;
    bool                    mbHeader;
    bool                    mbDataValid;
    SvxAdjust               meAdjust;
};

class ScAccessibleNoteTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleNoteTextData(ScPreviewShell* pViewShell,
                            const OUString& sText, const ScAddress& aCellPos, bool bMarkNote);
    virtual             ~ScAccessibleNoteTextData();

    virtual ScAccessibleTextData* Clone() const SAL_OVERRIDE;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    virtual SvxTextForwarder* GetTextForwarder() SAL_OVERRIDE;
    virtual SvxViewForwarder* GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool /* bCreate */ ) SAL_OVERRIDE { return NULL; }

    virtual void                UpdateData() SAL_OVERRIDE {  }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    ScEditEngineDefaulter*  mpEditEngine;
    SvxEditEngineForwarder* mpForwarder;
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

    VclPtr<vcl::Window>         mpWindow;
    EditEngine*                 mpEditEngine;
    TextForwarderPtr            mpTextForwarder;
    ViewForwarderPtr            mpViewForwarder;
    OUString                    maCellText;
    Rectangle                   maBoundBox;
    Size                        maCellSize;

public:
    explicit                    ScAccessibleCsvTextData(
                                    vcl::Window* pWindow,
                                    EditEngine* pEditEngine,
                                    const OUString& rCellText,
                                    const Rectangle& rBoundBox,
                                    const Size& rCellSize );
    virtual                     ~ScAccessibleCsvTextData();

    virtual ScAccessibleTextData* Clone() const SAL_OVERRIDE;

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    virtual SvxTextForwarder*   GetTextForwarder() SAL_OVERRIDE;
    virtual SvxViewForwarder*   GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate ) SAL_OVERRIDE;

    virtual void                UpdateData() SAL_OVERRIDE {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
