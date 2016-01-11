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

#ifndef INCLUDED_EDITENG_ACCESSIBLEEDITABLETEXTPARA_HXX
#define INCLUDED_EDITENG_ACCESSIBLEEDITABLETEXTPARA_HXX

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/compbase.hxx>
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
#include <editeng/AccessibleParaManager.hxx>
#include <editeng/AccessibleImageBullet.hxx>
#include <editeng/unoedprx.hxx>
#include <editeng/editengdllapi.h>

namespace accessibility
{
    typedef ::cppu::WeakComponentImplHelper< css::accessibility::XAccessible,
                                     css::accessibility::XAccessibleContext,
                                     css::accessibility::XAccessibleComponent,
                                     css::accessibility::XAccessibleEditableText,
                                     css::accessibility::XAccessibleEventBroadcaster,
                                     css::accessibility::XAccessibleTextAttributes,
                                     css::accessibility::XAccessibleHypertext,
                                     css::accessibility::XAccessibleMultiLineText,
                                     css::lang::XServiceInfo >  AccessibleTextParaInterfaceBase;

    /** This class implements the actual text paragraphs for the EditEngine/Outliner UAA
     */
    class EDITENG_DLLPUBLIC AccessibleEditableTextPara : public ::comphelper::OBaseMutex, public AccessibleTextParaInterfaceBase, public ::comphelper::OCommonAccessibleText
    {

    protected:
        // override OCommonAccessibleText methods
        virtual OUString                 implGetText() override;
        virtual css::lang::Locale        implGetLocale() override;
        virtual void                     implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;
        virtual void                     implGetParagraphBoundary( css::i18n::Boundary& rBoundary, sal_Int32 nIndex ) override;
        virtual void                     implGetLineBoundary( css::i18n::Boundary& rBoundary, sal_Int32 nIndex ) override;

    public:
        /// Create accessible object for given parent
        // #i27138#
        // - add parameter <_pParaManager> (default value NULL)
        //   This has to be the instance of <AccessibleParaManager>, which
        //   created and manages this accessible paragraph.
        AccessibleEditableTextPara ( const css::uno::Reference< css::accessibility::XAccessible >& rParent,
                                     const AccessibleParaManager* _pParaManager = nullptr );

        virtual ~AccessibleEditableTextPara ();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface (const css::uno::Type & rType) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole() throw (css::uno::RuntimeException, std::exception) override;
        /// Maximal length of text returned by getAccessibleDescription()
        enum { MaxDescriptionLen = 40 };
        virtual OUString SAL_CALL getAccessibleDescription() throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::lang::Locale SAL_CALL getLocale() throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleComponent
        virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Rectangle SAL_CALL getBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Point SAL_CALL getLocation(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleText (this comes implicitly inherited by XAccessibleEditableText AND by XAccessibleMultiLineText)
        virtual sal_Int32 SAL_CALL getCaretPosition() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) throw (css::lang::IndexOutOfBoundsException, css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getCharacterCount() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getSelectedText() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSelectionStart() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSelectionEnd() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

        // XAccessibleEditableText
        virtual sal_Bool SAL_CALL cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL pasteText( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL insertText( const OUString& sText, sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const css::uno::Sequence< css::beans::PropertyValue >& aAttributeSet ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setText( const OUString& sText ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleTextAttributes
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getDefaultAttributes( const css::uno::Sequence< OUString >& RequestedAttributes ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getRunAttributes( ::sal_Int32 Index, const css::uno::Sequence< OUString >& RequestedAttributes ) throw (css::beans::UnknownPropertyException, css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

        // XAccessibleHypertext
        virtual ::sal_Int32 SAL_CALL getHyperLinkCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleHyperlink > SAL_CALL getHyperLink( ::sal_Int32 nLinkIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getHyperLinkIndex( ::sal_Int32 nCharIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

        // XAccessibleMultiLineText
        virtual ::sal_Int32 SAL_CALL getLineNumberAtIndex( ::sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextAtLineNumber( ::sal_Int32 nLineNo ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::TextSegment SAL_CALL getTextAtLineWithCaret(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getNumberOfLineWithCaret(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService (const OUString& sServiceName) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

        // XServiceName
        static OUString SAL_CALL getServiceName() throw (css::uno::RuntimeException);

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
        sal_Int32 GetIndexInParent() const { return mnIndexInParent; }

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
        sal_Int32 GetParagraphIndex() const { return mnParagraphIndex; }

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
        void FireEvent(const sal_Int16 nEventId, const css::uno::Any& rNewValue = css::uno::Any(), const css::uno::Any& rOldValue = css::uno::Any() ) const;

        /// Sets the given state on the internal state set and fires STATE_CHANGE event. Don't hold locks when calling this!
        void SetState( const sal_Int16 nStateId );
        /// Unsets the given state on the internal state set and fires STATE_CHANGE event. Don't hold locks when calling this!
        void UnSetState( const sal_Int16 nStateId );

        static Rectangle LogicToPixel( const Rectangle& rRect, const MapMode& rMapMode, SvxViewForwarder& rForwarder );

        SvxEditSourceAdapter& GetEditSource() const;

        /** Query the SvxTextForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxAccessibleTextAdapter&   GetTextForwarder() const;

        /** Query the SvxViewForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxViewForwarder&   GetViewForwarder() const;

        /** Query whether a GetEditViewForwarder( sal_False ) will return a forwarder

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        bool    HaveEditView() const;

        /** Query the SvxEditViewForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxAccessibleTextEditViewAdapter& GetEditViewForwarder( bool bCreate = false ) const;

        /** Send a TEXT_CHANGED event for this paragraph

            This method internally caters for calculating text
            differences, and sends the appropriate Anys in the
            Accessibility::TEXT_CHANGED event
         */
        void TextChanged();

    private:
        AccessibleEditableTextPara( const AccessibleEditableTextPara& ) = delete;
        AccessibleEditableTextPara& operator= ( const AccessibleEditableTextPara& ) = delete;

        /** Calculate character range of similar attributes

            @param nStartIndex
            Therein, the start of the character range with the same attributes is returned

            @param nEndIndex
            Therein, the end (exclusively) of the character range with the same attributes is returned

            @param nIndex
            The character index at where to look for similar character attributes

            @return false, if the method was not able to determine the range
         */
        bool GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nIndex );

        // syntactic sugar for FireEvent
        void GotPropertyEvent( const css::uno::Any& rNewValue, const sal_Int16 nEventId ) const;
        void LostPropertyEvent( const css::uno::Any& rOldValue, const sal_Int16 nEventId ) const;

        int getNotifierClientId() const { return mnNotifierClientId; }

        /// Do we have children? This is the case for image bullets
        bool HaveChildren();

        const Point& GetEEOffset() const { return maEEOffset; }

        // Get text from forwarder
        OUString GetTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex );
        sal_Int32 GetTextLen() const;

        /** Get the current selection of this paragraph

            @return sal_False, if nothing in this paragraph is selected
         */
        bool GetSelection(sal_Int32& nStartPos, sal_Int32& nEndPos );

        /** create selection from Accessible selection.

        */
        ESelection  MakeSelection( sal_Int32 nStartEEIndex, sal_Int32 nEndEEIndex );
        ESelection  MakeSelection( sal_Int32 nEEIndex );
        ESelection  MakeCursor( sal_Int32 nEEIndex );

        // check whether index value is within permitted range

        /// Check whether 0<=nIndex<=n-1
        void CheckIndex( sal_Int32 nIndex );
        /// Check whether 0<=nIndex<=n
        void CheckPosition( sal_Int32 nIndex );
        /// Check whether 0<=nStart<=n and 0<=nEnd<=n
        void CheckRange( sal_Int32 nStart, sal_Int32 nEnd );

        void _correctValues( const sal_Int32 nIndex, css::uno::Sequence< css::beans::PropertyValue >& rValues );
        sal_Int32 SkipField(sal_Int32 nIndex, bool bForward);
        // get overlapped field, extend return string. Only extend forward for now
        void ExtendByField( css::accessibility::TextSegment& Segment );
        OUString GetFieldTypeNameAtIndex(sal_Int32 nIndex);
        // the paragraph index in the edit engine (guarded by solar mutex)
        sal_Int32   mnParagraphIndex;

        // our current index in the parent (guarded by solar mutex)
        sal_Int32   mnIndexInParent;

        // the current edit source (guarded by solar mutex)
        SvxEditSourceAdapter* mpEditSource;

        // the possible child (for image bullets, guarded by solar mutex)
        typedef WeakCppRef < css::accessibility::XAccessible, AccessibleImageBullet > WeakBullet;
        WeakBullet maImageBullet;

        // the last string used for an Accessibility::TEXT_CHANGED event (guarded by solar mutex)
        OUString maLastTextString;

        // the offset of the underlying EditEngine from the shape/cell (guarded by solar mutex)
        Point maEEOffset;

        // the current state set (updated from SetState/UnSetState and guarded by solar mutex)
        css::uno::Reference< css::accessibility::XAccessibleStateSet > mxStateSet;

        /// The shape we're the accessible for (unguarded)
        css::uno::Reference< css::accessibility::XAccessible > mxParent;

        /// Our listeners (guarded by maMutex)
        int mnNotifierClientId;
private:
        css::uno::Reference< css::accessibility::XAccessible > m_xAccInfo;

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
