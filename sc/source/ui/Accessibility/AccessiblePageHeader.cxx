/*************************************************************************
 *
 *  $RCSfile: AccessiblePageHeader.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2002-03-21 06:51:50 $
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

#include "AccessiblePageHeader.hxx"
#include "prevwsh.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"
#include "prevloc.hxx"

#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>

#include <vcl/window.hxx>
#include <svtools/smplhint.hxx>
#include <unotools/accessiblestatesethelper.hxx>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessiblePageHeader::ScAccessiblePageHeader( const ::com::sun::star::uno::Reference<
                                ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, sal_Bool bHeader, sal_Int32 nIndex ) :
    ScAccessibleContextBase( rxParent, AccessibleRole::TABLE ),
    mpViewShell( pViewShell ),
    mnIndex( nIndex ),
    mbHeader( bHeader )
{
    if (mpViewShell)
        mpViewShell->AddAccessibilityObject(*this);
}

ScAccessiblePageHeader::~ScAccessiblePageHeader()
{
    if (mpViewShell)
        mpViewShell->RemoveAccessibilityObject(*this);
}

void SAL_CALL ScAccessiblePageHeader::disposing()
{
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    ScAccessibleContextBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessiblePageHeader::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( SfxSimpleHint ))
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        ULONG nId = rRef.GetId();
        if ( nId == SFX_HINT_DYING )
            dispose();
    }
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeader::getAccessibleAt( const awt::Point& aPoint )
                                throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<XAccessible> xRet;

    //! ...

    return xRet;
}

void SAL_CALL ScAccessiblePageHeader::grabFocus() throw (uno::RuntimeException)
{
     ScUnoGuard aGuard;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
            xAccessibleComponent->grabFocus();
    }
}

//=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessiblePageHeader::getAccessibleChildCount() throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    sal_Int32 nRet = 0;

    //! ...

    return nRet;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeader::getAccessibleChild( sal_Int32 nIndex )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<XAccessible> xRet;

    //! ...

    if ( !xRet.is() )
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

sal_Int32 SAL_CALL ScAccessiblePageHeader::getAccessibleIndexInParent() throw (uno::RuntimeException)
{
    return mnIndex;
}

uno::Reference< XAccessibleRelationSet > SAL_CALL ScAccessiblePageHeader::getAccessibleRelationSet()
                                throw (uno::RuntimeException)
{
    //! missing
    return NULL;
}

uno::Reference< XAccessibleStateSet > SAL_CALL ScAccessiblePageHeader::getAccessibleStateSet()
                                throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    pStateSet->AddState(AccessibleStateType::ENABLED);
    pStateSet->AddState(AccessibleStateType::OPAQUE);
    if (isShowing())
        pStateSet->AddState(AccessibleStateType::SHOWING);
    if (isVisible())
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    return pStateSet;
}

//=====  XServiceInfo  ====================================================

rtl::OUString SAL_CALL ScAccessiblePageHeader::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScAccessiblePageHeader"));
}

uno::Sequence<rtl::OUString> SAL_CALL ScAccessiblePageHeader::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.text.AccessibleHeaderFooterView"));

    return aSequence;
}

//====  internal  =========================================================

::rtl::OUString SAL_CALL ScAccessiblePageHeader::createAccessibleDescription(void)
                    throw (uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( mbHeader ?
        "This is a header in a page preview of a Spreadsheet Document." :
        "This is a footer in a page preview of a Spreadsheet Document." );
}

::rtl::OUString SAL_CALL ScAccessiblePageHeader::createAccessibleName(void)
                    throw (uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( mbHeader ?
        "Spreadsheet Page Preview Header" : "Spreadsheet Page Preview Footer" );
}

Rectangle ScAccessiblePageHeader::GetBoundingBoxOnScreen() throw (uno::RuntimeException)
{
    Rectangle aCellRect(GetBoundingBox());
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative(NULL);
            aCellRect.setX(aCellRect.getX() + aRect.getX());
            aCellRect.setY(aCellRect.getY() + aRect.getY());
        }
    }
    return aCellRect;
}

Rectangle ScAccessiblePageHeader::GetBoundingBox() throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        if ( mbHeader )
            rData.GetHeaderPosition( aRect );
        else
            rData.GetFooterPosition( aRect );
    }
    return aRect;
}

sal_Bool ScAccessiblePageHeader::IsDefunc( const uno::Reference<XAccessibleStateSet>& rxParentStates )
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == NULL) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}


