/*************************************************************************
 *
 *  $RCSfile: AccessiblePreviewHeaderCell.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:06:09 $
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


#ifndef _SC_ACCESSIBLEPREVIEWHEADERCELL_HXX
#define _SC_ACCESSIBLEPREVIEWHEADERCELL_HXX

#ifndef _SC_ACCESSIBLECONTEXTBASE_HXX
#include "AccessibleContextBase.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEVALUE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleValue.hpp>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

class ScPreviewShell;
class ScPreviewTableInfo;
class accessibility::AccessibleTextHelper;

typedef cppu::ImplHelper1< ::drafts::com::sun::star::accessibility::XAccessibleValue>
                    ScAccessiblePreviewHeaderCellImpl;

class ScAccessiblePreviewHeaderCell :
        public ScAccessibleContextBase,
        public ScAccessiblePreviewHeaderCellImpl
{
public:
    ScAccessiblePreviewHeaderCell( const ::com::sun::star::uno::Reference<
                                ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell,
                            const ScAddress& rCellPos, sal_Bool bIsColHdr, sal_Bool bIsRowHdr,
                            sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewHeaderCell();

public:
     virtual void SAL_CALL disposing();

    //=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  XInterface  =====================================================

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw ();

    virtual void SAL_CALL release() throw ();

    //=====  XAccessibleValue  ================================================

    virtual ::com::sun::star::uno::Any SAL_CALL getCurrentValue() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setCurrentValue( const ::com::sun::star::uno::Any& aNumber )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getMaximumValue() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getMinimumValue() throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleAt( const ::com::sun::star::awt::Point& aPoint )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   grabFocus() throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleChild( sal_Int32 i )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
                            getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    ///=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual ::rtl::OUString SAL_CALL createAccessibleDescription(void) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL createAccessibleName(void) throw (::com::sun::star::uno::RuntimeException);

    virtual Rectangle GetBoundingBoxOnScreen(void) const throw(::com::sun::star::uno::RuntimeException);
    virtual Rectangle GetBoundingBox(void) const throw (::com::sun::star::uno::RuntimeException);

private:
    ScPreviewShell*     mpViewShell;
    accessibility::AccessibleTextHelper* mpTextHelper;
    sal_Int32           mnIndex;
    ScAddress           maCellPos;
    sal_Bool            mbColumnHeader;
    sal_Bool            mbRowHeader;
    mutable ScPreviewTableInfo* mpTableInfo;

    sal_Bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    void CreateTextHelper();
    void    FillTableInfo() const;
};


#endif
