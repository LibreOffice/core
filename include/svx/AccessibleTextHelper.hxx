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

#ifndef INCLUDED_SVX_ACCESSIBLETEXTHELPER_HXX
#define INCLUDED_SVX_ACCESSIBLETEXTHELPER_HXX

#include <memory>
#include <sal/types.h>
#include <tools/gen.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <svx/svxdllapi.h>


class SvxTextForwarder;
class SvxViewForwarder;
class SvxEditSource;

namespace accessibility
{

    class AccessibleTextHelper_Impl;

    /** Helper class for objects containing EditEngine/Outliner text

        This class provides the methods from the XAccessibleContext,
        XAccessibleEventBroadcaster and XAccessibleComponent
        interfaces, that are common to all accessible objects
        containing an edit engine.

        The text contained in the EditEngine/Outliner is presented as
        children of this class, namely for every text paragraph a
        AccessibleEditableTextPara child object is generated. As this
        class manages these children for itself, it has to send out
        AccessibleEventId::CHILD events on your
        behalf. Thus, you must forward every call to your
        addEventListener()/removeEventListener() methods to the
        AccessibleTextHelper (methods
        AddEventListener/RemoveEventListener), otherwise none or not
        every one of your event listener will notice child changes.

        You have to implement the SvxEditSource, SvxTextForwarder,
        SvxViewForwarder and SvxEditViewForwarder interfaces in order
        to enable your object to cooperate with this
        class. SvxTextForwarder encapsulates the fact that text
        objects do not necessarily have an EditEngine at their
        disposal, SvxViewForwarder and SvxEditViewForwarder do the
        same for the document and the edit view. The three mentioned
        forwarder objects are not stored by the AccessibleTextHelper,
        but fetched every time from the SvxEditSource. So you are best
        off making your SvxEditSource::Get*Forwarder methods cache the
        current forwarder.

        To support changes in edit mode or conversion of fixed text
        into EditEngine text, you can change the SvxEditSource this
        class is referring to. This might render all children invalid
        and change the child count, since the AccessibleTextHelper
        reinitializes itself from scratch.

        This class registers itself at the SvxEditSource as a state
        listener and manages the state of its children (i.e. the
        paragraphs). See the method documentation of
        AccessibleTextHelper::SetEditSource for the expected
        events. Generally, be prepared that when sending any of these
        events via SvxEditSource::GetBroadcaster() broadcaster, the
        AccessibleTextHelper will call the SvxEditSource and their
        forwarder to update it's state. Avoid being inconsistent in
        the facts you tell in the events, e.g. when sending a
        TEXT_HINT_PARAINSERTED event, the
        SvxEditSource::GetTextForwarder().GetParagraphCount() should
        already include the newly inserted paragraph.

        @attention All public methods must not be called with any
        mutex hold, except when calling from the main thread (with
        holds the solar mutex), unless stated otherwise. This is
        because they themselves might need the solar mutex in addition
        to the object mutex, and the ordering of the locking must be:
        first solar mutex, then object mutex. Furthermore, state
        change events might be fired internally.

        @derive Use this class in an aggregation and forward, or
        derive from it and overwrite.
        too.

        @see SvxEditSource
        @see SvxTextForwarder
        @see SvxViewForwarder
        @see SvxEditViewForwarder
    */
    class SVX_DLLPUBLIC AccessibleTextHelper
    {

    public:
        typedef ::std::vector< sal_Int16 > VectorOfStates;

        /** Create accessible text object for given edit source

            @param pEditSource
            The edit source to use. Object ownership is transferred
            from the caller to the callee. The object listens on the
            SvxEditSource for object disposal, so no provisions have
            to be taken if the caller destroys the data (e.g. the
            model) contained in the given SvxEditSource.

        */
        explicit AccessibleTextHelper( ::std::unique_ptr< SvxEditSource > && pEditSource );

        virtual ~AccessibleTextHelper();

    protected:

        // declared, but not defined
        AccessibleTextHelper( const AccessibleTextHelper& );
        // declared, but not defined
        AccessibleTextHelper& operator= ( const AccessibleTextHelper& );

    public:
        /** Query the current edit source

            @attention This method returns by reference, so you are
            responsible for serialization (typically, you acquired the
            solar mutex when calling this method). Thus, the method
            should only be called from the main office thread.

         */
        const SvxEditSource& GetEditSource() const;

        /** Set the current edit source

            @attention Might fire state change events, therefore,
            don't hold any mutex except solar mutex, which you are
            required to lock before. This method should only be called
            from the main office thread.

            The EditSource set here is required to broadcast out the
            following hints: EDITSOURCE_HINT_PARASMOVED,
            EDITSOURCE_HINT_SELECTIONCHANGED, TEXT_HINT_MODIFIED,
            TEXT_HINT_PARAINSERTED, TEXT_HINT_PARAREMOVED,
            TEXT_HINT_TEXTHEIGHTCHANGED,
            TEXT_HINT_VIEWSCROLLED. Otherwise, not all state changes
            will get noticed by the accessibility object. Further
            more, when the corresponding core object or the model is
            dying, either the edit source must be set to NULL or it
            has to broadcast a SFX_HINT_DYING hint.

            If the SvxEditSource's managed text can change between
            edit/non-edit mode (i.e. there are times when
            SvxEditSource::GetEditViewForwarder(sal_False) returns
            NULL), then the two additional hints are required:
            HINT_BEGEDIT and HINT_ENDEDIT. When the
            AccessibleTextHelper receives a HINT_BEGEDIT, it expects
            the SvxEditSource already in edit mode. On a HINT_ENDEDIT,
            edit mode must already been left. The rationale for these
            events are the fact that focus and selection have to be
            updated in edit mode, and completely relinquished and
            reset to the parent (for the focus) in non-edit mode.

            This class does not have a dispose method, since it is not
            a UNO component. Nevertheless, it holds C++ references to
            several core objects, so you should issue a
            SetEditSource(::std::unique_ptr<SvxEditSource>()) in
            your dispose() method.

            @param pEditSource
            The new edit source to set. Object ownership is transferred
            from the caller to the callee.
        */
        void SetEditSource( ::std::unique_ptr< SvxEditSource > && pEditSource );

        /** Set the event source

            You should set the event source before registering any
            event listener and before requesting any child. Children
            of this object receive the event source as their parent
            accessible object. That is, the event source is best set
            in your object's init method.

            @attention When setting a reference here, you should call
            Dispose() when you as the owner are disposing, since until
            then this object will hold that reference

            @param rInterface
            The interface that should be set as the source for
            accessibility events sent by this object.
         */
        void SetEventSource( const css::uno::Reference< css::accessibility::XAccessible >& rInterface );

        /** Set offset of EditEngine/Outliner from parent

            If the origin of the underlying EditEngine/Outliner does
            not correspond to the upper left corner of the object
            using this class, you have to specify the offset.

            @attention Might fire state change events, therefore,
            don't hold any mutex except solar mutex, which you are
            required to lock before. This method should only be called
            from the main office thread.

            @param rPoint
            The offset in screen coordinates (i.e. pixel)
        */
        void SetOffset( const Point& rPoint );

        /** Set offset the object adds to all children's indices

            This can be used if the owner of this object has children
            handled by itself. Setting an offset different from 0
            leads to this object mimicking that all its children are
            within the range [nOffset, GetChildCount()+nOffset). That
            means, GetChild() also expects the index to be in this
            range.

            @attention Might fire state change events, therefore,
            don't hold any mutex except solar mutex, which you are
            required to lock before. This method should only be called
            from the main office thread.

            @param nOffset
            The offset to add to every children's index.
        */
        void SetStartIndex( sal_Int32 nOffset );

        /** Query offset the object adds to all children's indices

            @return the offset to add to every children's index.
        */
        sal_Int32 GetStartIndex() const;

        /** Sets a vector of additional accessible states.

            The states are passed to every created child object
            (text paragraph). The state values are defined in
            css::accessibility::AccessibleStateType.

            This function has to be called before querying for
            any children (e.g. with GetChild()).
         */
        void SetAdditionalChildStates( const VectorOfStates& rChildStates );

        /** Update the visible children

            @attention Might fire state change events, therefore,
            don't hold any mutex except solar mutex, which you are
            required to lock before. This method should only be called
            from the main office thread.

            This method reevaluates the visibility of all
            children. Call this method if your visibility state has
            changed somehow, e.g. if the visible area has changed and
            the AccessibleTextHelper isn't notified internally
            (e.g. via TEXT_HINT_VIEWSCROLLED). Normally, there should
            not be a need to call this method.
        */
        void UpdateChildren();

        /** Drop all references and enter disposed state

            This method drops all references to external objects (also
            the event source reference set via SetEventSource()) and
            sets the object into the disposed state (i.e. the methods
            return default values or throw a uno::DisposedException
            exception).
         */
        void Dispose();

        /** Set the focus state of the accessibility object

            Since this class handles children which also might get the
            focus, the user of this class is encouraged to delegate
            focus handling. Whenever the focus state of the
            surrounding object changes, this method has to be called.

            The protocol of focus handling for a user of this class is
            then to call SetFocus() with the appropriate focus state,
            and HaveFocus() to determine the focus state you tell the
            outside.

            @attention Might fire state change events, therefore,
            don't hold any mutex except solar mutex, which you are
            required to lock before. This method should only be called
            from the main office thread.

            @param bHaveFocus
            Whether we got or we lost the focus. Set to true if
            focus is gotten, false otherwise.

            @see HaveFocus()
         */
        void SetFocus( bool bHaveFocus = true );

        /** Query the focus state of the surrounding object

            If focus handling is delegated to this class, determine
            focus state with this method. Be prepared that even if you
            set the focus with SetFocus(true), this method might
            return false. This is the case if one of the children
            actually got the focus.

            @return the state of the focus ownership
         */
        bool HaveFocus();

        // XAccessibleContext child handling methods

        /** Implements getAccessibleChildCount

            @attention Don't call with locked mutexes. You may hold
            the solar mutex, but this method acquires it anyway.
        */
        sal_Int32 GetChildCount();
        /** Implements getAccessibleChild

            @attention Don't call with locked mutexes. You may hold
            the solar mutex, but this method acquires it anyway.
        */
        css::uno::Reference< css::accessibility::XAccessible > GetChild( sal_Int32 i );

        // XAccessibleEventBroadcaster child related methods

        /** Implements addEventListener

            @attention Don't call with locked mutexes
        */
        void AddEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener );
        /** Implements removeEventListener

            @attention Don't call with locked mutexes
        */
        void RemoveEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener );

        // XAccessibleComponent child related methods

        /** Implements getAccessibleAt

            @attention Don't call with locked mutexes. You may hold
            the solar mutex, but this method acquires it anyway.
        */
        css::uno::Reference< css::accessibility::XAccessible > SAL_CALL GetAt( const css::awt::Point& aPoint );

    private:

        /// @dyn
        const std::unique_ptr< AccessibleTextHelper_Impl > mpImpl;

    };

} // end of namespace accessibility

#endif // INCLUDED_SVX_ACCESSIBLETEXTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
