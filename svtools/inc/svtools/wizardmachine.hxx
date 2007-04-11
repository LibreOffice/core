/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wizardmachine.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:43:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#define _SVTOOLS_WIZARDMACHINE_HXX_

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SVT_WIZDLG_HXX
#include <svtools/wizdlg.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

class Bitmap;
//.........................................................................
namespace svt
{
//.........................................................................

// wizard buttons
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
            eValidate,              // the data should be validated only, no traveling wll happen
            eValidateNoUI           // the data should be validated only, without displaying error messages and other UI
        };
    };

    class SAL_NO_VTABLE IWizardPage : public WizardTypes
    {
    public:
        // access control
        struct GrantAccess
        {
            friend class OWizardMachine;
        protected:
            GrantAccess() { }
        };

        enum COMMIT_REASON
        {
            CR_TRAVEL_NEXT = eTravelForward,
            CR_TRAVEL_PREVIOUS = eTravelBackward,
            CR_FINISH = eFinish,
            CR_VALIDATE = eValidate,
            CR_VALIDATE_NOUI = eValidateNoUI
        };
    public:
        //-----------------------------------------------------------------
        // methods which, though public, are acessible for the OWizardMachine only
        virtual void enableHeader( const Bitmap& _rBitmap, sal_Int32 _nPixelHeight, GrantAccess ) = 0;
        // This methods  behave somewhat different than ActivatePage/DeactivatePage
        // The latter are handled by the base class itself whenever changing the pages is in the offing,
        // i.e., when it's already decided which page is the next.
        // We may have situations where the next page depends on the state of the current, which needs
        // to be committed for this.
        // So initializePage and commitPage are designated to initialitzing/committing data on the page.
        virtual void        initializePage() = 0;
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason) = 0;
    };
    //=====================================================================
    //= OWizardPage
    //=====================================================================
    class OWizardMachine;
    struct WizardPageImplData;

    class SVT_DLLPUBLIC OWizardPage : public TabPage, public IWizardPage
    {
    private:
        WizardPageImplData*     m_pImpl;

    public:
        OWizardPage( OWizardMachine* _pParent, WinBits _nStyle = 0 );
        OWizardPage( OWizardMachine* _pParent, const ResId& _rResId );
        ~OWizardPage();

        // This methods  behave somewhat different than ActivatePage/DeactivatePage
        // The latter are handled by the base class itself whenever changing the pages is in the offing,
        // i.e., when it's already decided which page is the next.
        // We may have situations where the next page depends on the state of the current, which needs
        // to be committed for this.
        // So initializePage and commitPage are designated to initialitzing/committing data on the page.
        virtual void        initializePage();
        virtual sal_Bool    commitPage(IWizardPage::COMMIT_REASON _eReason);

        //-----------------------------------------------------------------
        // methods which, though public, are acessible for the OWizardMachine only
        virtual void        enableHeader( const Bitmap& _rBitmap, sal_Int32 _nPixelHeight, IWizardPage::GrantAccess );

    protected:
        // TabPage overridables
        virtual void    ActivatePage();

        /** checks whether or not the header is enabled

            The header can only be enabled by the OWizardMachine the page belongs to. This way, it is ensured
            that <em>all</em> or <em>none</em> of the pages have a header.
        */
        sal_Bool        isHeaderEnabled( ) const;

        /** sets the text of the header.

            To be called if the header is enabled only.

            @see isHeaderEnabled
        */
        void            setHeaderText( const String& _rHeaderText );

    protected:
        /** called from within ActivatePage, enables the wizards "Next" button depending on the return value of
            <member>determineNextButtonState</member>
        */
        void implCheckNextButton();

        /** determines whether or not the <em>Next</em> button should be enabled in the current situation.

            The default implementation always returns <TRUE/>.
        */
        virtual sal_Bool determineNextButtonState();
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
        SVT_DLLPRIVATE void             SetPage( USHORT nLevel, TabPage* pPage ) { WizardDialog::SetPage(nLevel, pPage); }
        //  TabPage*            GetPage( USHORT nLevel ) const { return WizardDialog::GetPage(nLevel); }
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
        SVT_DLLPRIVATE long calcRightHelpOffset(sal_uInt32 _nButtonFlags);

    public:
        /** ctor

            The ctor does not call FreeResource, this is the resposibility of the derived class.

            For the button flags, use any combination of the WZB_* flags.
        */
        OWizardMachine(Window* _pParent, const ResId& _rRes, sal_uInt32 _nButtonFlags, sal_Bool _bCheckButtonStates = sal_False, sal_Bool _bRoadmapMode = sal_False, sal_Int16 _nLeftAlignCount = 0  );
        ~OWizardMachine();

        /// enable (or disable) buttons
        void    enableButtons(sal_uInt32 _nWizardButtonFlags, sal_Bool _bEnable);
        /// set the default style for a button
        void    defaultButton(sal_uInt32 _nWizardButtonFlags);
        /// set the default style for a button
        void    defaultButton(PushButton* _pNewDefButton);

        /// set the base of the title to use - the title of the current page is appended
        void            setTitleBase(const String& _rTitleBase);
        const String&   getTitleBase() const;

    protected:
        // WizardDialog overridables
        virtual void        ActivatePage();
        virtual long        DeactivatePage();

        // our own overridables

        /// to override to create new pages
        virtual TabPage*    createPage(WizardState _nState) = 0;

        /// will be called when a new page is about to be displayed
        virtual void            enterState(WizardState _nState);

        /** will be called when the current state is about to be left for the given reason

            The base implementation in this class will simply call <member>OWizardPage::commitPage</member>
            for the current page, and return whatever this call returns.

            @param _eReason
                The reason why the state is to be left.
            @return
                <TRUE/> if and only if the page is allowed to be left
        */
        virtual sal_Bool        prepareLeaveCurrentState( CommitPageReason _eReason );

        /** will be called when the given state is left

            This is the very last possibility for derived classes to veto the deactivation
            of a page.

            @todo Normally, we would not need the return value here - derived classes now have
            the possibility to veto page deactivations in <member>prepareLeaveCurrentState</member>. However,
            changing this return type is too incompatible at the moment ...

            @return
                <TRUE/> if and only if the page is allowed to be left
        */
        virtual sal_Bool        leaveState( WizardState _nState );

        /** determine the next state to travel from the given one

            The default behaviour is linear traveling, overwrite this to change it

            Return WZS_INVALID_STATE to prevent traveling.
        */
        virtual WizardState     determineNextState(WizardState _nCurrentState);

        /** called when the finish button is pressed
            <p>By default, only the base class' Finnish method (which is not virtual) is called</p>
        */
        virtual sal_Bool onFinish(sal_Int32 _nResult);

        /** enables a header bitmap

            Usually, wizards contain a header. This header is as wide as the dialog and positioned at the very top.
            In addition, it contains a (rather small) bitmap on the left side, and right next to this bitmap, a short
            text describing the current page.

            If you call this method, this header is automatically created on every page. In addition, all
            other controls on the pages are moved below the header. The title of every page is used as text
            for the header.

            This method must not be called if there are already pages created.

            @param _rBitmap
                the bitmap to use for the header
            @param _nPixelHeight
                the height of the header in pixels.<br/>
                If -1 is passed, the default of 30 APPFONT units will be used.
        */
        void            enableHeader( const Bitmap& _rBitmap, sal_Int32 _nPixelHeight = -1 );

        /// travel to the next state
        sal_Bool        travelNext();

        /// travel to the previous state
        sal_Bool        travelPrevious();

        /**
            removes a page from the history. Should be called when the page is being disabled
        */
        void  removePageFromHistory( WizardState nToRemove );

        /** skip a state

            The method behaves as if from the current state, <arg>_nSteps</arg> <method>travelNext</method>s were
            called, but without actually creating or displaying the íntermediate pages. Only the
            (<arg>_nSteps</arg> + 1)th page is created.

            The skipped states appear in the state history, so <method>travelPrevious</method> will make use of them.

            A very essential precondition for using this method is that your <method>determineNextState</method>
            method is able to determine the next state without actually having the page of the current state.

            @return
                <TRUE/> if and only if traveling was successfull

            @see skipUntil
            @see skipBackwardUntil
        */
        sal_Bool        skip( sal_Int32 _nSteps = 1 );

        /** skips one or more states, until a given state is reached

            The method behaves as if from the current state, <method>travelNext</method>s were called
            successively, until <arg>_nTargetState</arg> is reached, but without actually creating or
            displaying the íntermediate pages.

            The skipped states appear in the state history, so <method>travelPrevious</method> will make use of them.

            @return
                <TRUE/> if and only if traveling was successfull

            @see skip
            @see skipBackwardUntil
        */
        sal_Bool        skipUntil( WizardState _nTargetState );

        /** moves back one or more states, until a given state is reached

            This method allows traveling backwards more than one state without actually showing the intermediate
            states.

            For instance, if you want to travel two steps backward at a time, you could used
            two travelPrevious calls, but this would <em>show</em> both pages, which is not necessary,
            since you're interested in the target page only. Using <member>skipBackwardUntil</member> reliefs
            you from this.

            @return
                <TRUE/> if and only if traveling was successfull

            @see skipUntil
            @see skip
        */
        sal_Bool        skipBackwardUntil( WizardState _nTargetState );

        /** returns the current state of the machine

            Vulgo, this is the identifier of the current tab page :)
        */
        WizardState     getCurrentState() const { return WizardDialog::GetCurLevel(); }

        virtual IWizardPage*    getWizardPage(TabPage* _pCurrentPage) const;

        /** prevent nested calls of links next/previous or page change with the roadmap control

         */
        bool IsInCallOfLink() const;

        void SetInCallOfLink( bool bSet );

    private:
       // long OnNextPage( PushButton* );
        DECL_DLLPRIVATE_LINK(OnNextPage, PushButton*);
        DECL_DLLPRIVATE_LINK(OnPrevPage, PushButton*);
        DECL_DLLPRIVATE_LINK(OnFinish, PushButton*);

        SVT_DLLPRIVATE void     implResetDefault(Window* _pWindow);
        SVT_DLLPRIVATE void     implUpdateTitle();
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // _SVTOOLS_WIZARDMACHINE_HXX_

