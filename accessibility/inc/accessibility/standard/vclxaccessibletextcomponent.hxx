/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibletextcomponent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:28:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTCOMPONENT_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTCOMPONENT_HXX

#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif

#ifndef COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX
#include <comphelper/accessibletexthelper.hxx>
#endif


//  ----------------------------------------------------
//  class VCLXAccessibleTextComponent
//  ----------------------------------------------------

typedef ::cppu::ImplHelper1 <
    ::com::sun::star::accessibility::XAccessibleText >  VCLXAccessibleTextComponent_BASE;

class VCLXAccessibleTextComponent : public VCLXAccessibleComponent,
                                    public ::comphelper::OCommonAccessibleText,
                                    public VCLXAccessibleTextComponent_BASE
{
protected:
    ::rtl::OUString                         m_sText;

    void                                    SetText( const ::rtl::OUString& sText );

    virtual void                            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    // OCommonAccessibleText
    virtual ::rtl::OUString                 implGetText();
    virtual ::com::sun::star::lang::Locale  implGetLocale();
    virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex );

    // XComponent
    virtual void SAL_CALL                   disposing();

public:
    VCLXAccessibleTextComponent( VCLXWindow* pVCLXWindow );
    ~VCLXAccessibleTextComponent();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XAccessibleText
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
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
};


#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTCOMPONENT_HXX

