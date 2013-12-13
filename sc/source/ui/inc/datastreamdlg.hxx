/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>

class DataStream;
class ScDocShell;
class SvtURLBox;

class DataStreamDlg : public ModalDialog
{
    ScDocShell *mpDocShell;

    SvtURLBox*      m_pCbUrl;
    PushButton*     m_pBtnBrowse;
    RadioButton*    m_pRBScriptData;
    RadioButton*    m_pRBValuesInLine;
    RadioButton*    m_pRBAddressValue;
    RadioButton*    m_pRBDataDown;
    RadioButton*    m_pRBRangeDown;
    RadioButton*    m_pRBNoMove;
    RadioButton*    m_pRBMaxLimit;
    Edit*           m_pEdRange;
    Edit*           m_pEdLimit;
    OKButton*       m_pBtnOk;
    VclFrame*       m_pVclFrameLimit;
    VclFrame*       m_pVclFrameMove;

    DECL_LINK(UpdateHdl, void *);
    DECL_LINK(BrowseHdl, void *);

    void UpdateEnable();

public:
    DataStreamDlg(ScDocShell *pDocShell, Window* pParent);
    ~DataStreamDlg() {}
    void Init(const OUString& rURL, const OUString& rRange, const sal_Int32 nLimit,
            const OUString& rMove, const sal_uInt32 nSettings);
    void StartStream(DataStream *pStream = 0);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
