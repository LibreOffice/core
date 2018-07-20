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
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Reference.h>

class SwMailMergeWizard;
class SwFrameFormat;
class SwOneExampleFrame;
class SwWrtShell;
class SwView;

namespace com{ namespace sun{ namespace star{ namespace beans{ class XPropertySet;}}}}

class SwMailMergeLayoutPage : public svt::OWizardPage
{
    VclPtr<VclContainer>       m_pPosition;

    VclPtr<CheckBox>           m_pAlignToBodyCB;
    VclPtr<FixedText>          m_pLeftFT;
    VclPtr<MetricField>        m_pLeftMF;
    VclPtr<MetricField>        m_pTopMF;

    VclPtr<VclContainer>       m_pGreetingLine;
    VclPtr<PushButton>         m_pUpPB;
    VclPtr<PushButton>         m_pDownPB;

    VclPtr<vcl::Window>             m_pExampleContainerWIN;

    VclPtr<ListBox>            m_pZoomLB;

    SwOneExampleFrame*  m_pExampleFrame;
    SwWrtShell*         m_pExampleWrtShell;

    OUString            m_sExampleURL;
    SwFrameFormat*           m_pAddressBlockFormat;

    bool                m_bIsGreetingInserted;

    VclPtr<SwMailMergeWizard>  m_pWizard;

    css::uno::Reference< css::beans::XPropertySet >  m_xViewProperties;

    DECL_LINK(PreviewLoadedHdl_Impl, SwOneExampleFrame&, void);
    DECL_LINK(ZoomHdl_Impl, ListBox&, void);
    DECL_LINK(ChangeAddressHdl_Impl, SpinField&, void);
    DECL_LINK(ChangeAddressLoseFocusHdl_Impl, Control&, void);
    DECL_LINK(GreetingsHdl_Impl, Button*, void);
    DECL_LINK(AlignToTextHdl_Impl, Button*, void);

    static SwFrameFormat*        InsertAddressFrame(
                            SwWrtShell& rShell,
                            SwMailMergeConfigItem const & rConfigItem,
                            const Point& rDestination,
                            bool bAlignToBody,
                            bool bExample);
    static void                 CheckForFieldsAndInsert(
                            SwWrtShell& rShell,
                            SwMailMergeConfigItem const & rConfigItem,
                            css::uno::Sequence< OUString> rEntries,
                            sal_uInt32 rCurrent);
    static void             InsertGreeting(SwWrtShell& rShell, SwMailMergeConfigItem const & rConfigItem, bool bExample);

    virtual void        ActivatePage() override;
    virtual bool        commitPage(::svt::WizardTypes::CommitPageReason _eReason) override;
public:
        SwMailMergeLayoutPage( SwMailMergeWizard* _pParent);
        virtual ~SwMailMergeLayoutPage() override;
    virtual void            dispose() override;

    static SwFrameFormat*        InsertAddressAndGreeting(SwView const * pView,
                                            SwMailMergeConfigItem& rConfigItem,
                                            const Point& rAddressPos,
                                            bool bAlignToBody);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
