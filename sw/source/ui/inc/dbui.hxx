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
#ifndef _DBUI_HXX
#define _DBUI_HXX

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>


class SW_DLLPUBLIC PrintMonitor: public ModelessDialog
{
public:
    enum PrintMonitorType
    {
        MONITOR_TYPE_PRINT,
        MONITOR_TYPE_SAVE
    };
    FixedText* m_pDocName;
    FixedText* m_pPrinting;
    FixedText* m_pPrinter;
    FixedText* m_pPrintInfo;
    CancelButton* m_pCancel;

    PrintMonitor( Window *pParent, PrintMonitorType eType );
};

class CreateMonitor : public ModelessDialog
{
public:
    CreateMonitor( Window *pParent );

    void SetTotalCount( sal_Int32 nTotal );
    void SetCurrentPosition( sal_Int32 nCurrent );

    void SetCancelHdl( const Link& rLink );

private: //methods
    void UpdateCountingText();

private: //member
    FixedText       m_aStatus;
    FixedText       m_aProgress;
    FixedText       m_aCreateDocuments;
    FixedText       m_aCounting;
    CancelButton    m_aCancelButton;

    String          m_sCountingPattern;
    String          m_sVariable_Total;
    String          m_sVariable_Position;
    sal_Int32       m_nTotalCount;
    sal_Int32       m_nCurrentPosition;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
