/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONDFORMATDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONDFORMATDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svx/fntctrl.hxx>
#include <svtools/ctrlbox.hxx>

#include "rangelst.hxx"
#include "condformathelper.hxx"
#include "viewdata.hxx"

#include "anyrefdg.hxx"

#define DLG_RET_ADD         8
#define DLG_RET_EDIT        16

class ScDocument;
class ScConditionalFormat;
class ScFormatEntry;
class ScConditionalFormat;
struct ScDataBarFormatData;
class ScCondFrmtEntry;

namespace condformat {

namespace dialog {

enum ScCondFormatDialogType
{
    NONE,
    CONDITION,
    COLORSCALE,
    DATABAR,
    ICONSET,
    DATE
};

}

}

class ScCondFormatDlg;

class ScCondFormatList : public Control
{
private:
    typedef std::vector<VclPtr<ScCondFrmtEntry>> EntryContainer;
    EntryContainer maEntries;

    bool mbHasScrollBar;
    VclPtr<ScrollBar> mpScrollBar;

    ScDocument* mpDoc;
    ScAddress maPos;
    ScRangeList maRanges;
    VclPtr<ScCondFormatDlg> mpDialogParent;

    void DoScroll(long nDiff);

public:
    ScCondFormatList(vcl::Window* pParent, WinBits nStyle);
    virtual ~ScCondFormatList();
    virtual void dispose() SAL_OVERRIDE;

    void init(ScDocument* pDoc, ScCondFormatDlg* pDialogParent, const ScConditionalFormat* pFormat,
        const ScRangeList& rRanges, const ScAddress& rPos,
        condformat::dialog::ScCondFormatDialogType eType);

    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;

    ScConditionalFormat* GetConditionalFormat() const;
    void RecalcAll();

    DECL_LINK_TYPED( AddBtnHdl, Button*, void );
    DECL_LINK_TYPED( RemoveBtnHdl, Button*, void );
    DECL_LINK_TYPED( ScrollHdl, ScrollBar*, void );
    DECL_LINK( EntrySelectHdl, ScCondFrmtEntry* );

    DECL_LINK( TypeListHdl, ListBox*);
    DECL_LINK_TYPED( AfterTypeListHdl, void*, void);
    DECL_LINK( ColFormatTypeHdl, ListBox*);
};

class ScCondFormatDlg : public ScAnyRefDlg
{
private:
    VclPtr<PushButton> mpBtnOk;
    VclPtr<PushButton> mpBtnAdd;
    VclPtr<PushButton> mpBtnRemove;
    VclPtr<PushButton> mpBtnCancel;
    VclPtr<FixedText> mpFtRange;
    VclPtr<formula::RefEdit> mpEdRange;
    VclPtr<formula::RefButton> mpRbRange;

    VclPtr<ScCondFormatList> mpCondFormList;
    sal_Int32 maKey;

    bool mbManaged;
    ScAddress maPos;
    ScViewData* mpViewData;

    VclPtr<formula::RefEdit> mpLastEdit;

    OUString msBaseTitle;
    void updateTitle();

    DECL_LINK( EdRangeModifyHdl, Edit* );
protected:

    virtual void RefInputDone( bool bForced = false ) SAL_OVERRIDE;
    void OkPressed();
    void CancelPressed();

public:
    SC_DLLPUBLIC ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pWindow,
                                 ScViewData* pViewData, const ScConditionalFormat* pFormat,
                                 const ScRangeList& rRange, const ScAddress& rPos,
                                 condformat::dialog::ScCondFormatDialogType eType, bool bManaged);
    virtual ~ScCondFormatDlg();
    virtual void dispose() SAL_OVERRIDE;

    SC_DLLPUBLIC ScConditionalFormat* GetConditionalFormat() const;

    static OUString GenerateXmlString(sal_uInt32 nIndex, sal_uInt8 nType, bool bManaged);
    static bool ParseXmlString(const OUString& sXMLString, sal_uInt32& nIndex,
                               sal_uInt8& nType, bool& bManaged);
    virtual void SetReference(const ScRange&, ScDocument*) SAL_OVERRIDE;
    virtual bool IsRefInputMode() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual bool IsTableLocked() const SAL_OVERRIDE;
    virtual bool Close() SAL_OVERRIDE;

    void InvalidateRefData();

    DECL_LINK_TYPED( BtnPressedHdl, Button*, void );
    DECL_LINK_TYPED( RangeGetFocusHdl, Control&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
