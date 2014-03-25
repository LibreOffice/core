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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_MMLAYOUTPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_MMLAYOUTPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <mailmergehelper.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Reference.h>

class SwMailMergeWizard;
class SwFrmFmt;
class SwOneExampleFrame;
class SwWrtShell;
class SwView;

namespace com{ namespace sun{ namespace star{ namespace beans{ class XPropertySet;}}}}

class SwMailMergeLayoutPage : public svt::OWizardPage
{
    VclContainer*       m_pPosition;

    CheckBox*           m_pAlignToBodyCB;
    FixedText*          m_pLeftFT;
    MetricField*        m_pLeftMF;
    MetricField*        m_pTopMF;

    VclContainer*       m_pGreetingLine;
    PushButton*         m_pUpPB;
    PushButton*         m_pDownPB;

    Window*             m_pExampleContainerWIN;

    ListBox*            m_pZoomLB;

    SwOneExampleFrame*  m_pExampleFrame;
    SwWrtShell*         m_pExampleWrtShell;

    OUString            m_sExampleURL;
    SwFrmFmt*           m_pAddressBlockFormat;

    bool                m_bIsGreetingInserted;

    SwMailMergeWizard*  m_pWizard;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  m_xViewProperties;

    DECL_LINK(PreviewLoadedHdl_Impl, void*);
    DECL_LINK(ZoomHdl_Impl, ListBox*);
    DECL_LINK(ChangeAddressHdl_Impl, void *);
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
    virtual bool        commitPage(::svt::WizardTypes::CommitPageReason _eReason);
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
