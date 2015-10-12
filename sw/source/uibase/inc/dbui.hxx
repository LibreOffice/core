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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DBUI_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DBUI_HXX

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

class SW_DLLPUBLIC CancelableDialog : public Dialog
{
    bool mbModal;

protected:
    VclPtr<CancelButton> m_pCancelButton;
    CancelableDialog( vcl::Window *pParent, bool modal, const OUString& rID,
                      const OUString& rUIXMLDescription );

    using Dialog::Execute;
    using Dialog::StartExecuteModal;

public:
    virtual ~CancelableDialog();
    virtual void dispose() override;

    void SetCancelHdl( const Link<Button*,void>& rLink );
    void Show();
};

class SW_DLLPUBLIC PrintMonitor: public CancelableDialog
{
public:
    enum PrintMonitorType
    {
        MONITOR_TYPE_PRINT,
        MONITOR_TYPE_SAVE
    };

    VclPtr<FixedText> m_pDocName;
    VclPtr<FixedText> m_pPrinting;
    VclPtr<FixedText> m_pPrinter;
    VclPtr<FixedText> m_pPrintInfo;

    PrintMonitor( vcl::Window *pParent, bool modal, PrintMonitorType eType );
    virtual ~PrintMonitor();
    virtual void dispose() override;
};

class CreateMonitor : public CancelableDialog
{
public:
    CreateMonitor( vcl::Window *pParent, bool modal );
    virtual ~CreateMonitor();
    virtual void dispose() override;

    void SetTotalCount( sal_Int32 nTotal );
    void SetCurrentPosition( sal_Int32 nCurrent );

private:
    void UpdateCountingText();

private:
    VclPtr<FixedText>      m_pCounting;

    OUString        m_sCountingPattern;
    OUString        m_sVariable_Total;
    OUString        m_sVariable_Position;
    sal_Int32       m_nTotalCount;
    sal_Int32       m_nCurrentPosition;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
