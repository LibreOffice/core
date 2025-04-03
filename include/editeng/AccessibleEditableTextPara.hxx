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

#include <config_options.h>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <comphelper/compbase.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include <comphelper/accessibletexthelper.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editengdllapi.h>
#include <editeng/unoedprx.hxx>
#include <unotools/weakref.hxx>

class SvxViewForwarder;
class MapMode;
class SvxAccessibleTextAdapter;
class SvxAccessibleTextEditViewAdapter;
namespace accessibility { class AccessibleImageBullet; }

namespace accessibility
{

class AccessibleParaManager;

typedef ::comphelper::WeakComponentImplHelper< css::accessibility::XAccessible,
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
class UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) AccessibleEditableTextPara final : public AccessibleTextParaInterfaceBase, private ::comphelper::OCommonAccessibleText
{

    // override OCommonAccessibleText methods
    virtual OUString                 implGetText() override;
    virtual css::lang::Locale        implGetLocale() override;
    virtual void                     implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;
    virtual void                     implGetParagraphBoundary( const OUString& rtext, css::i18n::Boundary& rBoundary, sal_Int32 nIndex ) override;
    virtual void                     implGetLineBoundary( const OUString& rtext, css::i18n::Boundary& rBoundary, sal_Int32 nIndex ) override;

public:
    /// Create accessible object for given parent
    // #i27138#
    // - add parameter <_pParaManager> (default value NULL)
    //   This has to be the instance of <AccessibleParaManager>, which
    //   created and manages this accessible paragraph.
    AccessibleEditableTextPara ( css::uno::Reference< css::accessibility::XAccessible > xParent,
                                 const AccessibleParaManager* _pParaManager = nullptr );

    virtual ~AccessibleEditableTextPara () override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface (const css::uno::Type & rType) override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent() override;
    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;
    /// Maximal length of text returned by getAccessibleDescription()
    enum { MaxDescriptionLen = 40 };
    virtual OUString SAL_CALL getAccessibleDescription() override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet() override;
    virtual css::lang::Locale SAL_CALL getLocale() override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleText (this comes implicitly inherited by XAccessibleEditableText AND by XAccessibleMultiLineText)
    virtual sal_Int32 SAL_CALL getCaretPosition() override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getCharacterCount() override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString SAL_CALL getSelectedText() override;
    virtual sal_Int32 SAL_CALL getSelectionStart() override;
    virtual sal_Int32 SAL_CALL getSelectionEnd() override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString SAL_CALL getText() override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleEditableText
    virtual sal_Bool SAL_CALL cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL pasteText( sal_Int32 nIndex ) override;
    virtual sal_Bool SAL_CALL deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL insertText( const OUString& sText, sal_Int32 nIndex ) override;
    virtual sal_Bool SAL_CALL replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) override;
    virtual sal_Bool SAL_CALL setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const css::uno::Sequence< css::beans::PropertyValue >& aAttributeSet ) override;
    virtual sal_Bool SAL_CALL setText( const OUString& sText ) override;

    // XAccessibleTextAttributes
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getDefaultAttributes( const css::uno::Sequence< OUString >& RequestedAttributes ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getRunAttributes( ::sal_Int32 Index, const css::uno::Sequence< OUString >& RequestedAttributes ) override;

    // XAccessibleHypertext
    virtual ::sal_Int32 SAL_CALL getHyperLinkCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleHyperlink > SAL_CALL getHyperLink( ::sal_Int32 nLinkIndex ) override;
    virtual ::sal_Int32 SAL_CALL getHyperLinkIndex( ::sal_Int32 nCharIndex ) override;

    // XAccessibleMultiLineText
    virtual ::sal_Int32 SAL_CALL getLineNumberAtIndex( ::sal_Int32 nIndex ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextAtLineNumber( ::sal_Int32 nLineNo ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextAtLineWithCaret(  ) override;
    virtual ::sal_Int32 SAL_CALL getNumberOfLineWithCaret(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService (const OUString& sServiceName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;

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
    void FireEvent(const sal_Int16 nEventId, const css::uno::Any& rNewValue = css::uno::Any(), const css::uno::Any& rOldValue = css::uno::Any());

    /// Sets the given state on the internal state set and fires STATE_CHANGE event. Don't hold locks when calling this!
    void SetState( const sal_Int64 nStateId );
    /// Unsets the given state on the internal state set and fires STATE_CHANGE event. Don't hold locks when calling this!
    void UnSetState( const sal_Int64 nStateId );

    static tools::Rectangle LogicToPixel( const tools::Rectangle& rRect, const MapMode& rMapMode, SvxViewForwarder const & rForwarder );

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

    int getNotifierClientId() const { return mnNotifierClientId; }

    /// Do we have children? This is the case for image bullets
    bool HaveChildren();

    /// If there is a bullet, return it's text length, otherwise return 0
    sal_Int32 GetBulletTextLength() const;

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

    void _correctValues( css::uno::Sequence< css::beans::PropertyValue >& rValues );
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
    unotools::WeakReference<AccessibleImageBullet> maImageBullet;

    // the last string used for an Accessibility::TEXT_CHANGED event (guarded by solar mutex)
    OUString maLastTextString;

    // the offset of the underlying EditEngine from the shape/cell (guarded by solar mutex)
    Point maEEOffset;

    // the current state set (updated from SetState/UnSetState and guarded by solar mutex)
    sal_Int64 mnStateSet;

    /// The shape we're the accessible for (unguarded)
    css::uno::Reference< css::accessibility::XAccessible > mxParent;

    /// Our listeners (guarded by maMutex)
    int mnNotifierClientId;
private:
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
