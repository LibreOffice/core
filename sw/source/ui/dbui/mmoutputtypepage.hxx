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
#ifndef _MAILMERGEOUTPUTTYPEPAGE_HXX
#define _MAILMERGEOUTPUTTYPEPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <mailmergehelper.hxx>
class SwMailMergeWizard;

class SwMailMergeOutputTypePage : public svt::OWizardPage
{
    SwBoldFixedInfo m_aHeaderFI;
    FixedInfo       m_aTypeFT;
    RadioButton     m_aLetterRB;
    RadioButton     m_aMailRB;

    SwBoldFixedInfo m_aHintHeaderFI;
    FixedInfo       m_aHintFI;

    String          m_sLetterHintHeader;
    String          m_sMailHintHeader;
    String          m_sLetterHint;
    String          m_sMailHint;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(TypeHdl_Impl, void *);

public:
        SwMailMergeOutputTypePage( SwMailMergeWizard* _pParent);
        ~SwMailMergeOutputTypePage();

};
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
