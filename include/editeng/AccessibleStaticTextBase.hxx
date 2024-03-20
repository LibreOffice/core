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
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <editeng/editengdllapi.h>

namespace com::sun::star::accessibility { class XAccessible; }

class SvxEditSource;

namespace accessibility
{

    class AccessibleStaticTextBase_Impl;

    typedef ::cppu::ImplHelper2<
        css::accessibility::XAccessibleText,
        css::accessibility::XAccessibleTextAttributes > AccessibleStaticTextBase_BASE;

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
            following hints: SfxHintId::EditSourceParasMoved,
            SfxHintId::EditSourceSelectionChanged, SfxHintId::TextModified,
            SfxHintId::TextParaInserted, SfxHintId::TextParaRemoved,
            SfxHintId::TextHeightChanged,
            SfxHintId::TextViewScrolled. Otherwise, not all state changes
            will get noticed by the accessibility object. Further
            more, when the corresponding core object or the model is
            dying, either the edit source must be set to NULL or it
            has to broadcast a SfxHintId::Dying hint.

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
        void SetEventSource( const css::uno::Reference< css::accessibility::XAccessible >& rInterface );

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
        virtual sal_Int32 SAL_CALL getCaretPosition() override final;
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override final;
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override final;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
        virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override final;
        virtual sal_Int32 SAL_CALL getCharacterCount() override final;
        virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override final;
        virtual OUString SAL_CALL getSelectedText() override final;
        virtual sal_Int32 SAL_CALL getSelectionStart() override final;
        virtual sal_Int32 SAL_CALL getSelectionEnd() override final;
        /// This will only work with a functional SvxEditViewForwarder, i.e. an EditEngine/Outliner in edit mode
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override final;
        virtual OUString SAL_CALL getText() override final;
        virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override final;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override final;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override final;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override final;
        /// This will only work with a functional SvxEditViewForwarder, i.e. an EditEngine/Outliner in edit mode
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override final;
        virtual sal_Bool SAL_CALL scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override final;

        // XAccessibleTextAttributes
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getDefaultAttributes( const css::uno::Sequence< OUString >& RequestedAttributes ) override final;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getRunAttributes( sal_Int32 Index, const css::uno::Sequence< OUString >& RequestedAttributes ) override final;

        // child-related methods from XAccessibleContext
        /// @throws css::uno::RuntimeException
        virtual sal_Int64 SAL_CALL getAccessibleChildCount();
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::uno::RuntimeException
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i );

        // child-related methods from XAccessibleComponent
        /// @throws css::uno::RuntimeException
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint );

    protected:
        tools::Rectangle GetParagraphBoundingBox() const;

    private:

        /// @dyn
        const std::unique_ptr< AccessibleStaticTextBase_Impl > mpImpl;

    };

} // end of namespace accessibility

#endif // INCLUDED_EDITENG_ACCESSIBLESTATICTEXTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
