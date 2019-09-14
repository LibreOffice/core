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
#include <vcl/dllapi.h>
#include <vcl/roadmapwizard.hxx>

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
    class VCL_DLLPUBLIC RoadmapWizard : public ModalDialog
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
        WindowAlign             meViewAlign;
        Link<RoadmapWizard*,void>  maActivateHdl;
        sal_Int16               mnLeftAlignCount;
        bool                    mbEmptyViewMargin;

        DECL_DLLPRIVATE_LINK( ImplHandleWizardLayoutTimerHdl, Timer*, void );

        // IMPORTANT:
        // traveling pages should not be done by calling these base class member, some mechanisms of this class
        // here (e.g. committing page data) depend on having full control over page traveling.
        // So use the travelXXX methods if you need to travel

    protected:
        long                LogicalCoordinateToPixel(int iCoordinate);
        /**sets the number of buttons which should be left-aligned. Normally, buttons are right-aligned.

            only to be used during construction, before any layouting happened
        */
        void                SetLeftAlignedButtonCount( sal_Int16 _nCount );
        /** declares the view area to have an empty margin

            Normally, the view area has a certain margin to the top/left/bottom/right of the
            dialog. By calling this method, you can reduce this margin to 0.
        */
        void                SetEmptyViewMargin();

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
        RoadmapWizard(vcl::Window* pParent);
        virtual ~RoadmapWizard( ) override;
        virtual void dispose() override;

        virtual void        Resize() override;
        virtual void        StateChanged( StateChangedType nStateChange ) override;
        virtual bool        EventNotify( NotifyEvent& rNEvt ) override;

        virtual void        ActivatePage();
        virtual bool        DeactivatePage();

        virtual void        queue_resize(StateChangedType eReason = StateChangedType::Layout) override;

        bool                ShowPrevPage();
        bool                ShowNextPage();
        bool                ShowPage( sal_uInt16 nLevel );
        bool                Finish( long nResult = 0 );
        sal_uInt16          GetCurLevel() const { return mnCurLevel; }

        void                AddPage( TabPage* pPage );
        void                RemovePage( TabPage* pPage );
        void                SetPage( sal_uInt16 nLevel, TabPage* pPage );
        TabPage*            GetPage( sal_uInt16 nLevel ) const;

        void                AddButton( Button* pButton, long nOffset = 0 );
        void                RemoveButton( Button* pButton );

        void                SetPrevButton( PushButton* pButton ) { mpPrevBtn = pButton; }
        void                SetNextButton( PushButton* pButton ) { mpNextBtn = pButton; }

        void                SetViewWindow( vcl::Window* pWindow ) { mpViewWindow = pWindow; }
        void                SetViewAlign( WindowAlign eAlign ) { meViewAlign = eAlign; }

        void                SetPageSizePixel( const Size& rSize ) { maPageSize = rSize; }
        const Size&         GetPageSizePixel() const { return maPageSize; }

        void                SetActivatePageHdl( const Link<RoadmapWizard*,void>& rLink ) { maActivateHdl = rLink; }

        /// enable (or disable) buttons
        void                enableButtons(WizardButtonFlags _nWizardButtonFlags, bool _bEnable);
        /// set the default style for a button
        void                defaultButton(WizardButtonFlags _nWizardButtonFlags);
        /// set the default style for a button
        void                defaultButton(PushButton* _pNewDefButton);

        /// set the base of the title to use - the title of the current page is appended
        void                setTitleBase(const OUString& _rTitleBase);

        /// determines whether there is a next state to which we can advance
        virtual bool        canAdvance() const;

        /** updates the user interface which deals with traveling in the wizard

            The default implementation simply checks whether both the current page and the wizard
            itself allow to advance to the next state (<code>canAdvance</code>), and enables the "Next"
            button if and only if this is the case.
        */
        virtual void        updateTravelUI();

        void            SetRoadmapHelpId( const OString& _rId );

        void            SetRoadmapInteractive( bool _bInteractive );

        void            InsertRoadmapItem(int nIndex, const OUString& rLabel, int nId, bool bEnabled);
        void            DeleteRoadmapItems();
        int             GetCurrentRoadmapItemID() const;
        void            SelectRoadmapItemByID(int nId);
        void            SetItemSelectHdl( const Link<LinkParamNone*,void>& _rHdl );
        void            ShowRoadmap(bool bShow);

        // returns whether a given state is enabled
        bool            isStateEnabled( WizardTypes::WizardState _nState ) const;

    protected:

        /// to override to create new pages
        virtual VclPtr<TabPage> createPage(WizardTypes::WizardState nState);

        /// will be called when a new page is about to be displayed
        virtual void        enterState(WizardTypes::WizardState _nState);

        /** will be called when the current state is about to be left for the given reason

            The base implementation in this class will simply call <member>OWizardPage::commitPage</member>
            for the current page, and return whatever this call returns.

            @param _eReason
                The reason why the state is to be left.
            @return
                <TRUE/> if and only if the page is allowed to be left
        */
        virtual bool        prepareLeaveCurrentState( WizardTypes::CommitPageReason eReason );

        /** will be called when the given state is left

            This is the very last possibility for derived classes to veto the deactivation
            of a page.

            @todo Normally, we would not need the return value here - derived classes now have
            the possibility to veto page deactivations in <member>prepareLeaveCurrentState</member>. However,
            changing this return type is too incompatible at the moment ...

            @return
                <TRUE/> if and only if the page is allowed to be left
        */
        virtual bool        leaveState(WizardTypes::WizardState _nState);

        /** determine the next state to travel from the given one

            This method ensures that traveling happens along the active path.

            Return WZS_INVALID_STATE to prevent traveling.

            @see activatePath
        */
        virtual WizardTypes::WizardState determineNextState(WizardTypes::WizardState nCurrentState) const;

        /** called when the finish button is pressed
            <p>By default, only the base class' Finish method (which is not virtual) is called</p>
        */
        virtual bool        onFinish();

        /// travel to the next state
        bool                travelNext();

        /// travel to the previous state
        bool                travelPrevious();

        /** enables the automatic enabled/disabled state of the "Next" button

            If this is <TRUE/>, then upon entering a new state, the "Next" button will automatically be
            enabled if and only if determineNextState does not return WZS_INVALID_STATE.
        */
        void                enableAutomaticNextButtonState();
        bool                isAutomaticNextButtonStateEnabled() const;

        /** removes a page from the history. Should be called when the page is being disabled
        */
        void                removePageFromHistory(WizardTypes::WizardState nToRemove);

        /** skip a state

            The method behaves as if from the current state, <arg>_nSteps</arg> <method>travelNext</method>s were
            called, but without actually creating or displaying the \EDntermediate pages. Only the
            (<arg>_nSteps</arg> + 1)th page is created.

            The skipped states appear in the state history, so <method>travelPrevious</method> will make use of them.

            A very essential precondition for using this method is that your <method>determineNextState</method>
            method is able to determine the next state without actually having the page of the current state.

            @see skipUntil
            @see skipBackwardUntil
        */
        void                    skip();

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

        virtual IWizardPageController*
                                getPageController( TabPage* _pCurrentPage ) const;

        /** retrieves a copy of the state history, i.e. all states we already visited
        */
        void                    getStateHistory(std::vector<WizardTypes::WizardState>& out_rHistory);

        /** declares a valid path in the wizard

            The very first path which is declared is automatically activated.

            Note that all paths which are declared must have the very first state in
            common. Also note that due to a restriction of the very base class (WizardDialog),
            this common first state must be 0.

            You cannot declare new paths once the wizard started, so it's recommended that
            you do all declarations within your derivee's constructor.

            @see activatePath

            @param _nId
                the unique id you wish to give this path. This id can later on be used
                to refer to the path which you just declared
        */
        void    declarePath( RoadmapWizardTypes::PathId _nPathId, const RoadmapWizardTypes::WizardPath& _lWizardStates);

        /** activates a path which has previously been declared with <member>declarePath</member>

            You can only activate paths which share the first <code>k</code> states with the path
            which is previously active (if any), where <code>k</code> is the index of the
            current state within the current path.

            <example>
            Say you have paths, <code>(0,1,2,5)</code> and <code>(0,1,4,5)</code>. This means that after
            step <code>1</code>, you either continue with state <code>2</code> or state <code>4</code>,
            and after this, you finish in state <code>5</code>.<br/>
            Now if the first path is active, and your current state is <code>1</code>, then you can
            easily switch to the second path, since both paths start with <code>(0,1)</code>.<br/>
            However, if your current state is <code>2</code>, then you can not switch to the second
            path anymore.
            </example>

            @param _nPathId
                the id of the path. The path must have been declared (under this id) with
                <member>declarePath</member> before it can be activated.

            @param _bDecideForIt
                If <TRUE/>, the path will be completely activated, even if it is a conflicting path
                (i.e. there is another path which shares the first <code>k</code> states with
                the to-be-activated path.)<br/>
                If <FALSE/>, then the new path is checked for conflicts with other paths. If such
                conflicts exists, the path is not completely activated, but only up to the point
                where it does <em>not</em> conflict.<br/>
                With the paths in the example above, if you activate the second path (when both are
                already declared), then only steps <code>0</code> and <code>1</code> are activated,
                since they are common to both paths.
        */
        void    activatePath( RoadmapWizardTypes::PathId _nPathId, bool _bDecideForIt = false );

        /** en- or disables a state

            In the wizard's roadmap, states to travel to can be freely chosen. To prevent
            users from selecting a state which is currently not available, you can declare this
            state as being disabled.

            A situation where you need this may be when you have a checkbox which, when checked
            by the user, enables a page with additional settings. As long as this checkbox is
            not checked, the respective state would be disabled.

            Note that in theory, you can declare multiple paths, instead of disabling states.
            For instance, if you have a path where one state can be potentially disabled, then
            you could declare a second path, which does not contain this state. However, the
            disadvantage is that then, not the complete path would be visible in the roadmap,
            but only all steps up to the point where the both paths diverge.<br/>
            Another disadvantage is that the number of needed paths grows exponentially with
            the number of states which can be potentially disabled.

            @see declarePath
        */
        void    enableState(WizardTypes::WizardState nState, bool _bEnable = true);

        /** returns true if and only if the given state is known in at least one declared path
        */
        bool    knowsState(WizardTypes::WizardState nState) const;

        /** returns a human readable name for a given state

            There is a default implementation for this method, which returns the display name
            as given in a call to describeState. If there is no description for the given state,
            this is worth an assertion in a non-product build, and then an empty string is
            returned.
        */
        virtual OUString  getStateDisplayName(WizardTypes::WizardState nState) const;

    private:
        DECL_DLLPRIVATE_LINK( OnRoadmapItemSelected, LinkParamNone*, void );

        /** updates the roadmap control to show the given path, as far as possible
            (modulo conflicts with other paths)
        */
        VCL_DLLPRIVATE void implUpdateRoadmap( );

        VCL_DLLPRIVATE void impl_construct();

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
        VCL_DLLPRIVATE void             ImplInitData();
        VCL_DLLPRIVATE void             ImplCalcSize( Size& rSize );
        VCL_DLLPRIVATE void             ImplPosCtrls();
        VCL_DLLPRIVATE void             ImplPosTabPage();
        VCL_DLLPRIVATE void             ImplShowTabPage( TabPage* pPage );
        VCL_DLLPRIVATE TabPage*         ImplGetPage( sal_uInt16 nLevel ) const;


        DECL_DLLPRIVATE_LINK(OnNextPage, Button*, void);
        DECL_DLLPRIVATE_LINK(OnPrevPage, Button*, void);
        DECL_DLLPRIVATE_LINK(OnFinish, Button*, void);

        VCL_DLLPRIVATE void     implResetDefault(vcl::Window const * _pWindow);
        VCL_DLLPRIVATE void     implUpdateTitle();
        VCL_DLLPRIVATE void     implConstruct( const WizardButtonFlags _nButtonFlags );
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
