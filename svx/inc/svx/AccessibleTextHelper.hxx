/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_ACCESSILE_TEXT_HELPER_HXX_
#define _SVX_ACCESSILE_TEXT_HELPER_HXX_

#include <memory>
#include <sal/types.h>
#include <tools/gen.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include "svx/svxdllapi.h"


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
        derive from it and overwrite. If the Remove/AddEventListener
        methods are overwritten, make sure FireEvent is adapted,
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
        explicit AccessibleTextHelper( ::std::auto_ptr< SvxEditSource > pEditSource );
        virtual ~AccessibleTextHelper();

    protected:

        // declared, but not defined
        AccessibleTextHelper( const AccessibleTextHelper& );
        // declared, but not defined
        AccessibleTextHelper& operator= ( const AccessibleTextHelper& );

    public:
        /** Query the current edit source

            @attention This method returns by reference, so you are
            responsible for serialization (typically, you aquired the
            solar mutex when calling this method). Thus, the method
            should only be called from the main office thread.

         */
        virtual const SvxEditSource& GetEditSource() const SAL_THROW((::com::sun::star::uno::RuntimeException));

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
            SetEditSource(::std::auto_ptr<SvxEditSource>(NULL)) in
            your dispose() method.

            @param pEditSource
            The new edit source to set. Object ownership is transferred
            from the caller to the callee.
        */
        virtual void SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((::com::sun::star::uno::RuntimeException));

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
        virtual void SetEventSource( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rInterface );

        /** Get the event source

            @return the interface that is set as the source for
            accessibility events sent by this object.
         */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetEventSource() const;

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
        virtual void SetOffset( const Point& rPoint );

        /** Query offset of EditEngine/Outliner from parent

            @return the offset in screen coordinates (i.e. pixel)
        */
        virtual Point GetOffset() const;

        /** Set offset the object adds to all children's indices

            This can be used if the owner of this object has children
            handled by itself. Setting an offset different from 0
            leads to this object mimicking that all it's children are
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
        virtual void SetStartIndex( sal_Int32 nOffset );

        /** Query offset the object adds to all children's indices

            @return the offset to add to every children's index.
        */
        virtual sal_Int32 GetStartIndex() const;

        /** Sets a vector of additional accessible states.

            The states are passed to every created child object
            (text paragraph). The state values are defined in
            com::sun::star::accessibility::AccessibleStateType.

            This function has to be called before querying for
            any children (e.g. with GetChild()).
         */
        void SetAdditionalChildStates( const VectorOfStates& rChildStates );

        /** Returns the additional accessible states for children.
         */
        const VectorOfStates& GetAdditionalChildStates() const;

        /** Update the visible children

            @attention Might fire state change events, therefore,
            don't hold any mutex except solar mutex, which you are
            required to lock before. This method should only be called
            from the main office thread.

            This method reevaluates the visibility of all
            childrens. Call this method if your visibility state has
            changed somehow, e.g. if the visible area has changed and
            the AccessibleTextHelper isn't notified internally
            (e.g. via TEXT_HINT_VIEWSCROLLED). Normally, there should
            not be a need to call this method.
        */
        virtual void UpdateChildren() SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Drop all references and enter disposed state

            This method drops all references to external objects (also
            the event source reference set via SetEventSource()) and
            sets the object into the disposed state (i.e. the methods
            return default values or throw a uno::DisposedException
            exception).
         */
        virtual void Dispose();

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
            Whether we got or we lost the focus. Set to sal_True if
            focus is gotten, sal_False otherwise.

            @see HaveFocus()
         */
        virtual void SetFocus( sal_Bool bHaveFocus = sal_True ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Query the focus state of the surrounding object

            If focus handling is delegated to this class, determine
            focus state with this method. Be prepared that even if you
            set the focus with SetFocus(sal_True), this method might
            return sal_False. This is the case if one of the children
            actually got the focus.

            @return the state of the focus ownership
         */
        virtual sal_Bool HaveFocus() SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Call this method to invoke all event listeners with the given event

            @attention Fires state change events, therefore, don't hold any mutex

            @param nEventId
            Id of the event to send, @see AccessibleEventId

            @param rNewValue
            The value we've changed into

            @param rOldValue
            The old value before the change
        */
        virtual void FireEvent( const sal_Int16 nEventId,
                                const ::com::sun::star::uno::Any& rNewValue = ::com::sun::star::uno::Any(),
                                const ::com::sun::star::uno::Any& rOldValue = ::com::sun::star::uno::Any() ) const;

        /** Call this method to invoke all event listeners with the given event

            @attention Fires state change events, therefore, don't hold any mutex

            @param rEvent
            The event to send, @see AccessibleEventObject

        */
        // TODO: make that virtual next time
        void FireEvent( const ::com::sun::star::accessibility::AccessibleEventObject& rEvent ) const;

        /** Query select state of the text managed by this object

            @attention Don't call with locked mutexes. You may hold
            the solar mutex, but this method aquires it anyway.

            @return sal_True, if the text or parts of it are currently selected
        */
        virtual sal_Bool IsSelected() const;

        // XAccessibleContext child handling methods
        //-----------------------------------------------------------------
        /** Implements getAccessibleChildCount

            @attention Don't call with locked mutexes. You may hold
            the solar mutex, but this method aquires it anyway.
        */
        virtual sal_Int32 GetChildCount() SAL_THROW((::com::sun::star::uno::RuntimeException));
        /** Implements getAccessibleChild

            @attention Don't call with locked mutexes. You may hold
            the solar mutex, but this method aquires it anyway.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetChild( sal_Int32 i ) SAL_THROW((::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException));

        // XAccessibleEventBroadcaster child related methods
        //-----------------------------------------------------------------
        /** Implements addEventListener

            @attention Don't call with locked mutexes
        */
        virtual void AddEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) SAL_THROW((::com::sun::star::uno::RuntimeException));
        /** Implements removeEventListener

            @attention Don't call with locked mutexes
        */
        virtual void RemoveEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        // XAccessibleComponent child related methods
        //-----------------------------------------------------------------
        /** Implements getAccessibleAt

            @attention Don't call with locked mutexes. You may hold
            the solar mutex, but this method aquires it anyway.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL GetAt( const ::com::sun::star::awt::Point& aPoint ) SAL_THROW((::com::sun::star::uno::RuntimeException));

    private:

        /// @dyn
        const std::auto_ptr< AccessibleTextHelper_Impl > mpImpl;

    };

} // end of namespace accessibility

#endif /* _SVX_ACCESSILE_TEXT_HELPER_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
