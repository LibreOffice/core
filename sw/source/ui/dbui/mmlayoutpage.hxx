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

#include <vcl/wizardmachine.hxx>
#include <mailmergehelper.hxx>
#include <com/sun/star/uno/Reference.h>

class SwMailMergeWizard;
class SwFrameFormat;
class SwOneExampleFrame;
class SwWrtShell;
class SwView;

namespace com{ namespace sun{ namespace star{ namespace beans{ class XPropertySet;}}}}

class SwMailMergeLayoutPage : public vcl::OWizardPage
{
    SwWrtShell*         m_pExampleWrtShell;

    OUString            m_sExampleURL;
    SwFrameFormat*      m_pAddressBlockFormat;

    bool                m_bIsGreetingInserted;

    VclPtr<SwMailMergeWizard>  m_pWizard;

    css::uno::Reference< css::beans::XPropertySet >  m_xViewProperties;

    std::unique_ptr<weld::Container> m_xPosition;
    std::unique_ptr<weld::CheckButton> m_xAlignToBodyCB;
    std::unique_ptr<weld::Label> m_xLeftFT;
    std::unique_ptr<weld::MetricSpinButton> m_xLeftMF;
    std::unique_ptr<weld::MetricSpinButton> m_xTopMF;
    std::unique_ptr<weld::Container> m_xGreetingLine;
    std::unique_ptr<weld::Button> m_xUpPB;
    std::unique_ptr<weld::Button> m_xDownPB;
    std::unique_ptr<weld::ComboBox> m_xZoomLB;
    std::unique_ptr<SwOneExampleFrame> m_xExampleFrame;
    std::unique_ptr<weld::CustomWeld> m_xExampleContainerWIN;

    DECL_LINK(PreviewLoadedHdl_Impl, SwOneExampleFrame&, void);
    DECL_LINK(ZoomHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ChangeAddressHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(GreetingsHdl_Impl, weld::Button&, void);
    DECL_LINK(AlignToTextHdl_Impl, weld::ToggleButton&, void);

    static SwFrameFormat* InsertAddressFrame(
                            SwWrtShell& rShell,
                            SwMailMergeConfigItem const & rConfigItem,
                            const Point& rDestination,
                            bool bAlignToBody,
                            bool bExample);
    static void             InsertGreeting(SwWrtShell& rShell, SwMailMergeConfigItem const & rConfigItem, bool bExample);

    virtual void        ActivatePage() override;
    virtual bool        commitPage(::vcl::WizardTypes::CommitPageReason _eReason) override;
public:
    SwMailMergeLayoutPage(SwMailMergeWizard* pWizard, TabPageParent pParent);
    virtual ~SwMailMergeLayoutPage() override;
    virtual void            dispose() override;

    static SwFrameFormat*        InsertAddressAndGreeting(SwView const * pView,
                                            SwMailMergeConfigItem& rConfigItem,
                                            const Point& rAddressPos,
                                            bool bAlignToBody);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
