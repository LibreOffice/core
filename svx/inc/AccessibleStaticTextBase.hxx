/*************************************************************************
 *
 *  $RCSfile: AccessibleStaticTextBase.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:50:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_ACCESSILE_STATIC_TEXT_BASE_HXX_
#define _SVX_ACCESSILE_STATIC_TEXT_BASE_HXX_

#include <memory>

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#endif

class SvxEditSource;

namespace accessibility
{

    class AccessibleStaticTextBase_Impl;

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
    class AccessibleStaticTextBase : public ::com::sun::star::accessibility::XAccessibleText
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
        explicit AccessibleStaticTextBase( ::std::auto_ptr< SvxEditSource > pEditSource );
        virtual ~AccessibleStaticTextBase();

    private:

        // declared, but not defined
        AccessibleStaticTextBase( const AccessibleStaticTextBase& );
        // declared, but not defined
        AccessibleStaticTextBase& operator= ( const AccessibleStaticTextBase& );

    public:
        /** Query the current edit source

            @attention This method returns by reference, so you are
            responsible for serialization (typically, you aquired the
            solar mutex when calling this method). Thus, the method
            should only be called from the main office thread.

        */
        virtual const SvxEditSource& GetEditSource() const SAL_THROW((::com::sun::star::uno::RuntimeException));

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
            SetEditSource(::std::auto_ptr<SvxEditSource>(NULL)) in
            your dispose() method.

            @param pEditSource
            The new edit source to set. Object ownership is transferred
            from the caller to the callee.
        */
        virtual void SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Set the event source

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
        virtual void SetOffset( const Point& rPoint );

        /** Query offset of EditEngine from parent

            @return the offset in screen coordinates (i.e. pixel)
        */
        virtual Point GetOffset() const;

        /** Update the visible children

            As this class currently does not represent any content
            using children, this does nothing at the moment.

            @attention You are required to have the solar mutex
            locked, when calling this method. Thus, the method should
            only be called from the main office thread.

            This method reevaluates the visibility of all
            childrens. Call this method if your visibility state has
            changed somehow, e.g. if the visible area has changed and
            the AccessibleStaticTextHelper isn't notified
            internally. Normally, there should not be a need to call
            this method.
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

        // XAccessibleText interface implementation
        virtual sal_Int32 SAL_CALL getCaretPosition() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getCharacterCount() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getSelectedText() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSelectionStart() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSelectionEnd() throw (::com::sun::star::uno::RuntimeException);
        /// This will only work with a functional SvxEditViewForwarder, i.e. an EditEngine/Outliner in edit mode
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::rtl::OUString SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::rtl::OUString SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::rtl::OUString SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        /// This will only work with a functional SvxEditViewForwarder, i.e. an EditEngine/Outliner in edit mode
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // child-related methods from XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // child-related methods from XAccessibleComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);

    private:

        /// @dyn
        const std::auto_ptr< AccessibleStaticTextBase_Impl > mpImpl;

    };

} // end of namespace accessibility

#endif /* _SVX_ACCESSILE_STATIC_TEXT_BASE_HXX_ */
