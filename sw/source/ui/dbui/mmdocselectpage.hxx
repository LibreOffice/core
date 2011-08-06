/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _MAILMERGEDOCSELECTPAGE_HXX
#define _MAILMERGEDOCSELECTPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <mailmergehelper.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
class SwMailMergeWizard;

class SwMailMergeDocSelectPage : public svt::OWizardPage
{
    SwBoldFixedInfo     m_aHeaderFI;
    FixedInfo           m_aHowToFT;
    RadioButton         m_aCurrentDocRB;
    RadioButton         m_aNewDocRB;
    RadioButton         m_aLoadDocRB;
    RadioButton         m_aLoadTemplateRB;
    RadioButton         m_aRecentDocRB;

    PushButton          m_aBrowseDocPB;
    PushButton          m_aBrowseTemplatePB;

    ListBox             m_aRecentDocLB;

    String              m_sLoadFileName;
    String              m_sLoadTemplateName;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(DocSelectHdl, RadioButton*);
    DECL_LINK(FileSelectHdl, PushButton*);

    virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

public:
        SwMailMergeDocSelectPage( SwMailMergeWizard* _pParent);
        ~SwMailMergeDocSelectPage();

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
