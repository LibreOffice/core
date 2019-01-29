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
#ifndef INCLUDED_SVTOOLS_WIZARDMACHINE_HXX
#define INCLUDED_SVTOOLS_WIZARDMACHINE_HXX

#include <memory>
#include <svtools/svtdllapi.h>
#include <svtools/wizdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/tabpage.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace weld {
    class Builder;
    class Container;
}

enum class WizardButtonFlags
{
    NONE                = 0x0000,
    NEXT                = 0x0001,
    PREVIOUS            = 0x0002,
    FINISH              = 0x0004,
    CANCEL              = 0x0008,
    HELP                = 0x0010,
};
namespace o3tl
{
    template<> struct typed_flags<WizardButtonFlags> : is_typed_flags<WizardButtonFlags, 0x001f> {};
}

namespace svt
{


// wizard states
#define WZS_INVALID_STATE       (WizardState(-1))


    //= WizardTypes

    struct WizardTypes
    {
        typedef sal_Int16  WizardState;
        enum CommitPageReason
        {
            eTravelForward,         // traveling forward (maybe with skipping pages)
            eTravelBackward,        // traveling backward (maybe with skipping pages)
            eFinish,                // the wizard is about to be finished
            eValidate               // the data should be validated only, no traveling will happen
        };
    };

    class SAL_NO_VTABLE IWizardPageController
    {
    public:

        // This methods  behave somewhat different than ActivatePage/DeactivatePage
        // The latter are handled by the base class itself whenever changing the pages is in the offing,
        // i.e., when it's already decided which page is the next.
        // We may have situations where the next page depends on the state of the current, which needs
        // to be committed for this.
        // So initializePage and commitPage are designated to initializing/committing data on the page.
        virtual void        initializePage() = 0;
        virtual bool        commitPage( WizardTypes::CommitPageReason _eReason ) = 0;

        /** determines whether or not it is allowed to advance to a next page

            You should make this dependent on the current state of the page only, not on
            states on other pages of the whole dialog.

            The default implementation always returns <TRUE/>.
        */
        virtual bool        canAdvance() const = 0;

    protected:
        ~IWizardPageController() {}
    };


    //= OWizardPage

    class SVT_DLLPUBLIC OWizardPage : public TabPage, public IWizardPageController
    {

    public:
        /** @param _pParent
                if the OWizardPage is used in an OWizardMachine, this parameter
                must be the OWizardMachine (which is derived from Window)
         */
        OWizardPage(vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription);
        OWizardPage(TabPageParent pParent, const OUString& rUIXMLDescription, const OString& rID);
        virtual void dispose() override;
        virtual ~OWizardPage() override;

        // IWizardPageController overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;

    protected:
        std::unique_ptr<weld::Builder> m_xBuilder;
        std::unique_ptr<weld::Container> m_xContainer;

        // TabPage overridables
        virtual void        ActivatePage() override;

        /** updates the travel-related UI elements of the OWizardMachine we live in (if any)

            If the parent of the tab page is a OWizardMachine, then updateTravelUI at this instance
            is called. Otherwise, nothing happens.
        */
        void                updateDialogTravelUI();
    };


    //= OWizardMachine

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
        using WizardDialog::AddPage;
        using WizardDialog::SetPage;
        //  TabPage*            GetPage( sal_uInt16 nLevel ) const { return WizardDialog::GetPage(nLevel); }
        // TODO: probably the complete page handling (next, previous etc.) should be prohibited ...

        // IMPORTANT:
        // traveling pages should not be done by calling these base class member, some mechanisms of this class
        // here (e.g. committing page data) depend on having full control over page traveling.
        // So use the travelXXX methods if you need to travel

    protected:
        VclPtr<OKButton>       m_pFinish;
        VclPtr<CancelButton>   m_pCancel;
        VclPtr<PushButton>     m_pNextPage;
        VclPtr<PushButton>     m_pPrevPage;
        VclPtr<HelpButton>     m_pHelp;

    private:
        // hold members in this structure to allow keeping compatible when members are added
        std::unique_ptr<WizardMachineImplData>  m_pImpl;

    public:
        OWizardMachine(vcl::Window* _pParent, WizardButtonFlags _nButtonFlags );
        virtual ~OWizardMachine() override;
        virtual void dispose() override;

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

    protected:
        // WizardDialog overridables
        virtual void        ActivatePage() override;
        virtual bool        DeactivatePage() override;

        // our own overridables

        /// to override to create new pages
        virtual VclPtr<TabPage> createPage(WizardState _nState) = 0;

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
        virtual bool        prepareLeaveCurrentState( CommitPageReason _eReason );

        /** will be called when the given state is left

            This is the very last possibility for derived classes to veto the deactivation
            of a page.

            @todo Normally, we would not need the return value here - derived classes now have
            the possibility to veto page deactivations in <member>prepareLeaveCurrentState</member>. However,
            changing this return type is too incompatible at the moment ...

            @return
                <TRUE/> if and only if the page is allowed to be left
        */
        virtual bool        leaveState( WizardState _nState );

        /** determine the next state to travel from the given one

            The default behaviour is linear traveling, overwrite this to change it

            Return WZS_INVALID_STATE to prevent traveling.
        */
        virtual WizardState determineNextState( WizardState _nCurrentState ) const;

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
        void                removePageFromHistory( WizardState nToRemove );

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
        bool                    skipUntil( WizardState _nTargetState );

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
        bool                    skipBackwardUntil( WizardState _nTargetState );

        /** returns the current state of the machine

            Vulgo, this is the identifier of the current tab page :)
        */
        WizardState             getCurrentState() const { return WizardDialog::GetCurLevel(); }

        virtual IWizardPageController*
                                getPageController( TabPage* _pCurrentPage ) const;

        /** retrieves a copy of the state history, i.e. all states we already visited
        */
        void                    getStateHistory( ::std::vector< WizardState >& _out_rHistory );

    public:
        class AccessGuard
        {
            friend class WizardTravelSuspension;
        private:
            AccessGuard() { }
        };

        void                   suspendTraveling( AccessGuard );
        void                   resumeTraveling( AccessGuard );
        bool                   isTravelingSuspended() const;

    protected:
        TabPage* GetOrCreatePage( const WizardState i_nState );

    private:
        DECL_DLLPRIVATE_LINK(OnNextPage, Button*, void);
        DECL_DLLPRIVATE_LINK(OnPrevPage, Button*, void);
        DECL_DLLPRIVATE_LINK(OnFinish, Button*, void);

        SVT_DLLPRIVATE void     implResetDefault(vcl::Window const * _pWindow);
        SVT_DLLPRIVATE void     implUpdateTitle();
        SVT_DLLPRIVATE void     implConstruct( const WizardButtonFlags _nButtonFlags );
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


}   // namespace svt


#endif // INCLUDED_SVTOOLS_WIZARDMACHINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
