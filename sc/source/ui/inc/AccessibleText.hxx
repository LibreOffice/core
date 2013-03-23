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
#if 1

#include "textuno.hxx"
#include "global.hxx"
#include "viewdata.hxx"
#include <editeng/svxenum.hxx>

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


// ============================================================================

class ScAccessibleTextData : public SfxListener
{
public:
                        ScAccessibleTextData() {}
    virtual             ~ScAccessibleTextData() {}

    virtual ScAccessibleTextData* Clone() const = 0;

    virtual void        Notify( SfxBroadcaster& /* rBC */, const SfxHint& /* rHint */ ) {}

    virtual SvxTextForwarder* GetTextForwarder() = 0;
    virtual SvxViewForwarder* GetViewForwarder() = 0;
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) = 0;
    virtual SfxBroadcaster& GetBroadcaster() const { return maBroadcaster; }

    virtual void                UpdateData() = 0;
    virtual void                SetDoUpdate(sal_Bool bValue) = 0;
    virtual sal_Bool            IsDirty() const = 0;

private:
    mutable SfxBroadcaster maBroadcaster;

    // prevent the using of this method of the base class
    ScCellEditSource* GetOriginalSource() { return NULL; }
};


// ============================================================================

class ScAccessibleCellBaseTextData : public ScAccessibleTextData,
                                     public ScCellTextData
{
public:
                        ScAccessibleCellBaseTextData(ScDocShell* pDocShellP,
                            const ScAddress& rP)
                            : ScCellTextData(pDocShellP, rP) {}
    virtual             ~ScAccessibleCellBaseTextData() {}
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) { ScCellTextData::Notify(rBC, rHint); }

    virtual void                UpdateData() { ScCellTextData::UpdateData(); }
    virtual void                SetDoUpdate(sal_Bool bValue) { ScCellTextData::SetDoUpdate(bValue); }
    virtual sal_Bool            IsDirty() const { return ScCellTextData::IsDirty(); }
};


// ============================================================================

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

class ScAccessibleCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessibleCellTextData(ScTabViewShell* pViewShell,
                            const ScAddress& rP, ScSplitPos eSplitPos, ScAccessibleCell* pAccCell);
    virtual             ~ScAccessibleCellTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    DECL_LINK( NotifyHdl, EENotify* );
protected:
    virtual void            GetCellText(const ScAddress& rCellPos, String& rText);
private:
    ScViewForwarder* mpViewForwarder;
    ScEditViewForwarder* mpEditViewForwarder;
    ScTabViewShell* mpViewShell;
    ScSplitPos meSplitPos;
    ScAccessibleCell* mpAccessibleCell;

    // prevent the using of this method of the base class
    ScCellEditSource* GetOriginalSource() { return NULL; }

    using ScAccessibleCellBaseTextData::GetDocShell;
    ScDocShell* GetDocShell(ScTabViewShell* pViewShell);
};


// ============================================================================

class ScAccessibleEditObjectTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleEditObjectTextData(EditView* pEditView, Window* pWin);
    virtual             ~ScAccessibleEditObjectTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    virtual void                UpdateData() {  }
    virtual void                SetDoUpdate(sal_Bool /* bValue */) {  }
    virtual sal_Bool            IsDirty() const { return false; }

    DECL_LINK( NotifyHdl, EENotify* );
protected:
    ScEditObjectViewForwarder* mpViewForwarder;
    ScEditViewForwarder* mpEditViewForwarder;
    EditView* mpEditView;
    EditEngine* mpEditEngine;
    SvxEditEngineForwarder* mpForwarder;
    Window* mpWindow;
};


// ============================================================================

class ScAccessibleEditLineTextData : public ScAccessibleEditObjectTextData
{
public:
                        ScAccessibleEditLineTextData(EditView* pEditView, Window* pWin);
    virtual             ~ScAccessibleEditLineTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    void Dispose();
    void TextChanged();
    void StartEdit();
    void EndEdit();
private:
    void ResetEditMode();

    sal_Bool mbEditEngineCreated;
};


// ============================================================================

class ScAccessiblePreviewCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewCellTextData(ScPreviewShell* pViewShell,
                            const ScAddress& rP);
    virtual             ~ScAccessiblePreviewCellTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool /* bCreate */ ) { return NULL; }

private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell* mpViewShell;

    // prevent the using of this method of the base class
    ScCellEditSource* GetOriginalSource() { return NULL; }

    using ScAccessibleCellBaseTextData::GetDocShell;
    ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};


// ============================================================================

class ScAccessiblePreviewHeaderCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
                            const String& rText, const ScAddress& rP, sal_Bool bColHeader, sal_Bool bRowHeader);
    virtual             ~ScAccessiblePreviewHeaderCellTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool /* bCreate */ ) { return NULL; }

private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell* mpViewShell;
    String          maText;
    sal_Bool        mbColHeader;
    sal_Bool        mbRowHeader;

    // prevent the using of this method of the base class
    ScCellEditSource* GetOriginalSource() { return NULL; }

    using ScAccessibleCellBaseTextData::GetDocShell;
    ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};


// ============================================================================

class ScAccessibleHeaderTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleHeaderTextData(ScPreviewShell* pViewShell,
                            const EditTextObject* pEditObj, sal_Bool bHeader, SvxAdjust eAdjust);
    virtual             ~ScAccessibleHeaderTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool /* bCreate */ ) { return NULL; }

    virtual void                UpdateData() {  }
    virtual void                SetDoUpdate(sal_Bool /* bValue */) {  }
    virtual sal_Bool            IsDirty() const { return false; }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    ScEditEngineDefaulter*  mpEditEngine;
    SvxEditEngineForwarder* mpForwarder;
    ScDocShell*             mpDocSh;
    const EditTextObject*   mpEditObj;
    sal_Bool                mbHeader;
    sal_Bool                mbDataValid;
    SvxAdjust               meAdjust;
};


// ============================================================================

class ScAccessibleNoteTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleNoteTextData(ScPreviewShell* pViewShell,
                            const String& sText, const ScAddress& aCellPos, sal_Bool bMarkNote);
    virtual             ~ScAccessibleNoteTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool /* bCreate */ ) { return NULL; }

    virtual void                UpdateData() {  }
    virtual void                SetDoUpdate(sal_Bool /* bValue */) {  }
    virtual sal_Bool            IsDirty() const { return false; }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    ScEditEngineDefaulter*  mpEditEngine;
    SvxEditEngineForwarder* mpForwarder;
    ScDocShell*             mpDocSh;
    String                  msText;
    ScAddress               maCellPos;
    sal_Bool                mbMarkNote;
    sal_Bool                mbDataValid;
};


// ============================================================================

class ScAccessibleCsvTextData : public ScAccessibleTextData
{
private:
    typedef ::std::auto_ptr< SvxTextForwarder > TextForwarderPtr;
    typedef ::std::auto_ptr< ScCsvViewForwarder > ViewForwarderPtr;

    Window*                     mpWindow;
    EditEngine*                 mpEditEngine;
    TextForwarderPtr            mpTextForwarder;
    ViewForwarderPtr            mpViewForwarder;
    String                      maCellText;
    Rectangle                   maBoundBox;
    Size                        maCellSize;

public:
    explicit                    ScAccessibleCsvTextData(
                                    Window* pWindow,
                                    EditEngine* pEditEngine,
                                    const String& rCellText,
                                    const Rectangle& rBoundBox,
                                    const Size& rCellSize );
    virtual                     ~ScAccessibleCsvTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder*   GetTextForwarder();
    virtual SvxViewForwarder*   GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    virtual void                UpdateData() {}
    virtual void                SetDoUpdate( sal_Bool /* bValue */ ) {}
    virtual sal_Bool            IsDirty() const { return false; }
};


// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
