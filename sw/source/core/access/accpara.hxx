/*************************************************************************
 *
 *  $RCSfile: accpara.hxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:12:50 $
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
#ifndef _ACCPARA_HXX
#define _ACCPARA_HXX

#ifndef _ACCCONTEXT_HXX
#include "acccontext.hxx"
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEDITABLETEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _ACCSELECTIONHELPER_HXX_
#include "accselectionhelper.hxx"
#endif

class SwTxtFrm;
class SwTxtNode;
class SwPaM;
class SwCrsrShell;
class SwAccessiblePortionData;
class SwXTextPortion;
namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace i18n { struct Boundary; }
} } }

class SwAccessibleParagraph :
        public SwAccessibleContext,
        public ::com::sun::star::accessibility::XAccessibleEditableText,
        public com::sun::star::accessibility::XAccessibleSelection
{
    ::rtl::OUString sDesc;  // protected by base classes mutex

    /// data for this paragraph's text portions; this contains the
    /// mapping from the core 'model string' to the accessible text
    /// string.
    /// pPortionData may be NULL; it should only be accessed through the
    /// Get/Clear/Has/UpdatePortionData() methods
    SwAccessiblePortionData* pPortionData;

    sal_Int32 nOldCaretPos; // The 'old' caret pos. It's only valid as long
                            // as the cursor is inside this object (protected by
                            // mutex)

    sal_Bool bIsHeading;    // protected by base classes mutex

    // implementation for XAccessibleSelection
    SwAccessibleSelectionHelper aSelectionHelper;


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
    SwPaM* GetCrsr();          /// helper for GetSelection and getCaretPosition

    /// for cut/copy/paste: execute a particular slot at the view shell
    void ExecuteAtViewShell( UINT16 nSlot );

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

    /// generate an empty boundary
    sal_Bool GetEmptyBoundary( com::sun::star::i18n::Boundary& rBound );

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

public:

    SwAccessibleParagraph( SwAccessibleMap *pMap, sal_Int32 nPara,
                           const SwTxtFrm *pTxtFrm );

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


    //=====  XAccessibleComponent  ============================================

    virtual void SAL_CALL grabFocus()
        throw (::com::sun::star::uno::RuntimeException);

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
    virtual ::rtl::OUString SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
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
    virtual void SAL_CALL selectAllAccessible(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

};


#endif

