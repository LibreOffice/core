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

#ifndef SVTOOLS_DIALOGCONTROLLING_HXX
#define SVTOOLS_DIALOGCONTROLLING_HXX

#include <svtools/svtdllapi.h>

#include <tools/link.hxx>
#include <vcl/button.hxx>

#include <vector>
#include <boost/shared_ptr.hpp>

//........................................................................
namespace svt
{
//........................................................................

    //=====================================================================
    //= IWindowOperator
    //=====================================================================
    /** an abstract interface for operating on a ->Window
    */
    class SVT_DLLPUBLIC SAL_NO_VTABLE IWindowOperator
    {
    public:
        /** called when an event happened which should be reacted to

            @param _rTrigger
                the event which triggered the call. If the Id of the event is 0, then this is the initial
                call which is made when ->_rOperateOn is added to the responsibility of the DialogController.
            @param _rOperateOn
                the window on which to operate
        */
        virtual void operateOn( const VclWindowEvent& _rTrigger, Window& _rOperateOn ) const = 0;

        virtual ~IWindowOperator();
    };
    typedef ::boost::shared_ptr< IWindowOperator >  PWindowOperator;

    //=====================================================================
    //= IWindowEventFilter
    //=====================================================================
    /** an abstract interface for deciding whether a ->VclWindowEvent
        is worth paying attention to
    */
    class SVT_DLLPUBLIC SAL_NO_VTABLE IWindowEventFilter
    {
    public:
        virtual bool payAttentionTo( const VclWindowEvent& _rEvent ) const = 0;

        virtual ~IWindowEventFilter();
    };
    typedef ::boost::shared_ptr< IWindowEventFilter >   PWindowEventFilter;

    //=====================================================================
    //= DialogController
    //=====================================================================
    struct DialogController_Data;
    /** a class controlling interactions between dialog controls

        An instance of this class listens to all events fired by a certain
        ->Control (more precise, a ->Window), the so-called instigator.

        Additionally, the ->DialogController maintains a list of windows which
        are affected by changes in the instigator window. Let's call those the
        dependent windows.

        Now, by help of an owner-provided ->IWindowEventFilter, the ->DialogController
        decides which events are worth attention. By help of an owner-provided
        ->IWindowOperator, it handles those events for all dependent windows.
    */
    class SVT_DLLPUBLIC DialogController
    {
    private:
        ::std::auto_ptr< DialogController_Data >    m_pImpl;

    public:
        DialogController( Window& _rInstigator, const PWindowEventFilter& _pEventFilter, const PWindowOperator& _pOperator );
        virtual ~DialogController();

        /** adds a window to the list of dependent windows

            @param _rWindow
                The window to add to the list of dependent windows.

                The caller is responsible for life-time control: The given window
                must live at least as long as the ->DialogController instance does.
        */
        void    addDependentWindow( Window& _rWindow );

        /** resets the controller so that no actions happened anymore.

            The instances is disfunctional after this method has been called.
        */
        void    reset();

    private:
        void    impl_Init();
        void    impl_updateAll( const VclWindowEvent& _rTriggerEvent );
        void    impl_update( const VclWindowEvent& _rTriggerEvent, Window& _rWindow );

        DECL_LINK( OnWindowEvent, const VclWindowEvent* );

    private:
        DialogController( const DialogController& );            // never implemented
        DialogController& operator=( const DialogController& ); // never implemented
    };
    typedef ::boost::shared_ptr< DialogController > PDialogController;

    //=====================================================================
    //= ControlDependencyManager
    //=====================================================================
    struct ControlDependencyManager_Data;
    /** helper class for managing control dependencies

        Instances of this class are intended to be held as members of a dialog/tabpage/whatever
        class, with easy administration of inter-control dependencies (such as "Enable
        control X if and only if checkbox Y is checked).
    */
    class SVT_DLLPUBLIC ControlDependencyManager
    {
    private:
        ::std::auto_ptr< ControlDependencyManager_Data >    m_pImpl;

    public:
        ControlDependencyManager();
        ~ControlDependencyManager();

        /** clears all dialog controllers previously added to the manager
        */
        void    clear();

        /** ensures that a given window is enabled or disabled, according to the check state
            of a given radio button
            @param _rRadio
                denotes the radio button whose check state is to observe
            @param _rDependentWindow
                denotes the window which should be enabled when ->_rRadio is checked, and
                disabled when it's unchecked
        */
        void    enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow );
        void    enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow1, Window& _rDependentWindow2 );
        void    enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3 );
        void    enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3, Window& _rDependentWindow4, Window& _rDependentWindow5 );

        /** ensures that a given window is enabled or disabled, according to the mark state
            of a given check box
            @param _rBox
                denotes the check box whose mark state is to observe
            @param _rDependentWindow
                denotes the window which should be enabled when ->_rBox is marked, and
                disabled when it's unmarked
        */
        void    enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow );
        void    enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow1, Window& _rDependentWindow2 );
        void    enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3, Window& _rDependentWindow4 );

        /** adds a non-standard controller whose functionality is not covered by the other methods

            @param _pController
                the controller to add to the manager. Must not be <NULL/>.
        */
        void    addController( const PDialogController& _pController );

    private:
        ControlDependencyManager( const ControlDependencyManager& );            // never implemented
        ControlDependencyManager& operator=( const ControlDependencyManager& ); // never implemented
    };

    //=====================================================================
    //= EnableOnCheck
    //=====================================================================
    /** a helper class implementing the ->IWindowOperator interface,
        which enables a dependent window depending on the check state of
        an instigator window.

        @see DialogController
    */
    template< class CHECKABLE >
    class SVT_DLLPUBLIC EnableOnCheck : public IWindowOperator
    {
    public:
        typedef CHECKABLE   SourceType;

    private:
        SourceType& m_rCheckable;

    public:
        /** constructs the instance

            @param _rCheckable
                a ->Window instance which supports a boolean method IsChecked. Usually
                a ->RadioButton or ->CheckBox
        */
        EnableOnCheck( SourceType& _rCheckable )
            :m_rCheckable( _rCheckable )
        {
        }

        virtual void operateOn( const VclWindowEvent& /*_rTrigger*/, Window& _rOperateOn ) const
        {
            _rOperateOn.Enable( m_rCheckable.IsChecked() );
        }
    };

    //=====================================================================
    //= FilterForRadioOrCheckToggle
    //=====================================================================
    /** a helper class implementing the ->IWindowEventFilter interface,
        which filters for radio buttons or check boxes being toggled.

        Technically, the class simply filters for the ->VCLEVENT_RADIOBUTTON_TOGGLE
        and the ->VCLEVENT_CHECKBOX_TOGGLE event.
    */
    class SVT_DLLPUBLIC FilterForRadioOrCheckToggle : public IWindowEventFilter
    {
        const Window&   m_rWindow;
    public:
        FilterForRadioOrCheckToggle( const Window& _rWindow )
            :m_rWindow( _rWindow )
        {
        }

        bool payAttentionTo( const VclWindowEvent& _rEvent ) const
        {
            if  (   ( _rEvent.GetWindow() == &m_rWindow )
                &&  (   ( _rEvent.GetId() == VCLEVENT_RADIOBUTTON_TOGGLE )
                    ||  ( _rEvent.GetId() == VCLEVENT_CHECKBOX_TOGGLE )
                    )
                )
                return true;
            return false;
        }
    };

    //=====================================================================
    //= RadioDependentEnabler
    //=====================================================================
    /** a ->DialogController derivee which enables or disables its dependent windows,
        depending on the check state of a radio button.

        The usage of this class is as simple as
        <code>
        pController = new RadioDependentEnabler( m_aOptionSelectSomething );
        pController->addDependentWindow( m_aLabelSelection );
        pController->addDependentWindow( m_aListSelection );
        </code>

        With this, both <code>m_aLabelSelection</code> and <code>m_aListSelection</code> will
        be disabled if and only <code>m_aOptionSelectSomething</code> is checked.
    */
    class SVT_DLLPUBLIC RadioDependentEnabler : public DialogController
    {
    public:
        RadioDependentEnabler( RadioButton& _rButton )
            :DialogController( _rButton,
                PWindowEventFilter( new FilterForRadioOrCheckToggle( _rButton ) ),
                PWindowOperator( new EnableOnCheck< RadioButton >( _rButton ) ) )
        {
        }

        RadioDependentEnabler( CheckBox& _rBox )
            :DialogController( _rBox,
                PWindowEventFilter( new FilterForRadioOrCheckToggle( _rBox ) ),
                PWindowOperator( new EnableOnCheck< CheckBox >( _rBox ) ) )
        {
        }
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVTOOLS_DIALOGCONTROLLING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
