/*************************************************************************
 *
 *  $RCSfile: AccessibleCell.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2002-02-19 08:26:13 $
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


#include "AccessibleCell.hxx"
#ifndef SC_TABVWSH_HXX
#include "tabvwsh.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif
#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef SC_MISCUNO_HXX
#include "miscuno.hxx"
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif

#include <float.h>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleCell::ScAccessibleCell (
        const uno::Reference<XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScAddress& rCellAddress,
        sal_Int32 nIndex,
        ScSplitPos eSplitPos)
    :
    ScAccessibleContextBase (rxParent, AccessibleRole::TABLE_CELL),
    mpViewShell(pViewShell),
    mpDoc(GetDocument(pViewShell)),
    maCellAddress(rCellAddress),
    mnIndex(nIndex),
    meSplitPos(eSplitPos)
{
}

ScAccessibleCell::~ScAccessibleCell ()
{
}

    //=====  XInterface  ======================================================

uno::Any SAL_CALL
    ScAccessibleCell::queryInterface(
    const uno::Type & rType )
    throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( XAccessibleValue )

    return ScAccessibleContextBase::queryInterface( rType );
}

/** Increase the reference count.
*/
void SAL_CALL
    ScAccessibleCell::acquire (void)
    throw ()
{
    OWeakObject::acquire ();
}

/** Decrease the reference count.
*/
void SAL_CALL
    ScAccessibleCell::release (void)
    throw ()
{
    OWeakObject::release ();
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleCell::getAccessibleAt(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    uno::Reference< XAccessible > xAccessible = NULL;
    // should be implemented in the Accessible Text helper
    return xAccessible;
}

sal_Bool SAL_CALL ScAccessibleCell::isVisible(  )
        throw (uno::RuntimeException)
{
     ::osl::MutexGuard aGuard (maMutex);
    // test whether the cell is hidden (column/row - hidden/filtered)
    sal_Bool bVisible(sal_True);
    if (mpDoc)
    {
        BYTE nColFlags = mpDoc->GetColFlags(maCellAddress.Col(), maCellAddress.Tab());
        BYTE nRowFlags = mpDoc->GetRowFlags(maCellAddress.Row(), maCellAddress.Tab());
        if (((nColFlags & CR_HIDDEN) == CR_HIDDEN) || ((nColFlags & CR_FILTERED) == CR_FILTERED) ||
            ((nRowFlags & CR_HIDDEN) == CR_HIDDEN) || ((nRowFlags & CR_FILTERED) == CR_FILTERED))
            bVisible = sal_False;
    }
    return bVisible;
}

void SAL_CALL ScAccessibleCell::grabFocus(  )
        throw (uno::RuntimeException)
{
     ::osl::MutexGuard aGuard (maMutex);
    if (getAccessibleParent().is() && mpViewShell)
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
        {
            xAccessibleComponent->grabFocus();
            mpViewShell->SetCursor(maCellAddress.Col(), maCellAddress.Row());
        }
    }
}

Rectangle ScAccessibleCell::GetBoundingBoxOnScreen(void)
        throw (uno::RuntimeException)
{
    Rectangle aCellRect(GetBoundingBox());
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative(NULL);
            aCellRect.setX(aCellRect.getX() + aRect.getX());
            aCellRect.setY(aCellRect.getY() + aRect.getY());
        }
    }
    return aCellRect;
}

Rectangle ScAccessibleCell::GetBoundingBox(void)
        throw (uno::RuntimeException)
{
    Rectangle aCellRect;
    if (mpViewShell && mpViewShell->GetViewData())
    {
        sal_Int32 nSizeX, nSizeY;
        mpViewShell->GetViewData()->GetMergeSizePixel(
            maCellAddress.Col(), maCellAddress.Row(), nSizeX, nSizeY);
        aCellRect.SetSize(Size(nSizeX, nSizeY));
        aCellRect.SetPos(mpViewShell->GetViewData()->GetScrPos(maCellAddress.Col(), maCellAddress.Row(), meSplitPos));
    }
    return aCellRect;
}

    //=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessibleCell::getAccessibleChildCount (void)
                    throw (uno::RuntimeException)
{
    sal_Int32 nCount(0);
    // should call the Helper class of Thorsten Behrens to get the child count
    return nCount;
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleCell::getAccessibleChild (sal_Int32 nIndex)
        throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException)
{
    DBG_ERROR("not implemented yet");
    return uno::Reference< XAccessible >();
}

sal_Int32
    ScAccessibleCell::getAccessibleIndexInParent (void)
        throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    return mnIndex;
}

::rtl::OUString SAL_CALL
    ScAccessibleCell::createAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    rtl::OUString sDescription;

    if (mpDoc)
    {
        ScPostIt aNote;
        mpDoc->GetNote(maCellAddress.Col(), maCellAddress.Row(), maCellAddress.Tab(), aNote);
        sDescription = rtl::OUString (aNote.GetText());
    }

    if (!sDescription.getLength())
        sDescription = getAccessibleName();

    return sDescription;
}

::rtl::OUString SAL_CALL
    ScAccessibleCell::createAccessibleName (void)
    throw (uno::RuntimeException)
{
    rtl::OUString sName;
    String sAddress;
    // Document not needed, because only the cell address, but not the tablename is needed
    maCellAddress.Format( sAddress, SCA_VALID, NULL );
    sName = rtl::OUString(sAddress);
    return sName;
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleCell::getAccessibleStateSet (void)
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    if (IsEditable(xParentStates))
    {
        pStateSet->AddState(AccessibleStateType::EDITABLE);
        pStateSet->AddState(AccessibleStateType::RESIZABLE);
    }
    pStateSet->AddState(AccessibleStateType::ENABLED);
    pStateSet->AddState(AccessibleStateType::MULTILINE);
    pStateSet->AddState(AccessibleStateType::MULTISELECTABLE);
    if (IsOpaque(xParentStates))
        pStateSet->AddState(AccessibleStateType::OPAQUE);
    pStateSet->AddState(AccessibleStateType::SELECTABLE);
    if (IsSelected(xParentStates))
        pStateSet->AddState(AccessibleStateType::SELECTED);
    if (IsShowing(xParentStates))
        pStateSet->AddState(AccessibleStateType::SHOWING);
    pStateSet->AddState(AccessibleStateType::TRANSIENT);
    if (IsVisible(xParentStates))
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    return pStateSet;
}

    //=====  XAccessibleValue  ================================================

uno::Any SAL_CALL
    ScAccessibleCell::getCurrentValue(  )
    throw (uno::RuntimeException)
{
     ::osl::MutexGuard aGuard (maMutex);
    uno::Any aAny;
    if (mpDoc)
        aAny <<= mpDoc->GetValue(maCellAddress);

    return aAny;
}

sal_Bool SAL_CALL
    ScAccessibleCell::setCurrentValue( const uno::Any& aNumber )
    throw (uno::RuntimeException)
{
     ::osl::MutexGuard aGuard (maMutex);
    double fValue;
    sal_Bool bResult(sal_False);
    if((aNumber >>= fValue) && mpDoc)
    {
        mpDoc->SetValue(maCellAddress.Col(), maCellAddress.Row(), maCellAddress.Tab(), fValue);
        bResult = sal_True;
    }
    return bResult;
}

uno::Any SAL_CALL
    ScAccessibleCell::getMaximumValue(  )
    throw (uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= DBL_MAX;

    return aAny;
}

uno::Any SAL_CALL
    ScAccessibleCell::getMinimumValue(  )
    throw (uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= DBL_MIN;

    return aAny;
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleCell::getImplementationName (void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleCell"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL
    ScAccessibleCell::getSupportedServiceNames (void)
        throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.AccessibleCell"));

    return aSequence;
}

    //=====  XTypeProvider  ===================================================

uno::Sequence< uno::Type> SAL_CALL ScAccessibleCell::getTypes (void)
        throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    uno::Sequence< uno::Type>
        aTypeSequence = ScAccessibleContextBase::getTypes();
    sal_Int32 nOldSize(aTypeSequence.getLength());
    aTypeSequence.realloc(nOldSize + 1);
    uno::Type* pTypes = aTypeSequence.getArray();

    pTypes[nOldSize] = ::getCppuType((const uno::Reference<
            XAccessibleValue>*)0);

    return aTypeSequence;
}

    //====  internal  =========================================================

sal_Bool ScAccessibleCell::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (mpDoc == NULL) || (mpViewShell == NULL) || !getAccessibleParent().is() ||
         (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

sal_Bool ScAccessibleCell::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    sal_Bool bEditable(sal_True);
    if (rxParentStates.is() && !rxParentStates->contains(AccessibleStateType::EDITABLE) &&
        mpDoc)
    {
        // here I have to test whether the protection of the table should influence this cell.
        const ScProtectionAttr* pItem = (const ScProtectionAttr*)mpDoc->GetAttr(
            maCellAddress.Col(), maCellAddress.Row(),
            maCellAddress.Tab(), ATTR_PROTECTION);
        if (pItem)
            bEditable = !pItem->GetProtection();
    }
    return bEditable;
}

sal_Bool ScAccessibleCell::IsOpaque(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    // test whether there is a background color
    sal_Bool bOpaque(sal_True);
    if (mpDoc)
    {
        const SvxBrushItem* pItem = (const SvxBrushItem*)mpDoc->GetAttr(
            maCellAddress.Col(), maCellAddress.Row(),
            maCellAddress.Tab(), ATTR_PROTECTION);
        if (pItem)
            bOpaque = pItem->GetColor() != COL_TRANSPARENT;
    }
    return bOpaque;
}

sal_Bool ScAccessibleCell::IsSelected(const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return sal_False;
}

sal_Bool ScAccessibleCell::IsShowing(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::SHOWING));
}

sal_Bool ScAccessibleCell::IsVisible(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::VISIBLE));
}

ScDocument* ScAccessibleCell::GetDocument(ScTabViewShell* pViewShell)
{
    ScDocument* pDoc = NULL;
    if (pViewShell && pViewShell->GetViewData())
        pDoc = pViewShell->GetViewData()->GetDocument();
    return pDoc;
}

