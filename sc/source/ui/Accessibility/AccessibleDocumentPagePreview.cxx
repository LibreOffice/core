/*************************************************************************
 *
 *  $RCSfile: AccessibleDocumentPagePreview.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2002-02-20 13:49:21 $
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


#ifndef _SC_ACCESSIBLEDOCUMENTPAGEPREVIEW_HXX
#include "AccessibleDocumentPagePreview.hxx"
#endif
#ifndef SC_ACCESSIBILITYHINTS_HXX
#include "AccessibilityHints.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

    //=====  internal  ========================================================

ScAccessibleDocumentPagePreview::ScAccessibleDocumentPagePreview(
        const uno::Reference<XAccessible>& rxParent)
    : ScAccessibleDocumentBase(rxParent)
{
}

ScAccessibleDocumentPagePreview::~ScAccessibleDocumentPagePreview(void)
{
}

void ScAccessibleDocumentPagePreview::SetDefunc()
{
    ScAccessibleDocumentBase::SetDefunc();
}

    //=====  SfxListener  =====================================================

void ScAccessibleDocumentPagePreview::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    DBG_ERROR("not implemented");
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleAt(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    uno::Reference<XAccessible> xAccessible = NULL;
    DBG_ERROR("not implemented");
    return xAccessible;
}

void SAL_CALL ScAccessibleDocumentPagePreview::grabFocus(  )
        throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    DBG_ERROR("not implemented");
}

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
long SAL_CALL
    ScAccessibleDocumentPagePreview::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    DBG_ERROR("not implemented");
    return 0;
}

    /// Return the specified child or NULL if index is invalid.
uno::Reference<XAccessible> SAL_CALL
    ScAccessibleDocumentPagePreview::getAccessibleChild (long nIndex)
    throw (uno::RuntimeException/*,
        lang::IndexOutOfBoundsException*/)
{
    ::osl::MutexGuard aGuard (maMutex);
    uno::Reference<XAccessible> xAccessible;// = GetChild(nIndex);
    DBG_ERROR("not implemented");
    return xAccessible;
}

    /// Return the set of current states.
uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleDocumentPagePreview::getAccessibleStateSet (void)
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    DBG_ERROR("not implemented");
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

::rtl::OUString SAL_CALL
    ScAccessibleDocumentPagePreview::getImplementationName (void)
    throw (uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleDocumentPagePreview"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL
    ScAccessibleDocumentPagePreview::getSupportedServiceNames (void)
        throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.AccessibleSpreadsheetPageView"));

    return aSequence;
}

    //=====  internal  ========================================================

::rtl::OUString SAL_CALL
    ScAccessibleDocumentPagePreview::createAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("This is a page preview of a Spreadsheet Document."));
}

::rtl::OUString SAL_CALL
    ScAccessibleDocumentPagePreview::createAccessibleName (void)
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM ("Spreadsheet Document Page Preview "));
    return sName;
}

Rectangle ScAccessibleDocumentPagePreview::GetBoundingBoxOnScreen()
    throw (uno::RuntimeException)
{
    Rectangle aRect;
    DBG_ERROR("not implemented");
    return aRect;
}

Rectangle ScAccessibleDocumentPagePreview::GetBoundingBox()
    throw (uno::RuntimeException)
{
    Rectangle aRect;
    DBG_ERROR("not implemented");
    return aRect;
}

sal_Bool ScAccessibleDocumentPagePreview::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}
