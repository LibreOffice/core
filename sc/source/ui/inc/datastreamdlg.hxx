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

    SvtURLBox*      m_pCbUrl;
    PushButton*     m_pBtnBrowse;
    RadioButton*    m_pRBDirectData;
    RadioButton*    m_pRBScriptData;
    RadioButton*    m_pRBValuesInLine;
    RadioButton*    m_pRBAddressValue;
    CheckBox*       m_pCBRefreshOnEmpty;
    RadioButton*    m_pRBDataDown;
    RadioButton*    m_pRBRangeDown;
    RadioButton*    m_pRBNoMove;
    RadioButton*    m_pRBMaxLimit;
    RadioButton*    m_pRBUnlimited;
    Edit*           m_pEdRange;
    Edit*           m_pEdLimit;
    OKButton*       m_pBtnOk;
    VclFrame*       m_pVclFrameLimit;
    VclFrame*       m_pVclFrameMove;

    DECL_LINK(UpdateHdl, void *);
    DECL_LINK(BrowseHdl, void *);

    void UpdateEnable();
    ScRange GetStartRange();

public:
    DataStreamDlg(ScDocShell *pDocShell, vcl::Window* pParent);

    void Init( const DataStream& rStrm );

    void StartStream();
};

}

#endif // INCLUDED_SC_SOURCE_UI_INC_DATASTREAMDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
