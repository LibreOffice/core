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
#ifndef _ACCPARA_HXX
#define _ACCPARA_HXX

#include <acccontext.hxx>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/accessibility/XAccessibleTextMarkup.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <hash_map>
#include <accselectionhelper.hxx>
// --> OD 2010-02-19 #i108125#
#include <calbck.hxx>
// <--

class SwTxtFrm;
class SwTxtNode;
class SwPaM;
class SwAccessiblePortionData;
class SwAccessibleHyperTextData;
class SwXTextPortion;
// --> OD 2010-02-19 #i108125#
class SwParaChangeTrackingInfo;
// <--

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace i18n { struct Boundary; }
    namespace accessibility { class XAccessibleHyperlink; }
} } }

typedef ::std::hash_map< ::rtl::OUString,
                         ::com::sun::star::beans::PropertyValue,
                         ::rtl::OUStringHash,
                         ::std::equal_to< ::rtl::OUString > > tAccParaPropValMap;

class SwAccessibleParagraph :
        // --> OD 2010-02-19 #i108125#
        public SwClient,
        // <--
        public SwAccessibleContext,
        public ::com::sun::star::accessibility::XAccessibleEditableText,
        public com::sun::star::accessibility::XAccessibleSelection,
        public com::sun::star::accessibility::XAccessibleHypertext,
        public com::sun::star::accessibility::XAccessibleTextMarkup,
        public com::sun::star::accessibility::XAccessibleMultiLineText,
        public ::com::sun::star::accessibility::XAccessibleTextAttributes
{
    friend class SwAccessibleHyperlink;

    ::rtl::OUString sDesc;  // protected by base classes mutex

    /// data for this paragraph's text portions; this contains the
    /// mapping from the core 'model string' to the accessible text
    /// string.
    /// pPortionData may be NULL; it should only be accessed through the
    /// Get/Clear/Has/UpdatePortionData() methods
    SwAccessiblePortionData* pPortionData;
    SwAccessibleHyperTextData *pHyperTextData;

    sal_Int32 nOldCaretPos; // The 'old' caret pos. It's only valid as long
                            // as the cursor is inside this object (protected by
                            // mutex)

    sal_Bool bIsHeading;    // protected by base classes mutex

    // implementation for XAccessibleSelection
    SwAccessibleSelectionHelper aSelectionHelper;

    // --> OD 2010-02-19 #i108125#
    SwParaChangeTrackingInfo* mpParaChangeTrackInfo;
    // <--

    /// get the SwTxtNode (requires frame; check before)
    const SwTxtNode* GetTxtNode() const;

    /// get the (accessible) text string (requires frame; check before)
    ::rtl::OUString GetString();

    ::rtl::OUString GetDescription();

    // get the current care position
    sal_Int32 GetCaretPos();

    /// determine the current selection. Fill the values with
    /// -1 if there is no selection in the this paragraph
    sal_Bool GetSelection(sal_Int32& nStart, sal_Int32& nEnd);

    // helper for GetSelection and getCaretPosition
    // --> OD 2005-12-20 #i27301#
    // - add parameter <_bForSelection>, which indicates, if the cursor is
    //   retrieved for selection or for caret position.
    SwPaM* GetCursor( const bool _bForSelection );

    /// for cut/copy/paste: execute a particular slot at the view shell
    void ExecuteAtViewShell( sal_uInt16 nSlot );

    /// helper method for get/setAttributes
    /// (for the special case of (nEndIndex==-1) a single character will
    ///  be selected)
    SwXTextPortion* CreateUnoPortion( sal_Int32 nStart, sal_Int32 nEnd );


    // methods for checking the parameter range:

    /// does nPos point to a char?
    sal_Bool IsValidChar(sal_Int32 nPos, sal_Int32 nLength);

    /// does nPos point to a position? (may be behind the last character)
    sal_Bool IsValidPosition(sal_Int32 nPos, sal_Int32 nLength);

    /// is nBegin...nEnd a valid range? (nEnd points past the last character)
    sal_Bool IsValidRange(sal_Int32 nBegin, sal_Int32 nEnd, sal_Int32 nLength);

    /// Ensure ordered range (i.e. nBegin is smaller then nEnd)
    inline void OrderRange(sal_Int32& nBegin, sal_Int32& nEnd)
    {
        if( nBegin > nEnd )
        {
            sal_Int32 nTmp = nBegin; nBegin = nEnd; nEnd = nTmp;
        }
    }

    // --> OD 2006-07-13 #i63870#
    void _getDefaultAttributesImpl(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes,
            tAccParaPropValMap& rDefAttrSeq,
            const bool bOnlyCharAttrs = false );
    void _getRunAttributesImpl(
            const sal_Int32 nIndex,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes,
            tAccParaPropValMap& rRunAttrSeq );
    // <--

public:

    sal_Bool IsHeading() const;

protected:

    // Set states for getAccessibleStateSet.
    // This drived class additinaly sets MULTILINE(1), MULTISELECTABLE(+),
    // FOCUSABLE(+) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );

    virtual void _InvalidateContent( sal_Bool bVisibleDataFired );

    virtual void _InvalidateCursorPos();
    virtual void _InvalidateFocus();

    virtual ~SwAccessibleParagraph();

    //=====  handling of data for the text portions ===========================

    /// force update of new portion data
    void UpdatePortionData()
        throw( com::sun::star::uno::RuntimeException );

    /// remove the current portion data
    void ClearPortionData();

    /// get portion data; update if necesary
    SwAccessiblePortionData& GetPortionData()
        throw( com::sun::star::uno::RuntimeException )
    {
        if( pPortionData == NULL )
            UpdatePortionData();
        return *pPortionData;
    }

    /// determine if portion data is currently available
    sal_Bool HasPortionData()   { return (pPortionData != NULL); }


    //=====  helpers for word boundaries  ====================================

    sal_Bool GetCharBoundary( com::sun::star::i18n::Boundary& rBound,
                              const rtl::OUString& rText,
                              sal_Int32 nPos );
    sal_Bool GetWordBoundary( com::sun::star::i18n::Boundary& rBound,
                              const rtl::OUString& rText,
                              sal_Int32 nPos );
    sal_Bool GetSentenceBoundary( com::sun::star::i18n::Boundary& rBound,
                                  const rtl::OUString& rText,
                                  sal_Int32 nPos );
    sal_Bool GetLineBoundary( com::sun::star::i18n::Boundary& rBound,
                              const rtl::OUString& rText,
                              sal_Int32 nPos );
    sal_Bool GetParagraphBoundary( com::sun::star::i18n::Boundary& rBound,
                                   const rtl::OUString& rText,
                                   sal_Int32 nPos );
    sal_Bool GetAttributeBoundary( com::sun::star::i18n::Boundary& rBound,
                                   const rtl::OUString& rText,
                                   sal_Int32 nPos );
    sal_Bool GetGlyphBoundary( com::sun::star::i18n::Boundary& rBound,
                               const rtl::OUString& rText,
                               sal_Int32 nPos );

    /// get boundaries of word/sentence/etc. for specified text type
    /// Does all argument checking, and then delegates to helper methods above.
    sal_Bool GetTextBoundary( com::sun::star::i18n::Boundary& rBound,
                              const rtl::OUString& rText,
                              sal_Int32 nPos,
                              sal_Int16 aTextType )
        throw (
            ::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew);

public:

    SwAccessibleParagraph( SwAccessibleMap& rInitMap,
                           const SwTxtFrm& rTxtFrm );

    inline operator ::com::sun::star::accessibility::XAccessibleText *();

    virtual sal_Bool HasCursor();   // required by map to remember that object

    //=====  XAccessibleContext  ==============================================

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException);

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale (void)
        throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    /** paragraphs are in relation CONTENT_FLOWS_FROM and/or CONTENT_FLOWS_TO

        OD 2005-12-02 #i27138#

        @author OD
    */
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ============================================

    virtual void SAL_CALL grabFocus()
        throw (::com::sun::star::uno::RuntimeException);
    // --> OD 2007-01-17 #i71385#
    virtual sal_Int32 SAL_CALL getForeground()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException);
    // <--

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XInterface  ======================================================

    // (XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAcessibleContext and
    // XAccessibleEditableText).

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire(  ) throw ()
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw ()
        { SwAccessibleContext::release(); };

    //====== XTypeProvider ====================================================
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //=====  XAccesibleText  ==================================================
    virtual sal_Int32 SAL_CALL getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    //=====  XAccesibleEditableText  ==========================================
    virtual sal_Bool SAL_CALL cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL pasteText( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL insertText( const ::rtl::OUString& sText, sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::rtl::OUString& sReplacement ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aAttributeSet ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setText( const ::rtl::OUString& sText ) throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleSelection  ============================================
    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL clearAccessibleSelection(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL selectAllAccessibleChildren(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);

    // --> OD 2004-11-16 #111714# - index has to be treated as global child index.
    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    //=====  XAccessibleHypertext  ============================================
    virtual sal_Int32 SAL_CALL getHyperLinkCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleHyperlink >
        SAL_CALL getHyperLink( sal_Int32 nLinkIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getHyperLinkIndex( sal_Int32 nCharIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);

    // --> OD 2008-05-19 #i71360#
    //=====  XAccesibleTextMarkup  ============================================
    virtual sal_Int32 SAL_CALL getTextMarkupCount( sal_Int32 nTextMarkupType )
            throw (::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL
            getTextMarkup( sal_Int32 nTextMarkupIndex,
                           sal_Int32 nTextMarkupType )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                   ::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::accessibility::TextSegment > SAL_CALL
            getTextMarkupAtIndex( sal_Int32 nCharIndex,
                                  sal_Int32 nTextMarkupType )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                   ::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::uno::RuntimeException);
    // <--

    // --> OD 2008-05-29 #i89175#
    //=====  XAccessibleMultiLineText  ========================================
    virtual sal_Int32 SAL_CALL getLineNumberAtIndex( sal_Int32 nIndex )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                   ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL
            getTextAtLineNumber( sal_Int32 nLineNo )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                   ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL
            getTextAtLineWithCaret()
            throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getNumberOfLineWithCaret()
            throw (::com::sun::star::uno::RuntimeException);
    // <--

    // --> OD 2006-07-11 #i63870#
    //=====  XAccesibleTextAttributes  ========================================
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getDefaultAttributes( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getRunAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    // <--
};

inline SwAccessibleParagraph::operator ::com::sun::star::accessibility::XAccessibleText *()
{
    return static_cast<
        ::com::sun::star::accessibility::XAccessibleEditableText * >( this );
}

#endif

