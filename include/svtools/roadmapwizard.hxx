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

#ifndef SVTOOLS_INC_ROADMAPWIZARD_HXX
#define SVTOOLS_INC_ROADMAPWIZARD_HXX

#include "svtools/svtdllapi.h"
#include <svtools/wizardmachine.hxx>

//........................................................................
namespace svt
{
//........................................................................

    struct RoadmapWizardImpl;
    class RoadmapWizard;

    struct RoadmapWizardTypes
    {
    public:
        typedef sal_Int16                                   PathId;
        typedef ::std::vector< WizardTypes::WizardState >   WizardPath;
        typedef TabPage* (* RoadmapPageFactory)( RoadmapWizard& );
    };

    //====================================================================
    //= RoadmapWizard
    //====================================================================
    /** is - no, not a wizard for a roadmap, but the base class for wizards
        <em>supporting</em> a roadmap.

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
    class SVT_DLLPUBLIC RoadmapWizard : public OWizardMachine, public RoadmapWizardTypes
    {
    private:
        RoadmapWizardImpl*  m_pImpl;

    public:
        RoadmapWizard(
            Window* _pParent,
            const ResId& _rRes,
            sal_uInt32 _nButtonFlags = WZB_NEXT | WZB_PREVIOUS | WZB_FINISH | WZB_CANCEL | WZB_HELP
        );
        RoadmapWizard(
            Window* _pParent,
            const WinBits i_nStyle,
            sal_uInt32 _nButtonFlags = WZB_NEXT | WZB_PREVIOUS | WZB_FINISH | WZB_CANCEL | WZB_HELP
        );
        ~RoadmapWizard( );

        void            SetRoadmapHelpId( const OString& _rId );

        void            SetRoadmapInteractive( sal_Bool _bInteractive );
        virtual void    Resize();
        virtual void    StateChanged( StateChangedType nStateChange );

        // returns whether a given state is enabled
        bool            isStateEnabled( WizardState _nState ) const;

        // WizardDialog overridables
        virtual bool    canAdvance() const;
        virtual void    updateTravelUI();

protected:
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

            @param _nFirstState
                the first state in this path. Must not be WZS_INVALID_STATE.<br/>
                Declare an arbitrary number of states after this one, and terminate the sequence
                with a WZS_INVALID_STATE.
        */
        void    declarePath( PathId _nPathId, WizardState _nFirstState, ... );
        void    declarePath( PathId _nPathId, const WizardPath& _lWizardStates);

        /** provides basic information about a state

            The given display name is used in the default implementation of getStateDisplayName,
            and the given factory is used in the default implementation of createPage.
        */
        void    describeState( WizardState _nState, const OUString& _rStateDisplayName, RoadmapPageFactory _pPageFactory );

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
        void    activatePath( PathId _nPathId, bool _bDecideForIt = false );

        /** determine the next state to travel from the given one

            This method (which is declared in OWizardMachine and overwritten here)
            ensures that traveling happens along the active path.

            @see activatePath
        */
        virtual WizardState     determineNextState( WizardState _nCurrentState ) const;

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
        void    enableState( WizardState _nState, bool _bEnable = true );

        /** returns true if and only if the given state is known in at least one declared path
        */
        bool    knowsState( WizardState _nState ) const;

        // OWizardMachine overriables
        virtual void            enterState( WizardState _nState );

        /** returns a human readable name for a given state

            There is a default implementation for this method, which returns the display name
            as given in a call to describeState. If there is no description for the given state,
            this is worth an assertion in a non-product build, and then an empty string is
            returned.
        */
        virtual OUString  getStateDisplayName( WizardState _nState ) const;

        /** creates a page for a given state

            This member is inherited from OWizardMachine, and default-implemented in this class
            for all states which have been described using describeState.
        */
        virtual TabPage*    createPage( WizardState _nState );

        /** asks for a new label of the wizard page

         */
        void    updateRoadmapItemLabel( WizardState _nState );

    protected:
#ifdef DBG_UTIL
        const sal_Char* checkInvariants() const;
        friend const char* CheckInvariants( const void* pVoid );
#endif

    private:
        SVT_DLLPRIVATE void ResizeFixedLine();

        DECL_DLLPRIVATE_LINK( OnRoadmapItemSelected, void* );

        /** updates the roadmap control to show the given path, as far as possible
            (modulo conflicts with other paths)
        */
        SVT_DLLPRIVATE void implUpdateRoadmap( );

    private:
        SVT_DLLPRIVATE void impl_construct();
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // OOO_SVTOOLS_INC_ROADMAPWIZARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
