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
#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCPARA_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCPARA_HXX

#include "acccontext.hxx"
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/accessibility/XAccessibleTextMarkup.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextSelection.hpp>
#include <txmsrt.hxx>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include "accselectionhelper.hxx"
#include <calbck.hxx>
#include <unordered_map>

class SwField;
class SwTextFrame;
class SwTextNode;
class SwPaM;
class SwAccessiblePortionData;
class SwAccessibleHyperTextData;
class SwRangeRedline;
class SwXTextPortion;
class SwParaChangeTrackingInfo; //#i108125#

namespace com { namespace sun { namespace star {
    namespace i18n { struct Boundary; }
    namespace accessibility { class XAccessibleHyperlink; }
    namespace style { struct TabStop; }
} } }

typedef std::unordered_map< OUString,
                         css::beans::PropertyValue > tAccParaPropValMap;

class SwAccessibleParagraph :
        public SfxListener,
        public SwAccessibleContext,
        public css::accessibility::XAccessibleEditableText,
        public css::accessibility::XAccessibleSelection,
        public css::accessibility::XAccessibleHypertext,
        public css::accessibility::XAccessibleTextMarkup,
        public css::accessibility::XAccessibleMultiLineText,
        public css::accessibility::XAccessibleTextAttributes,
        public css::accessibility::XAccessibleTextSelection,
        public css::accessibility::XAccessibleExtendedAttributes
{
    friend class SwAccessibleHyperlink;

    OUString m_sDesc;  // protected by base classes mutex

    // data for this paragraph's text portions; this contains the
    // mapping from the core 'model string' to the accessible text
    // string.
    // pPortionData may be NULL; it should only be accessed through the
    // Get/Clear/Has/UpdatePortionData() methods
    std::unique_ptr<SwAccessiblePortionData> m_pPortionData;
    std::unique_ptr<SwAccessibleHyperTextData> m_pHyperTextData;

    sal_Int32 m_nOldCaretPos; // The 'old' caret pos. It's only valid as long
                            // as the cursor is inside this object (protected by
                            // mutex)

    bool m_bIsHeading;    // protected by base classes mutex
    sal_Int32 m_nHeadingLevel;

    // implementation for XAccessibleSelection
    SwAccessibleSelectionHelper m_aSelectionHelper;

    std::unique_ptr<SwParaChangeTrackingInfo> mpParaChangeTrackInfo; // #i108125#

    // XAccessibleComponent
    bool m_bLastHasSelection;

    /// get the (accessible) text string (requires frame; check before)
    OUString const & GetString();

    static OUString GetDescription();

    // get the current care position
    sal_Int32 GetCaretPos();

    // determine the current selection. Fill the values with
    // -1 if there is no selection in the this paragraph
    // @param pSelection (optional) check only Nth selection in ring
    bool GetSelectionAtIndex(sal_Int32 * pSelection, sal_Int32& nStart, sal_Int32& nEnd);
    bool GetSelection(sal_Int32& nStart, sal_Int32& nEnd) {
        return GetSelectionAtIndex(nullptr, nStart, nEnd);
    }

    // helper for GetSelection and getCaretPosition
    // #i27301# - add parameter <_bForSelection>, which indicates,
    // if the cursor is retrieved for selection or for caret position.
    SwPaM* GetCursor( const bool _bForSelection );

    // for cut/copy/paste: execute a particular slot at the view shell
    void ExecuteAtViewShell( sal_uInt16 nSlot );

    // helper method for get/setAttributes
    // (for the special case of (nEndIndex==-1) a single character will
    //  be selected)
    SwXTextPortion* CreateUnoPortion( sal_Int32 nStart, sal_Int32 nEnd );

    // methods for checking the parameter range:

    // does nPos point to a char?
    static bool IsValidChar(sal_Int32 nPos, sal_Int32 nLength);

    // does nPos point to a position? (may be behind the last character)
    static bool IsValidPosition(sal_Int32 nPos, sal_Int32 nLength);

    // is nBegin...nEnd a valid range? (nEnd points past the last character)
    static bool IsValidRange(sal_Int32 nBegin, sal_Int32 nEnd, sal_Int32 nLength);

    // Ensure ordered range (i.e. nBegin is smaller then nEnd)
    static void OrderRange(sal_Int32& nBegin, sal_Int32& nEnd)
    {
        if( nBegin > nEnd )
        {
            sal_Int32 nTmp = nBegin; nBegin = nEnd; nEnd = nTmp;
        }
    }

    const SwRangeRedline* GetRedlineAtIndex();
    OUString GetFieldTypeNameAtIndex(sal_Int32 nIndex);

    // #i63870#
    void _getDefaultAttributesImpl(
            const css::uno::Sequence< OUString >& aRequestedAttributes,
            tAccParaPropValMap& rDefAttrSeq,
            const bool bOnlyCharAttrs = false );
    void _getRunAttributesImpl(
            const sal_Int32 nIndex,
            const css::uno::Sequence< OUString >& aRequestedAttributes,
            tAccParaPropValMap& rRunAttrSeq );

    void _getSupplementalAttributesImpl(
            const css::uno::Sequence< OUString >& aRequestedAttributes,
            tAccParaPropValMap& rSupplementalAttrSeq );

    void _correctValues(
            const sal_Int32 nIndex,
            std::vector< css::beans::PropertyValue >& rValues );

public:
    bool IsHeading() const;

protected:

    // Set states for getAccessibleStateSet.
    // This derived class additionally sets MULTILINE(1), MULTISELECTABLE(+),
    // FOCUSABLE(+) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) override;

    virtual void InvalidateContent_( bool bVisibleDataFired ) override;

    virtual void InvalidateCursorPos_() override;
    virtual void InvalidateFocus_() override;

    virtual ~SwAccessibleParagraph() override;

    // handling of data for the text portions

    // force update of new portion data
    /// @throws css::uno::RuntimeException
    void UpdatePortionData();

    // remove the current portion data
    void ClearPortionData();

    // get portion data; update if necessary
    /// @throws css::uno::RuntimeException
    SwAccessiblePortionData& GetPortionData()
    {
        if( m_pPortionData == nullptr )
            UpdatePortionData();
        return *m_pPortionData;
    }

    //helpers for word boundaries

    bool GetCharBoundary( css::i18n::Boundary& rBound,
                              sal_Int32 nPos );
    bool GetWordBoundary( css::i18n::Boundary& rBound,
                              const OUString& rText,
                              sal_Int32 nPos );
    bool GetSentenceBoundary( css::i18n::Boundary& rBound,
                                  const OUString& rText,
                                  sal_Int32 nPos );
    bool GetLineBoundary( css::i18n::Boundary& rBound,
                              const OUString& rText,
                              sal_Int32 nPos );
    static bool GetParagraphBoundary( css::i18n::Boundary& rBound,
                                   const OUString& rText );
    bool GetAttributeBoundary( css::i18n::Boundary& rBound,
                                   sal_Int32 nPos );
    bool GetGlyphBoundary( css::i18n::Boundary& rBound,
                               const OUString& rText,
                               sal_Int32 nPos );

    // get boundaries of word/sentence/etc. for specified text type
    // Does all argument checking, and then delegates to helper methods above.
    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    bool GetTextBoundary( css::i18n::Boundary& rBound,
                              const OUString& rText,
                              sal_Int32 nPos,
                              sal_Int16 aTextType );

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

public:

    SwAccessibleParagraph( std::shared_ptr<SwAccessibleMap> const& pInitMap,
                           const SwTextFrame& rTextFrame );

    inline operator css::accessibility::XAccessibleText *();

    virtual bool HasCursor() override;   // required by map to remember that object

    css::uno::Sequence< css::style::TabStop > GetCurrentTabStop( sal_Int32 nIndex  );
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;

    // XAccessibleContext

    // Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    // Return the parents locale or throw exception if this object has no
    // parent yet/anymore.
    virtual css::lang::Locale SAL_CALL
        getLocale() override;

    // #i27138# - paragraphs are in relation CONTENT_FLOWS_FROM and/or CONTENT_FLOWS_TO
    virtual css::uno::Reference<
            css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet() override;

    // XAccessibleComponent

    virtual void SAL_CALL grabFocus() override;
    // #i71385#
    virtual sal_Int32 SAL_CALL getForeground() override;
    virtual sal_Int32 SAL_CALL getBackground() override;

    // XServiceInfo

    // Returns an identifier for the implementation of this object.
    virtual OUString SAL_CALL
        getImplementationName() override;

    // Return whether the specified service is supported by this class.
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName) override;

    // Returns a list of all supported services.  In this case that is just
    // the AccessibleContext service.
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    // XInterface

    // (XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAccessibleContext and
    // XAccessibleEditableText).

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType ) override;

    virtual void SAL_CALL acquire(  ) throw () override
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw () override
        { SwAccessibleContext::release(); };

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition() override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getCharacterCount(  ) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString SAL_CALL getSelectedText(  ) override;
    virtual sal_Int32 SAL_CALL getSelectionStart() override;
    virtual sal_Int32 SAL_CALL getSelectionEnd() override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString SAL_CALL getText(  ) override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;

    // XAccessibleEditableText
    virtual sal_Bool SAL_CALL cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL pasteText( sal_Int32 nIndex ) override;
    virtual sal_Bool SAL_CALL deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL insertText( const OUString& sText, sal_Int32 nIndex ) override;
    virtual sal_Bool SAL_CALL replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) override;
    virtual sal_Bool SAL_CALL setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const css::uno::Sequence< css::beans::PropertyValue >& aAttributeSet ) override;
    virtual sal_Bool SAL_CALL setText( const OUString& sText ) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex ) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex ) override;

    // index has to be treated as global child index.
    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nChildIndex ) override;

    // XAccessibleHypertext
    virtual sal_Int32 SAL_CALL getHyperLinkCount() override;
    virtual css::uno::Reference<
            css::accessibility::XAccessibleHyperlink >
        SAL_CALL getHyperLink( sal_Int32 nLinkIndex ) override;
    virtual sal_Int32 SAL_CALL getHyperLinkIndex( sal_Int32 nCharIndex ) override;

    // #i71360#
    // XAccessibleTextMarkup
    virtual sal_Int32 SAL_CALL getTextMarkupCount( sal_Int32 nTextMarkupType ) override;

    virtual css::accessibility::TextSegment SAL_CALL
            getTextMarkup( sal_Int32 nTextMarkupIndex,
                           sal_Int32 nTextMarkupType ) override;

    virtual css::uno::Sequence< css::accessibility::TextSegment > SAL_CALL
            getTextMarkupAtIndex( sal_Int32 nCharIndex,
                                  sal_Int32 nTextMarkupType ) override;

    // XAccessibleTextSelection
    virtual sal_Bool SAL_CALL scrollToPosition( const css::awt::Point& aPoint, sal_Bool isLeftTop ) override;
    virtual sal_Int32 SAL_CALL getSelectedPortionCount(  ) override;
    virtual sal_Int32 SAL_CALL getSeletedPositionStart( sal_Int32 nSelectedPortionIndex ) override;
    virtual sal_Int32 SAL_CALL getSeletedPositionEnd( sal_Int32 nSelectedPortionIndex ) override;
    virtual sal_Bool SAL_CALL removeSelection( sal_Int32 selectionIndex ) override;
    virtual sal_Int32 SAL_CALL  addSelection( sal_Int32 selectionIndex, sal_Int32 startOffset, sal_Int32 endOffset) override;
    // XAccessibleExtendedAttributes
    virtual css::uno::Any SAL_CALL getExtendedAttributes() override ;
    sal_Int32 GetRealHeadingLevel();

    // #i89175#
    // XAccessibleMultiLineText
    virtual sal_Int32 SAL_CALL getLineNumberAtIndex( sal_Int32 nIndex ) override;

    virtual css::accessibility::TextSegment SAL_CALL
            getTextAtLineNumber( sal_Int32 nLineNo ) override;

    virtual css::accessibility::TextSegment SAL_CALL
            getTextAtLineWithCaret() override;

    virtual sal_Int32 SAL_CALL getNumberOfLineWithCaret() override;

    // #i63870#
    // XAccessibleTextAttributes
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getDefaultAttributes( const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getRunAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
};

inline SwAccessibleParagraph::operator css::accessibility::XAccessibleText *()
{
    return static_cast< css::accessibility::XAccessibleEditableText * >( this );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
