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
#ifndef _MAILMERGEOUTPUTTYPEPAGE_HXX
#define _MAILMERGEOUTPUTTYPEPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <mailmergehelper.hxx>
class SwMailMergeWizard;
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeOutputTypePage : public svt::OWizardPage
{
    SwBoldFixedInfo m_aHeaderFI;
    FixedInfo       m_aTypeFT;
    RadioButton     m_aLetterRB;
    RadioButton     m_aMailRB;

    SwBoldFixedInfo m_aHintHeaderFI;
    FixedInfo       m_aHintFI;
    FixedInfo       m_aNoMailHintFI;

    String          m_sLetterHintHeader;
    String          m_sMailHintHeader;
    String          m_sLetterHint;
    String          m_sMailHint;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(TypeHdl_Impl, RadioButton*);

public:
        SwMailMergeOutputTypePage( SwMailMergeWizard* _pParent);
        ~SwMailMergeOutputTypePage();

};
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
