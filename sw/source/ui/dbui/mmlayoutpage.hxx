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
#ifndef _MAILMERGELAYOUTPAGE_HXX
#define _MAILMERGELAYOUTPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <mailmergehelper.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Reference.h>

class SwMailMergeWizard;
class SwFrmFmt;
class SwOneExampleFrame;
class SwWrtShell;
class SwView;

namespace com{ namespace sun{ namespace star{ namespace beans{ class XPropertySet;}}}}
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeLayoutPage : public svt::OWizardPage
{
    SwBoldFixedInfo     m_aHeaderFI;

    FixedLine           m_aPositionFL;

    CheckBox            m_aAlignToBodyCB;
    FixedText           m_aLeftFT;
    MetricField         m_aLeftMF;
    FixedText           m_aTopFT;
    MetricField         m_aTopMF;

    FixedLine           m_aGreetingLineFL;
    FixedText           m_aUpFT;
    PushButton          m_aUpPB;
    FixedText           m_aDownFT;
    PushButton          m_aDownPB;

    Window              m_aExampleContainerWIN;
    Window              m_aExampleWIN;

    FixedText           m_aZoomFT;
    ListBox             m_aZoomLB;

    SwOneExampleFrame*  m_pExampleFrame;
    SwWrtShell*         m_pExampleWrtShell;

    String              m_sExampleURL;
    SwFrmFmt*           m_pAddressBlockFormat;

    bool                m_bIsGreetingInserted;

    SwMailMergeWizard*  m_pWizard;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  m_xViewProperties;

    DECL_LINK(PreviewLoadedHdl_Impl, void*);
    DECL_LINK(ZoomHdl_Impl, ListBox*);
    DECL_LINK(ChangeAddressHdl_Impl, MetricField*);
    DECL_LINK(GreetingsHdl_Impl, PushButton*);
    DECL_LINK(AlignToTextHdl_Impl, CheckBox*);

    static SwFrmFmt*        InsertAddressFrame(
                            SwWrtShell& rShell,
                            SwMailMergeConfigItem& rConfigItem,
                            const Point& rDestination,
                            bool bAlignToBody,
                            bool bExample);
    static void             InsertGreeting(SwWrtShell& rShell, SwMailMergeConfigItem& rConfigItem, bool bExample);

    virtual void        ActivatePage();
    virtual sal_Bool    commitPage(::svt::WizardTypes::CommitPageReason _eReason);
public:
        SwMailMergeLayoutPage( SwMailMergeWizard* _pParent);
        ~SwMailMergeLayoutPage();

    static SwFrmFmt*        InsertAddressAndGreeting(SwView* pView,
                                            SwMailMergeConfigItem& rConfigItem,
                                            const Point& rAddressPos,
                                            bool bAlignToBody);
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
