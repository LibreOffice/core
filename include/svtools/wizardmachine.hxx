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
#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#define _SVTOOLS_WIZARDMACHINE_HXX_

#include "svtools/svtdllapi.h"
#include <svtools/wizdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/tabpage.hxx>
#include <comphelper/stl_types.hxx>

class Bitmap;
//.........................................................................
namespace svt
{
//.........................................................................

// wizard buttons
#define WZB_NONE                0x0000
#define WZB_NEXT                0x0001
#define WZB_PREVIOUS            0x0002
#define WZB_FINISH              0x0004
#define WZB_CANCEL              0x0008
#define WZB_HELP                0x0010

// wizard states
#define WZS_INVALID_STATE       ((WizardState)-1)

    //=====================================================================
    //= WizardTypes
    //=====================================================================
    struct WizardTypes
    {
        typedef sal_Int16  WizardState;
        enum CommitPageReason
        {
            eTravelForward,         // traveling forward (maybe with skipping pages)
            eTravelBackward,        // traveling backward (maybe with skipping pages)
            eFinish,                // the wizard is about to be finished
            eValidate               // the data should be validated only, no traveling wll happen
        };
    };

    class SAL_NO_VTABLE IWizardPageController
    {
    public:
        //-----------------------------------------------------------------
        // This methods  behave somewhat different than ActivatePage/DeactivatePage
        // The latter are handled by the base class itself whenever changing the pages is in the offing,
        // i.e., when it's already decided which page is the next.
        // We may have situations where the next page depends on the state of the current, which needs
        // to be committed for this.
        // So initializePage and commitPage are designated to initialitzing/committing data on the page.
        virtual void        initializePage() = 0;
        virtual sal_Bool    commitPage( WizardTypes::CommitPageReason _eReason ) = 0;

        /** determines whether or not it is allowed to advance to a next page

            You should make this dependent on the current state of the page only, not on
            states on other pages of the whole dialog.

            The default implementation always returns <TRUE/>.
        */
        virtual bool    canAdvance() const = 0;

    protected:
        ~IWizardPageController() {}
    };

    //=====================================================================
    //= OWizardPage
    //=====================================================================
    class OWizardMachine;
    struct WizardPageImplData;

    class SVT_DLLPUBLIC OWizardPage : public TabPage, public IWizardPageController
    {
    private:
        WizardPageImplData*     m_pImpl;

    public:
        /** @param _pParent
                if the OWizardPage is used in an OWizardMachine, this parameter
                must be the OWizardMachine (which is derived from Window)
         */
        OWizardPage( Window* _pParent, const ResId& _rResId );
        ~OWizardPage();

        // IWizardPageController overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( WizardTypes::CommitPageReason _eReason );
        virtual bool        canAdvance() const;

    protected:
        // TabPage overridables
        virtual void    ActivatePage();

        /** updates the travel-related UI elements of the OWizardMachine we live in (if any)

            If the parent of the tab page is a OWizardMachine, then updateTravelUI at this instance
            is called. Otherwise, nothing happens.
        */
        void    updateDialogTravelUI();
    };

    //=====================================================================
    //= OWizardMachine
    //=====================================================================
    struct WizardMachineImplData;
    /** implements some kind of finite automata, where the states of the automata exactly correlate
        with tab pages.

        That is, the machine can have up to n states, where at each point in time exactly one state is
        the current one. A state being current is represented as one of n tab pages being displayed
        currently.

        The class handles the UI for traveling between the states (e.g. it administrates the <em>Next</em> and
        <em>Previous</em> buttons which you usually find in a wizard.

        Derived classes have to implement the travel logic by overriding <member>determineNextState</member>,
        which has to determine the state which follows the current state. Since this may depend
        on the actual data presented in the wizard (e.g. checkboxes checked, or something like this),
        they can implement non-linear traveling this way.
    */

    class SVT_DLLPUBLIC OWizardMachine : public WizardDialog, public WizardTypes
    {
    private:
        // restrict access to some aspects of our base class
        SVT_DLLPRIVATE void             AddPage( TabPage* pPage ) { WizardDialog::AddPage(pPage); }
        SVT_DLLPRIVATE void             RemovePage( TabPage* pPage ) { WizardDialog::RemovePage(pPage); }
        SVT_DLLPRIVATE void             SetPage( sal_uInt16 nLevel, TabPage* pPage ) { WizardDialog::SetPage(nLevel, pPage); }
        //  TabPage*            GetPage( sal_uInt16 nLevel ) const { return WizardDialog::GetPage(nLevel); }
        // TODO: probably the complete page handling (next, previous etc.) should be prohibited ...

        // IMPORTANT:
        // traveling pages should not be done by calling these base class member, some mechanisms of this class
        // here (e.g. committing page data) depend on having full control over page traveling.
        // So use the travelXXX methods if you need to travel

    protected:
        OKButton*       m_pFinish;
        CancelButton*   m_pCancel;
        PushButton*     m_pNextPage;
        PushButton*     m_pPrevPage;
        HelpButton*     m_pHelp;

    private:
        WizardMachineImplData*
                        m_pImpl;
            // hold members in this structure to allow keeping compatible when members are added

        SVT_DLLPRIVATE void addButtons(Window* _pParent, sal_uInt32 _nButtonFlags);

    public:
        /** ctor

            The ctor does not call FreeResource, this is the resposibility of the derived class.

            For the button flags, use any combination of the WZB_* flags.
        */
        OWizardMachine(Window* _pParent, const ResId& _rRes, sal_uInt32 _nButtonFlags );
        OWizardMachine(Window* _pParent, const WinBits i_nStyle, sal_uInt32 _nButtonFlags );
        ~OWizardMachine();

        /// enable (or disable) buttons
        void    enableButtons(sal_uInt32 _nWizardButtonFlags, sal_Bool _bEnable);
        /// set the default style for a button
        void    defaultButton(sal_uInt32 _nWizardButtonFlags);
        /// set the default style for a button
        void    defaultButton(PushButton* _pNewDefButton);

        /// set the base of the title to use - the title of the current page is appended
        void            setTitleBase(const OUString& _rTitleBase);

        /// determines whether there is a next state to which we can advance
        virtual bool    canAdvance() const;

        /** updates the user interface which deals with traveling in the wizard

            The default implementation simply checks whether both the current page and the wizard
            itself allow to advance to the next state (<code>canAdvance</code>), and enables the "Next"
            button if and only if this is the case.
        */
        virtual void    updateTravelUI();

    protected:
        // WizardDialog overridables
        virtual void        ActivatePage();
        virtual long        DeactivatePage();

        // our own overridables

        /// to override to create new pages
        virtual TabPage*    createPage(WizardState _nState) = 0;

        /// will be called when a new page is about to be displayed
        virtual void        enterState(WizardState _nState);

        /** will be called when the current state is about to be left for the given reason

            The base implementation in this class will simply call <member>OWizardPage::commitPage</member>
            for the current page, and return whatever this call returns.

            @param _eReason
                The reason why the state is to be left.
            @return
                <TRUE/> if and only if the page is allowed to be left
        */
        virtual sal_Bool    prepareLeaveCurrentState( CommitPageReason _eReason );

        /** will be called when the given state is left

            This is the very last possibility for derived classes to veto the deactivation
            of a page.

            @todo Normally, we would not need the return value here - derived classes now have
            the possibility to veto page deactivations in <member>prepareLeaveCurrentState</member>. However,
            changing this return type is too incompatible at the moment ...

            @return
                <TRUE/> if and only if the page is allowed to be left
        */
        virtual sal_Bool    leaveState( WizardState _nState );

        /** determine the next state to travel from the given one

            The default behaviour is linear traveling, overwrite this to change it

            Return WZS_INVALID_STATE to prevent traveling.
        */
        virtual WizardState determineNextState( WizardState _nCurrentState ) const;

        /** called when the finish button is pressed
            <p>By default, only the base class' Finnish method (which is not virtual) is called</p>
        */
        virtual sal_Bool    onFinish();

        /// travel to the next state
        sal_Bool            travelNext();

        /// travel to the previous state
        sal_Bool            travelPrevious();

        /** enables the automatic enabled/disabled state of the "Next" button

            If this is <TRUE/>, then upon entering a new state, the "Next" button will automatically be
            enabled if and only if determineNextState does not return WZS_INVALID_STATE.
        */
        void                enableAutomaticNextButtonState( bool _bEnable = true );
        bool                isAutomaticNextButtonStateEnabled() const;

        /** removes a page from the history. Should be called when the page is being disabled
        */
        void                removePageFromHistory( WizardState nToRemove );

        /** skip a state

            The method behaves as if from the current state, <arg>_nSteps</arg> <method>travelNext</method>s were
            called, but without actually creating or displaying the \EDntermediate pages. Only the
            (<arg>_nSteps</arg> + 1)th page is created.

            The skipped states appear in the state history, so <method>travelPrevious</method> will make use of them.

            A very essential precondition for using this method is that your <method>determineNextState</method>
            method is able to determine the next state without actually having the page of the current state.

            @return
                <TRUE/> if and only if traveling was successful

            @see skipUntil
            @see skipBackwardUntil
        */
        sal_Bool                skip( sal_Int32 _nSteps = 1 );

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
        sal_Bool                skipUntil( WizardState _nTargetState );

        /** moves back one or more states, until a given state is reached

            This method allows traveling backwards more than one state without actually showing the intermediate
            states.

            For instance, if you want to travel two steps backward at a time, you could used
            two travelPrevious calls, but this would <em>show</em> both pages, which is not necessary,
            since you're interested in the target page only. Using <member>skipBackwardUntil</member> reliefs
            you from this.

            @return
                <TRUE/> if and only if traveling was successful

            @see skipUntil
            @see skip
        */
        sal_Bool                skipBackwardUntil( WizardState _nTargetState );

        /** returns the current state of the machine

            Vulgo, this is the identifier of the current tab page :)
        */
        WizardState             getCurrentState() const { return WizardDialog::GetCurLevel(); }

        virtual IWizardPageController*
                                getPageController( TabPage* _pCurrentPage ) const;

        /** retrieves a copy of the state history, i.e. all states we already visited
        */
        void    getStateHistory( ::std::vector< WizardState >& _out_rHistory );

    public:
        class AccessGuard { friend class WizardTravelSuspension; private: AccessGuard() { } };

        void suspendTraveling( AccessGuard );
        void resumeTraveling( AccessGuard );
        bool isTravelingSuspended() const;

    protected:
        TabPage* GetOrCreatePage( const WizardState i_nState );

    private:
       // long OnNextPage( PushButton* );
        DECL_DLLPRIVATE_LINK(OnNextPage, void*);
        DECL_DLLPRIVATE_LINK(OnPrevPage, void*);
        DECL_DLLPRIVATE_LINK(OnFinish, void*);

        SVT_DLLPRIVATE void     implResetDefault(Window* _pWindow);
        SVT_DLLPRIVATE void     implUpdateTitle();
        SVT_DLLPRIVATE void     implConstruct( const sal_uInt32 _nButtonFlags );
    };

    /// helper class to temporarily suspend any traveling in the wizard
    class WizardTravelSuspension
    {
    public:
        WizardTravelSuspension( OWizardMachine& _rWizard )
            :m_rWizard( _rWizard )
        {
            m_rWizard.suspendTraveling( OWizardMachine::AccessGuard() );
        }

        ~WizardTravelSuspension()
        {
            m_rWizard.resumeTraveling( OWizardMachine::AccessGuard() );
        }

    private:
        OWizardMachine& m_rWizard;
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // _SVTOOLS_WIZARDMACHINE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
