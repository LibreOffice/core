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

#ifndef _SVX_ACCESSIBLE_EDITABLE_TEXT_PARA_HXX
#define _SVX_ACCESSIBLE_EDITABLE_TEXT_PARA_HXX

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/compbase9.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>

#include <comphelper/accessibletexthelper.hxx>
#include <comphelper/broadcasthelper.hxx>
#include "editeng/AccessibleParaManager.hxx"
#include "editeng/AccessibleImageBullet.hxx"
#include "editeng/unoedprx.hxx"
#include "editeng/editengdllapi.h"

namespace accessibility
{
    typedef ::cppu::PartialWeakComponentImplHelper9< ::com::sun::star::accessibility::XAccessible,
                                     ::com::sun::star::accessibility::XAccessibleContext,
                                     ::com::sun::star::accessibility::XAccessibleComponent,
                                     ::com::sun::star::accessibility::XAccessibleEditableText,
                                     ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                                     ::com::sun::star::accessibility::XAccessibleTextAttributes,
                                     ::com::sun::star::accessibility::XAccessibleHypertext,
                                     ::com::sun::star::accessibility::XAccessibleMultiLineText,
                                     ::com::sun::star::lang::XServiceInfo >  AccessibleTextParaInterfaceBase;

    /** This class implements the actual text paragraphs for the EditEngine/Outliner UAA
     */
    class EDITENG_DLLPUBLIC AccessibleEditableTextPara : public ::comphelper::OBaseMutex, public AccessibleTextParaInterfaceBase, public ::comphelper::OCommonAccessibleText
    {

    protected:
        // override OCommonAccessibleText methods
        virtual ::rtl::OUString                 implGetText();
        virtual ::com::sun::star::lang::Locale  implGetLocale();
        virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex );
        virtual void                            implGetParagraphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                            implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );

    public:
        /// Create accessible object for given parent
        // #i27138#
        // - add parameter <_pParaManager> (default value NULL)
        //   This has to be the the instance of <AccessibleParaManager>, which
        //   created and manages this accessible paragraph.
        AccessibleEditableTextPara ( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rParent,
                                     const AccessibleParaManager* _pParaManager = NULL );

        virtual ~AccessibleEditableTextPara ();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface (const ::com::sun::star::uno::Type & rType) throw (::com::sun::star::uno::RuntimeException);

        // XComponent

        using WeakComponentImplHelperBase::addEventListener;
        using WeakComponentImplHelperBase::removeEventListener;

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole() throw (::com::sun::star::uno::RuntimeException);
        /// Maximal length of text returned by getAccessibleDescription()
        enum { MaxDescriptionLen = 40 };
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale() throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleComponent
        virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleText (this comes implicitely inherited by XAccessibleEditableText AND by XAccessibleMultiLineText)
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
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleEditableText
        virtual sal_Bool SAL_CALL cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL pasteText( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL insertText( const ::rtl::OUString& sText, sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::rtl::OUString& sReplacement ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aAttributeSet ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setText( const ::rtl::OUString& sText ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleTextAttributes
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getDefaultAttributes( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& RequestedAttributes ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getRunAttributes( ::sal_Int32 Index, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& RequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleHypertext
        virtual ::sal_Int32 SAL_CALL getHyperLinkCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleHyperlink > SAL_CALL getHyperLink( ::sal_Int32 nLinkIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getHyperLinkIndex( ::sal_Int32 nCharIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleMultiLineText
        virtual ::sal_Int32 SAL_CALL getLineNumberAtIndex( ::sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtLineNumber( ::sal_Int32 nLineNo ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtLineWithCaret(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getNumberOfLineWithCaret(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName (void) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService (const ::rtl::OUString& sServiceName) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames (void) throw (::com::sun::star::uno::RuntimeException);

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName (void) throw (::com::sun::star::uno::RuntimeException);

        /** Set the current index in the accessibility parent

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetIndexInParent( sal_Int32 nIndex );

        /** Get the current index in the accessibility parent

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        sal_Int32 GetIndexInParent() const;

        /** Set the current paragraph number

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetParagraphIndex( sal_Int32 nIndex );

        /** Query the current paragraph number (0 - nParas-1)

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        sal_Int32 GetParagraphIndex() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Set the edit engine offset

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetEEOffset( const Point& rOffset );

        /** Set the EditEngine offset

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetEditSource( SvxEditSourceAdapter* pEditSource );

        /** Dispose this object

            Notifies and deregisters the listeners, drops all references.
         */
        void Dispose();

        /// Calls all Listener objects to tell them the change. Don't hold locks when calling this!
        virtual void FireEvent(const sal_Int16 nEventId, const ::com::sun::star::uno::Any& rNewValue = ::com::sun::star::uno::Any(), const ::com::sun::star::uno::Any& rOldValue = ::com::sun::star::uno::Any() ) const;

        /// Queries the given state on the internal state set
        bool HasState( const sal_Int16 nStateId );
        /// Sets the given state on the internal state set and fires STATE_CHANGE event. Don't hold locks when calling this!
        void SetState( const sal_Int16 nStateId );
        /// Unsets the given state on the internal state set and fires STATE_CHANGE event. Don't hold locks when calling this!
        void UnSetState( const sal_Int16 nStateId );

        static Rectangle LogicToPixel( const Rectangle& rRect, const MapMode& rMapMode, SvxViewForwarder& rForwarder );

        SvxEditSourceAdapter& GetEditSource() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Query the SvxTextForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxAccessibleTextAdapter&   GetTextForwarder() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Query the SvxViewForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxViewForwarder&   GetViewForwarder() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Query whether a GetEditViewForwarder( sal_False ) will return a forwarder

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        sal_Bool    HaveEditView() const;

        /** Query the SvxEditViewForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxAccessibleTextEditViewAdapter& GetEditViewForwarder( sal_Bool bCreate = sal_False ) const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Send a TEXT_CHANGED event for this paragraph

            This method internally caters for calculating text
            differences, and sends the appropriate Anys in the
            Accessibility::TEXT_CHANGED event
         */
        void TextChanged();

    private:

        // declared, but not defined
        AccessibleEditableTextPara( const AccessibleEditableTextPara& );
        AccessibleEditableTextPara& operator= ( const AccessibleEditableTextPara& );

        /** Calculate character range of similar attributes

            @param nStartIndex
            Therein, the start of the character range with the same attributes is returned

            @param nEndIndex
            Therein, the end (exclusively) of the character range with the same attributes is returned

            @param nIndex
            The character index at where to look for similar character attributes

            @return sal_False, if the method was not able to determine the range
         */
        sal_Bool GetAttributeRun( sal_uInt16& nStartIndex, sal_uInt16& nEndIndex, sal_Int32 nIndex );

        // syntactic sugar for FireEvent
        void GotPropertyEvent( const ::com::sun::star::uno::Any& rNewValue, const sal_Int16 nEventId ) const;
        void LostPropertyEvent( const ::com::sun::star::uno::Any& rOldValue, const sal_Int16 nEventId ) const;

        /** Query the visibility state

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.

            @return the visibility state. Per definition, a defunc object is no longer visible
         */
        sal_Bool IsVisible() const;

        int getNotifierClientId() const;

        // retrieve text interface for given paragraph index
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleText > GetParaInterface( sal_Int32 nIndex );

        /// Do we have children? This is the case for image bullets
        sal_Bool HaveChildren();

        /// Is the underlying object in edit mode
        sal_Bool IsActive() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        const Point& GetEEOffset() const;

        // Get text from forwarder
        String GetText( sal_Int32 nIndex ) SAL_THROW((::com::sun::star::uno::RuntimeException));
        String GetTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) SAL_THROW((::com::sun::star::uno::RuntimeException));
        sal_uInt16 GetTextLen() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Get the current selection of this paragraph

            @return sal_False, if nothing in this paragraph is selected
         */
        sal_Bool GetSelection( sal_uInt16& nStartPos, sal_uInt16& nEndPos ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** create selection from Accessible selection.

        */
        ESelection  MakeSelection( sal_Int32 nStartEEIndex, sal_Int32 nEndEEIndex );
        ESelection  MakeSelection( sal_Int32 nEEIndex );
        ESelection  MakeCursor( sal_Int32 nEEIndex );

        // check whether index value is within permitted range

        /// Check whether 0<=nIndex<=n-1
        void CheckIndex( sal_Int32 nIndex ) SAL_THROW((::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException));
        /// Check whether 0<=nIndex<=n
        void CheckPosition( sal_Int32 nIndex ) SAL_THROW((::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException));
        /// Check whether 0<=nStart<=n and 0<=nEnd<=n
        void CheckRange( sal_Int32 nStart, sal_Int32 nEnd ) SAL_THROW((::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException));

        // the paragraph index in the edit engine (guarded by solar mutex)
        sal_Int32   mnParagraphIndex;

        // our current index in the parent (guarded by solar mutex)
        sal_Int32   mnIndexInParent;

        // the current edit source (guarded by solar mutex)
        SvxEditSourceAdapter* mpEditSource;

        // the possible child (for image bullets, guarded by solar mutex)
        typedef WeakCppRef < ::com::sun::star::accessibility::XAccessible, AccessibleImageBullet > WeakBullet;
        WeakBullet maImageBullet;

        // the last string used for an Accessibility::TEXT_CHANGED event (guarded by solar mutex)
        ::rtl::OUString maLastTextString;

        // the offset of the underlying EditEngine from the shape/cell (guarded by solar mutex)
        Point maEEOffset;

        // the current state set (updated from SetState/UnSetState and guarded by solar mutex)
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > mxStateSet;

        /// The shape we're the accessible for (unguarded)
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > mxParent;

        /// Our listeners (guarded by maMutex)
        int mnNotifierClientId;

        // Text paragraphs should provide FLOWS_TO and FLOWS_FROM relations (#i27138#)
        // the paragraph manager, which created this instance - is NULL, if
        // instance isn't created by AccessibleParaManager.
        // Needed for method <getAccessibleRelationSet()> to retrieve predecessor
        // paragraph and the successor paragraph.
        const AccessibleParaManager* mpParaManager;
    };

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
