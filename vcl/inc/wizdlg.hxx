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

#ifndef INCLUDED_VCL_WIZDLG_HXX
#define INCLUDED_VCL_WIZDLG_HXX

#include <memory>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/roadmapwizard.hxx>
#include <vcl/tabpage.hxx>

struct ImplWizPageData
{
    ImplWizPageData*    mpNext;
    VclPtr<TabPage>     mpPage;
};

namespace vcl
{
    struct RoadmapWizardImpl;
    class RoadmapWizard;

    namespace RoadmapWizardTypes
    {
        typedef VclPtr<TabPage> (* RoadmapPageFactory)( RoadmapWizard& );
    };

    //= RoadmapWizard

    /** wizard for a roadmap

        The basic new concept introduced is a <em>path</em>:<br/>
        A <em>path</em> is a sequence of states, which are to be executed in a linear order.
        Elements in the path can be skipped, depending on choices the user makes.

        In the most simple wizards, you will have only one path consisting of <code>n</code> elements,
        which are to be visited successively.

        In a slightly more complex wizard, you will have one linear path, were certain
        steps might be skipped due to user input. For instance, the user may decide to not specify
        certain aspects of the to-be-created object (e.g. by unchecking a check box),
        and the wizard then will simply disable the step which corresponds to this step.

        In a yet more advanced wizards, you will have several paths of length <code>n1</code> and
        <code>n2</code>, which share at least the first <code>k</code> states (where <code>k</code>
        is at least 1), and an arbitrary number of other states.
    */
    class RoadmapWizard : public Dialog
    {
    private:
        Idle                    maWizardLayoutIdle;
        Size                    maPageSize;
        ImplWizPageData*        mpFirstPage;
        ImplWizButtonData*      mpFirstBtn;
        VclPtr<TabPage>         mpCurTabPage;
        VclPtr<PushButton>      mpPrevBtn;
        VclPtr<PushButton>      mpNextBtn;
        VclPtr<vcl::Window>     mpViewWindow;
        sal_uInt16              mnCurLevel;
        sal_Int16               mnLeftAlignCount;
        bool                    mbEmptyViewMargin;

        DECL_LINK( ImplHandleWizardLayoutTimerHdl, Timer*, void );

        // IMPORTANT:
        // traveling pages should not be done by calling these base class member, some mechanisms of this class
        // here (e.g. committing page data) depend on having full control over page traveling.
        // So use the travelXXX methods if you need to travel

    protected:
        tools::Long                LogicalCoordinateToPixel(int iCoordinate);
        /**sets the number of buttons which should be left-aligned. Normally, buttons are right-aligned.

            only to be used during construction, before any layouting happened
        */
        void                SetLeftAlignedButtonCount( sal_Int16 _nCount );

        void                CalcAndSetSize();

    public:
        VclPtr<OKButton>       m_pFinish;
        VclPtr<CancelButton>   m_pCancel;
        VclPtr<PushButton>     m_pNextPage;
        VclPtr<PushButton>     m_pPrevPage;
        VclPtr<HelpButton>     m_pHelp;

    private:
        std::unique_ptr<WizardMachineImplData> m_xWizardImpl;
        // hold members in this structure to allow keeping compatible when members are added
        std::unique_ptr<RoadmapWizardImpl> m_xRoadmapImpl;

    public:
        RoadmapWizard(vcl::Window* pParent, WinBits nStyle = WB_STDDIALOG, InitFlag eFlag = InitFlag::Default);
        virtual ~RoadmapWizard( ) override;
        virtual void dispose() override;

        virtual void        Resize() override;
        virtual void        StateChanged( StateChangedType nStateChange ) override;
        virtual bool        EventNotify( NotifyEvent& rNEvt ) override;

        void                ActivatePage();

        virtual void        queue_resize(StateChangedType eReason = StateChangedType::Layout) override;

        bool                ShowPage( sal_uInt16 nLevel );
        bool                Finish( tools::Long nResult = 0 );
        sal_uInt16          GetCurLevel() const { return mnCurLevel; }

        void                AddPage( TabPage* pPage );
        void                RemovePage( TabPage* pPage );
        void                SetPage( sal_uInt16 nLevel, TabPage* pPage );
        TabPage*            GetPage( sal_uInt16 nLevel ) const;

        void                AddButton( Button* pButton, tools::Long nOffset = 0 );
        void                RemoveButton( Button* pButton );

        void                SetPageSizePixel( const Size& rSize ) { maPageSize = rSize; }
        const Size&         GetPageSizePixel() const { return maPageSize; }

        /// enable (or disable) buttons
        void                enableButtons(WizardButtonFlags _nWizardButtonFlags, bool _bEnable);

        /// determines whether there is a next state to which we can advance
        bool            canAdvance() const;

        void            SetRoadmapHelpId( const OString& _rId );

        void            InsertRoadmapItem(int nIndex, const OUString& rLabel, int nId, bool bEnabled);
        void            DeleteRoadmapItems();
        int             GetCurrentRoadmapItemID() const;
        void            SelectRoadmapItemByID(int nId, bool bGrabFocus = true);
        void            SetItemSelectHdl( const Link<LinkParamNone*,void>& _rHdl );
        void            ShowRoadmap(bool bShow);

        FactoryFunction GetUITestFactory() const override;

    protected:

        /// to override to create new pages
        VclPtr<TabPage>     createPage(WizardTypes::WizardState nState);

        /// will be called when a new page is about to be displayed
        void                enterState(WizardTypes::WizardState _nState);

        /** will be called when the current state is about to be left for the given reason

            The base implementation in this class will simply call <member>OWizardPage::commitPage</member>
            for the current page, and return whatever this call returns.

            @param _eReason
                The reason why the state is to be left.
            @return
                <TRUE/> if and only if the page is allowed to be left
        */
        bool                prepareLeaveCurrentState( WizardTypes::CommitPageReason eReason );

        /** determine the next state to travel from the given one

            This method ensures that traveling happens along the active path.

            Return WZS_INVALID_STATE to prevent traveling.

            @see activatePath
        */
        WizardTypes::WizardState determineNextState(WizardTypes::WizardState nCurrentState) const;

        /// travel to the next state
        bool                travelNext();

        /// travel to the previous state
        bool                travelPrevious();

        /** enables the automatic enabled/disabled state of the "Next" button

            If this is <TRUE/>, then upon entering a new state, the "Next" button will automatically be
            enabled if and only if determineNextState does not return WZS_INVALID_STATE.
        */
        bool                isAutomaticNextButtonStateEnabled() const;

        /** removes a page from the history. Should be called when the page is being disabled
        */
        void                removePageFromHistory(WizardTypes::WizardState nToRemove);

        /** skips one or more states, until a given state is reached

            The method behaves as if from the current state, <method>travelNext</method>s were called
            successively, until <arg>_nTargetState</arg> is reached, but without actually creating or
            displaying the \EDntermediate pages.

            The skipped states appear in the state history, so <method>travelPrevious</method> will make use of them.

            @return
                <TRUE/> if and only if traveling was successful

            @see skip
            @see skipBackwardUntil
        */
        bool                    skipUntil(WizardTypes::WizardState nTargetState);

        /** moves back one or more states, until a given state is reached

            This method allows traveling backwards more than one state without actually showing the intermediate
            states.

            For instance, if you want to travel two steps backward at a time, you could used
            two travelPrevious calls, but this would <em>show</em> both pages, which is not necessary,
            since you're interested in the target page only. Using <member>skipBackwardUntil</member> relieves
            you of this.

            @return
                <TRUE/> if and only if traveling was successful

            @see skipUntil
            @see skip
        */
        bool                    skipBackwardUntil(WizardTypes::WizardState nTargetState);

        /** returns the current state of the machine

            Vulgo, this is the identifier of the current tab page :)
        */
        WizardTypes::WizardState getCurrentState() const { return GetCurLevel(); }

        static IWizardPageController*  getPageController( TabPage* _pCurrentPage );

        /** returns a human readable name for a given state

            There is a default implementation for this method, which returns the display name
            as given in a call to describeState. If there is no description for the given state,
            this is worth an assertion in a non-product build, and then an empty string is
            returned.
        */
        OUString  getStateDisplayName(WizardTypes::WizardState nState) const;

    private:
        DECL_LINK( OnRoadmapItemSelected, LinkParamNone*, void );

        /** updates the roadmap control to show the given path, as far as possible
            (modulo conflicts with other paths)
        */
        void implUpdateRoadmap( );

    public:
        class AccessGuard
        {
            friend class RoadmapWizardTravelSuspension;
        private:
            AccessGuard() { }
        };

        void                   suspendTraveling( AccessGuard );
        void                   resumeTraveling( AccessGuard );
        bool                   isTravelingSuspended() const;

    protected:
        TabPage* GetOrCreatePage(const WizardTypes::WizardState i_nState);

    private:
        void             ImplCalcSize( Size& rSize );
        void             ImplPosCtrls();
        void             ImplPosTabPage();
        void             ImplShowTabPage( TabPage* pPage );
        TabPage*         ImplGetPage( sal_uInt16 nLevel ) const;


        DECL_LINK(OnNextPage, Button*, void);
        DECL_LINK(OnPrevPage, Button*, void);
        DECL_LINK(OnFinish, Button*, void);

        void     implUpdateTitle();
        void     implConstruct( const WizardButtonFlags _nButtonFlags );
    };

    /// helper class to temporarily suspend any traveling in the wizard
    class RoadmapWizardTravelSuspension
    {
    public:
        RoadmapWizardTravelSuspension(RoadmapWizard& rWizard)
            : m_pOWizard(&rWizard)
        {
            m_pOWizard->suspendTraveling(RoadmapWizard::AccessGuard());
        }

        ~RoadmapWizardTravelSuspension()
        {
            if (m_pOWizard)
                m_pOWizard->resumeTraveling(RoadmapWizard::AccessGuard());
        }

    private:
        VclPtr<RoadmapWizard> m_pOWizard;
    };

} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
