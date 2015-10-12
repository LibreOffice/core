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

#ifndef INCLUDED_EDITENG_ACCESSIBLESTATICTEXTBASE_HXX
#define INCLUDED_EDITENG_ACCESSIBLESTATICTEXTBASE_HXX

#include <memory>
#include <tools/gen.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <editeng/editengdllapi.h>


class SvxEditSource;
class SvxEditViewForwarder;

namespace accessibility
{

    class AccessibleStaticTextBase_Impl;

    typedef ::cppu::ImplHelper2<
        ::com::sun::star::accessibility::XAccessibleText,
        ::com::sun::star::accessibility::XAccessibleTextAttributes > AccessibleStaticTextBase_BASE;

    /** Helper class for objects containing EditEngine/Outliner text

        This class implements the XAccessibleText interface for static
        text, somewhat similar to the children of the
        AccessibleTextHelper class. Currently, there are no children,
        i.e. the whole text is presented in one big chunk. This might
        change in the future, if a need for image bullets should
        arise. These, by convention, would be represented as children
        of the text.

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

        As this class is intended for static (i.e. non-changing) text
        only, no event broadcasting is necessary. You must handle
        visibility by yourself, the bounding boxes returned by
        getCharacterBounds() are relative to your accessibility
        object.

        @attention All public non-UNO methods (those are the uppercase
        ones) must not be called with any mutex hold, except when
        calling from the main thread (with holds the solar mutex),
        unless stated otherwise. This is because they themselves might
        need the solar mutex in addition to the object mutex, and the
        ordering of the locking must be: first solar mutex, then
        object mutex. Furthermore, state change events might be fired
        internally.

        @derive Use this class as a base for objects containing static
        edit engine text. To avoid overwriting every interface method
        to intercept derived object defunc state, just set NULL as the
        edit source. Every interface method will then properly throw
        an exception.
    */
    class EDITENG_DLLPUBLIC AccessibleStaticTextBase : public AccessibleStaticTextBase_BASE
    {

    public:
        /** Create accessible text object for given edit source

            @param pEditSource
            The edit source to use. Object ownership is transferred
            from the caller to the callee. The object listens on the
            SvxEditSource for object disposal, so no provisions have
            to be taken if the caller destroys the data (e.g. the
            model) contained in the given SvxEditSource.

        */
        explicit AccessibleStaticTextBase( ::std::unique_ptr< SvxEditSource > && pEditSource );

        virtual ~AccessibleStaticTextBase();

    private:
        AccessibleStaticTextBase( const AccessibleStaticTextBase& ) = delete;
        AccessibleStaticTextBase& operator= ( const AccessibleStaticTextBase& ) = delete;

    public:

        /** Set the current edit source

            @attention You are required to have the solar mutex
            locked, when calling this method. Thus, the method should
            only be called from the main office thread.

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

            @attention When setting a reference here, you should call
            Dispose() when you as the owner are disposing, since until
            then this object will hold that reference

            @param rInterface
            The interface that should be set as the source for
            accessibility events sent by this object.
         */
        void SetEventSource( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rInterface );

        /** Set offset of EditEngine from parent

            @attention You are required to have the solar mutex
            locked, when calling this method. Thus, the method should
            only be called from the main office thread.

            If the origin of the underlying EditEngine does
            not correspond to the upper left corner of the object
            using this class, you have to specify the offset.

            @param rPoint
            The offset in screen coordinates (i.e. pixel)
        */
        void SetOffset( const Point& rPoint );

        /** Drop all references and enter disposed state

            This method drops all references to external objects (also
            the event source reference set via SetEventSource()) and
            sets the object into the disposed state (i.e. the methods
            return default values or throw a uno::DisposedException
            exception).
         */
        void Dispose();

        // XAccessibleText interface implementation
        virtual sal_Int32 SAL_CALL getCaretPosition() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getCharacterCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getSelectedText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSelectionStart() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSelectionEnd() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        /// This will only work with a functional SvxEditViewForwarder, i.e. an EditEngine/Outliner in edit mode
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        /// This will only work with a functional SvxEditViewForwarder, i.e. an EditEngine/Outliner in edit mode
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XAccessibleTextAttributes
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getDefaultAttributes( const ::com::sun::star::uno::Sequence< OUString >& RequestedAttributes ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getRunAttributes( sal_Int32 Index, const ::com::sun::star::uno::Sequence< OUString >& RequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // child-related methods from XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);

        // child-related methods from XAccessibleComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        Rectangle GetParagraphBoundingBox() const;

    private:

        /// @dyn
        const std::unique_ptr< AccessibleStaticTextBase_Impl > mpImpl;

    };

} // end of namespace accessibility

#endif // INCLUDED_EDITENG_ACCESSIBLESTATICTEXTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
