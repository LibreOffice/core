/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _MAILMERGELAYOUTPAGE_HXX
#define _MAILMERGELAYOUTPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <mailmergehelper.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
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


