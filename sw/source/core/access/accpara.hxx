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

#include <acccontext.hxx>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/accessibility/XAccessibleTextMarkup.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextSelection.hpp>
#include <txmsrt.hxx>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <boost/unordered_map.hpp>
#include <accselectionhelper.hxx>
#include <calbck.hxx>

class SwField;
class SwTxtFrm;
class SwTxtNode;
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

typedef ::boost::unordered_map< OUString,
                         ::com::sun::star::beans::PropertyValue,
                         OUStringHash,
                         ::std::equal_to< OUString > > tAccParaPropValMap;

class SwAccessibleParagraph :
        public SwClient, // #i108125#
        public SwAccessibleContext,
        public ::com::sun::star::accessibility::XAccessibleEditableText,
        public com::sun::star::accessibility::XAccessibleSelection,
        public com::sun::star::accessibility::XAccessibleHypertext,
        public com::sun::star::accessibility::XAccessibleTextMarkup,
        public com::sun::star::accessibility::XAccessibleMultiLineText,
        public ::com::sun::star::accessibility::XAccessibleTextAttributes,
        public com::sun::star::accessibility::XAccessibleTextSelection,
        public  com::sun::star::accessibility::XAccessibleExtendedAttributes
{
    friend class SwAccessibleHyperlink;

    OUString sDesc;  // protected by base classes mutex

    // data for this paragraph's text portions; this contains the
    // mapping from the core 'model string' to the accessible text
    // string.
    // pPortionData may be NULL; it should only be accessed through the
    // Get/Clear/Has/UpdatePortionData() methods
    SwAccessiblePortionData* pPortionData;
    SwAccessibleHyperTextData *pHyperTextData;

    sal_Int32 nOldCaretPos; // The 'old' caret pos. It's only valid as long
                            // as the cursor is inside this object (protected by
                            // mutex)

    sal_Bool bIsHeading;    // protected by base classes mutex
    sal_Int32 nHeadingLevel;

    // implementation for XAccessibleSelection
    SwAccessibleSelectionHelper aSelectionHelper;

    SwParaChangeTrackingInfo* mpParaChangeTrackInfo; // #i108125#

    /// get the SwTxtNode (requires frame; check before)
    const SwTxtNode* GetTxtNode() const;

    /// get the (accessible) text string (requires frame; check before)
    OUString GetString();

    OUString GetDescription();

    // get the current care position
    sal_Int32 GetCaretPos();

    // determine the current selection. Fill the values with
    // -1 if there is no selection in the this paragraph
    sal_Bool GetSelection(sal_Int32& nStart, sal_Int32& nEnd);

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
    sal_Bool IsValidChar(sal_Int32 nPos, sal_Int32 nLength);

    // does nPos point to a position? (may be behind the last character)
    sal_Bool IsValidPosition(sal_Int32 nPos, sal_Int32 nLength);

    // is nBegin...nEnd a valid range? (nEnd points past the last character)
    sal_Bool IsValidRange(sal_Int32 nBegin, sal_Int32 nEnd, sal_Int32 nLength);

    // Ensure ordered range (i.e. nBegin is smaller then nEnd)
    void OrderRange(sal_Int32& nBegin, sal_Int32& nEnd)
    {
        if( nBegin > nEnd )
        {
            sal_Int32 nTmp = nBegin; nBegin = nEnd; nEnd = nTmp;
        }
    }

    const SwRangeRedline* GetRedlineAtIndex( sal_Int32 nPos );
    OUString GetFieldTypeNameAtIndex(sal_Int32 nIndex);

    // #i63870#
    void _getDefaultAttributesImpl(
            const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes,
            tAccParaPropValMap& rDefAttrSeq,
            const bool bOnlyCharAttrs = false );
    void _getRunAttributesImpl(
            const sal_Int32 nIndex,
            const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes,
            tAccParaPropValMap& rRunAttrSeq );

    void _getSupplementalAttributesImpl(
            const sal_Int32 nIndex,
            const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes,
            tAccParaPropValMap& rSupplementalAttrSeq );

    void _correctValues(
            const sal_Int32 nIndex,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rValues );

public:
    SwTOXSortTabBase* GetTOXSortTabBase();

    sal_Bool IsHeading() const;

protected:

    // Set states for getAccessibleStateSet.
    // This drived class additinaly sets MULTILINE(1), MULTISELECTABLE(+),
    // FOCUSABLE(+) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) SAL_OVERRIDE;

    virtual void _InvalidateContent( sal_Bool bVisibleDataFired ) SAL_OVERRIDE;

    virtual void _InvalidateCursorPos() SAL_OVERRIDE;
    virtual void _InvalidateFocus() SAL_OVERRIDE;

    virtual ~SwAccessibleParagraph();

    // handling of data for the text portions

    // force update of new portion data
    void UpdatePortionData()
        throw( com::sun::star::uno::RuntimeException );

    // remove the current portion data
    void ClearPortionData();

    // get portion data; update if necessary
    SwAccessiblePortionData& GetPortionData()
        throw( com::sun::star::uno::RuntimeException )
    {
        if( pPortionData == NULL )
            UpdatePortionData();
        return *pPortionData;
    }

    // determine if portion data is currently available
    sal_Bool HasPortionData()   { return (pPortionData != NULL); }

    //helpers for word boundaries

    sal_Bool GetCharBoundary( com::sun::star::i18n::Boundary& rBound,
                              const OUString& rText,
                              sal_Int32 nPos );
    sal_Bool GetWordBoundary( com::sun::star::i18n::Boundary& rBound,
                              const OUString& rText,
                              sal_Int32 nPos );
    sal_Bool GetSentenceBoundary( com::sun::star::i18n::Boundary& rBound,
                                  const OUString& rText,
                                  sal_Int32 nPos );
    sal_Bool GetLineBoundary( com::sun::star::i18n::Boundary& rBound,
                              const OUString& rText,
                              sal_Int32 nPos );
    sal_Bool GetParagraphBoundary( com::sun::star::i18n::Boundary& rBound,
                                   const OUString& rText,
                                   sal_Int32 nPos );
    sal_Bool GetAttributeBoundary( com::sun::star::i18n::Boundary& rBound,
                                   const OUString& rText,
                                   sal_Int32 nPos );
    sal_Bool GetGlyphBoundary( com::sun::star::i18n::Boundary& rBound,
                               const OUString& rText,
                               sal_Int32 nPos );

    // get boundaries of word/sentence/etc. for specified text type
    // Does all argument checking, and then delegates to helper methods above.
    sal_Bool GetTextBoundary( com::sun::star::i18n::Boundary& rBound,
                              const OUString& rText,
                              sal_Int32 nPos,
                              sal_Int16 aTextType )
        throw (
            ::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew) SAL_OVERRIDE;

public:

    SwAccessibleParagraph( SwAccessibleMap& rInitMap,
                           const SwTxtFrm& rTxtFrm );

    inline operator ::com::sun::star::accessibility::XAccessibleText *();

    virtual sal_Bool HasCursor() SAL_OVERRIDE;   // required by map to remember that object

    com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop > GetCurrentTabStop( sal_Int32 nIndex  );
    virtual sal_Int16 SAL_CALL getAccessibleRole (void)     throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleContext

    // Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return the parents locale or throw exception if this object has no
    // parent yet/anymore.
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale (void)
        throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // #i27138# - paragraphs are in relation CONTENT_FLOWS_FROM and/or CONTENT_FLOWS_TO
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleComponent

    virtual void SAL_CALL grabFocus()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // #i71385#
    virtual sal_Int32 SAL_CALL getForeground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo

    // Returns an identifier for the implementation of this object.
    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return whether the specified service is supported by this class.
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Returns a list of all supported services.  In this case that is just
    // the AccessibleContext service.
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XInterface

    // (XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAcessibleContext and
    // XAccessibleEditableText).

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL acquire(  ) throw () SAL_OVERRIDE
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw () SAL_OVERRIDE
        { SwAccessibleContext::release(); };

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSelectedText(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSelectionStart()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSelectionEnd()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleEditableText
    virtual sal_Bool SAL_CALL cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL pasteText( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL insertText( const OUString& sText, sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aAttributeSet ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL setText( const OUString& sText ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL clearAccessibleSelection(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL selectAllAccessibleChildren(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // index has to be treated as global child index.
    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XAccessibleHypertext
    virtual sal_Int32 SAL_CALL getHyperLinkCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleHyperlink >
        SAL_CALL getHyperLink( sal_Int32 nLinkIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getHyperLinkIndex( sal_Int32 nCharIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // #i71360#
    // XAccesibleTextMarkup
    virtual sal_Int32 SAL_CALL getTextMarkupCount( sal_Int32 nTextMarkupType )
            throw (::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL
            getTextMarkup( sal_Int32 nTextMarkupIndex,
                           sal_Int32 nTextMarkupType )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                   ::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::accessibility::TextSegment > SAL_CALL
            getTextMarkupAtIndex( sal_Int32 nCharIndex,
                                  sal_Int32 nTextMarkupType )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                   ::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleTextSelection
    virtual sal_Bool SAL_CALL scrollToPosition( const ::com::sun::star::awt::Point& aPoint, sal_Bool isLeftTop )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSelectedPortionCount(  )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSeletedPositionStart( sal_Int32 nSelectedPortionIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSeletedPositionEnd( sal_Int32 nSelectedPortionIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL removeSelection( sal_Int32 selectionIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL  addSelection( sal_Int32 selectionIndex, sal_Int32 startOffset, sal_Int32 endOffset)
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    // XAccessibleExtendedAttributes
    virtual ::com::sun::star::uno::Any SAL_CALL getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
    sal_Bool GetSelectionAtIndex(sal_Int32& nIndex, sal_Int32& nStart, sal_Int32& nEnd);
    sal_Int32 GetRealHeadingLevel();
    // XAccessibleComponent
    sal_Bool m_bLastHasSelection;

    // #i89175#
    // XAccessibleMultiLineText
    virtual sal_Int32 SAL_CALL getLineNumberAtIndex( sal_Int32 nIndex )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                   ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL
            getTextAtLineNumber( sal_Int32 nLineNo )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                   ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL
            getTextAtLineWithCaret()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getNumberOfLineWithCaret()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // #i63870#
    // XAccesibleTextAttributes
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getDefaultAttributes( const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getRunAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

inline SwAccessibleParagraph::operator ::com::sun::star::accessibility::XAccessibleText *()
{
    return static_cast<
        ::com::sun::star::accessibility::XAccessibleEditableText * >( this );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
