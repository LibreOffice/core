/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_DATASTREAMDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DATASTREAMDLG_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>

#include "datastream.hxx"

class ScDocShell;
class SvtURLBox;
class ScRange;

namespace sc {

class DataStreamDlg : public ModalDialog
{
    ScDocShell *mpDocShell;

    VclPtr<SvtURLBox>      m_pCbUrl;
    VclPtr<PushButton>     m_pBtnBrowse;
    VclPtr<RadioButton>    m_pRBDirectData;
    VclPtr<RadioButton>    m_pRBScriptData;
    VclPtr<RadioButton>    m_pRBValuesInLine;
    VclPtr<RadioButton>    m_pRBAddressValue;
    VclPtr<CheckBox>       m_pCBRefreshOnEmpty;
    VclPtr<RadioButton>    m_pRBDataDown;
    VclPtr<RadioButton>    m_pRBRangeDown;
    VclPtr<RadioButton>    m_pRBNoMove;
    VclPtr<RadioButton>    m_pRBMaxLimit;
    VclPtr<RadioButton>    m_pRBUnlimited;
    VclPtr<Edit>           m_pEdRange;
    VclPtr<Edit>           m_pEdLimit;
    VclPtr<OKButton>       m_pBtnOk;
    VclPtr<VclFrame>       m_pVclFrameLimit;
    VclPtr<VclFrame>       m_pVclFrameMove;

    DECL_LINK_TYPED(UpdateClickHdl, Button*, void);
    DECL_LINK(UpdateHdl, void *);
    DECL_LINK_TYPED(BrowseHdl, Button*, void);

    void UpdateEnable();
    ScRange GetStartRange();

public:
    DataStreamDlg(ScDocShell *pDocShell, vcl::Window* pParent);
    virtual ~DataStreamDlg();
    virtual void dispose() override;

    void Init( const DataStream& rStrm );

    void StartStream();
};

}

#endif // INCLUDED_SC_SOURCE_UI_INC_DATASTREAMDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
