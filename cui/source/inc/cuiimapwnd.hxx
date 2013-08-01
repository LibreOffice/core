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
#ifndef _CUI_IMAPWND_HXX
#define _CUI_IMAPWND_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <svl/itempool.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/transfer.hxx>
#include <svtools/imap.hxx>
#include <sfx2/frame.hxx>
#include <svtools/svmedit.hxx>

class URLDlg : public ModalDialog
{
    Edit*                m_pEdtURL;
    ComboBox*            m_pCbbTargets;
    Edit*                m_pEdtName;
    Edit*                m_pEdtAlternativeText;
    VclMultiLineEdit*       m_pEdtDescription;

public:

                        URLDlg( Window* pWindow,
                                const String& rURL, const String& rAlternativeText, const String& rDescription,
                                const String& rTarget, const String& rName,
                                TargetList& rTargetList );

    String              GetURL() const { return m_pEdtURL->GetText(); }
    String              GetAltText() const { return m_pEdtAlternativeText->GetText(); }
    String              GetDesc() const { return m_pEdtDescription->GetText(); }
    String              GetTarget() const { return m_pCbbTargets->GetText(); }
    String              GetName() const { return m_pEdtName->GetText(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
