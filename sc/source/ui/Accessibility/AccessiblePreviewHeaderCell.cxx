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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <tools/gen.hxx>
#include "AccessibleText.hxx"
#include "editsrc.hxx"
#include <svx/AccessibleTextHelper.hxx>
#include "AccessiblePreviewHeaderCell.hxx"
#include "AccessibilityHints.hxx"
#include "prevwsh.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"
#include "prevloc.hxx"
#include "scresid.hxx"
#ifndef SC_SC_HRC
#include "sc.hrc"
#endif

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <vcl/window.hxx>
#include <svl/smplhint.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/helper/convert.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessiblePreviewHeaderCell::ScAccessiblePreviewHeaderCell( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell,
                            const ScAddress& rCellPos, sal_Bool bIsColHdr, sal_Bool bIsRowHdr,
                            sal_Int32 nIndex ) :
    ScAccessibleContextBase( rxParent, AccessibleRole::TABLE_CELL ),
    mpViewShell( pViewShell ),
    mpTextHelper( NULL ),
    mnIndex( nIndex ),
    maCellPos( rCellPos ),
    mbColumnHeader( bIsColHdr ),
    mbRowHeader( bIsRowHdr ),
    mpTableInfo( NULL )
{
    if (mpViewShell)
        mpViewShell->AddAccessibilityObject(*this);
}

ScAccessiblePreviewHeaderCell::~ScAccessiblePreviewHeaderCell()
{
    if (mpViewShell)
        mpViewShell->RemoveAccessibilityObject(*this);
}

void SAL_CALL ScAccessiblePreviewHeaderCell::disposing()
{
    ScUnoGuard aGuard;
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

       if (mpTableInfo)
        DELETEZ (mpTableInfo);

    ScAccessibleContextBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessiblePreviewHeaderCell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( SfxSimpleHint ))
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        sal_uLong nId = rRef.GetId();
        if (nId == SC_HINT_ACC_VISAREACHANGED)
        {
            if (mpTextHelper)
                mpTextHelper->UpdateChildren();
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            //  column / row layout may change with any document change,
            //  so it must be invalidated
            DELETEZ( mpTableInfo );
        }
    }

    ScAccessibleContextBase::Notify(rBC, rHint);
}

//=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
{
    uno::Any aAny (ScAccessiblePreviewHeaderCellImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBase::queryInterface(rType);
}

void SAL_CALL ScAccessiblePreviewHeaderCell::acquire()
    throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessiblePreviewHeaderCell::release()
    throw ()
{
    ScAccessibleContextBase::release();
}

//=====  XAccessibleValue  ================================================

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::getCurrentValue() throw (uno::RuntimeException)
{
     ScUnoGuard aGuard;
    IsObjectValid();

    double fValue(0.0);
    if (mbColumnHeader)
        fValue = maCellPos.Col();
    else
        fValue = maCellPos.Row();

    uno::Any aAny;
    aAny <<= fValue;
    return aAny;
}

sal_Bool SAL_CALL ScAccessiblePreviewHeaderCell::setCurrentValue( const uno::Any& /* aNumber */ )
                                                                throw (uno::RuntimeException)
{
    //  it is not possible to set a value
    return sal_False;
}

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::getMaximumValue() throw (uno::RuntimeException)
{
     ScUnoGuard aGuard;
    IsObjectValid();

    double fValue(0.0);
    if (mbColumnHeader)
        fValue = MAXCOL;
    else
        fValue = MAXROW;
    uno::Any aAny;
    aAny <<= fValue;
    return aAny;
}

uno::Any SAL_CALL ScAccessiblePreviewHeaderCell::getMinimumValue() throw (uno::RuntimeException)
{
    double fValue(0.0);
    uno::Any aAny;
    aAny <<= fValue;
    return aAny;
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleAtPoint( const awt::Point& rPoint )
                                throw (uno::RuntimeException)
{
    uno::Reference<XAccessible> xRet;
    if (containsPoint(rPoint))
    {
         ScUnoGuard aGuard;
        IsObjectValid();

        if(!mpTextHelper)
            CreateTextHelper();

        xRet = mpTextHelper->GetAt(rPoint);
    }

    return xRet;
}

void SAL_CALL ScAccessiblePreviewHeaderCell::grabFocus() throw (uno::RuntimeException)
{
     ScUnoGuard aGuard;
    IsObjectValid();
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
            xAccessibleComponent->grabFocus();
    }
}

//=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleChildCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleChild(sal_Int32 nIndex)
                            throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

sal_Int32 SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleIndexInParent() throw (uno::RuntimeException)
{
    return mnIndex;
}

uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessiblePreviewHeaderCell::getAccessibleStateSet()
                            throw(uno::RuntimeException)
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
    else
    {
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::MULTI_LINE);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        pStateSet->AddState(AccessibleStateType::TRANSIENT);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

//=====  XServiceInfo  ====================================================

rtl::OUString SAL_CALL ScAccessiblePreviewHeaderCell::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScAccessiblePreviewHeaderCell"));
}

uno::Sequence<rtl::OUString> SAL_CALL ScAccessiblePreviewHeaderCell::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.table.AccessibleCellView"));

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessiblePreviewHeaderCell::getTypes()
        throw (uno::RuntimeException)
{
    return comphelper::concatSequences(ScAccessiblePreviewHeaderCellImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessiblePreviewHeaderCell::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        aId.realloc (16);
        rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
    }
    return aId;
}

//====  internal  =========================================================

Rectangle ScAccessiblePreviewHeaderCell::GetBoundingBoxOnScreen() const throw (uno::RuntimeException)
{
    Rectangle aCellRect;

    FillTableInfo();

    if (mpTableInfo)
    {
        const ScPreviewColRowInfo& rColInfo = mpTableInfo->GetColInfo()[maCellPos.Col()];
        const ScPreviewColRowInfo& rRowInfo = mpTableInfo->GetRowInfo()[maCellPos.Row()];

        aCellRect = Rectangle( rColInfo.nPixelStart, rRowInfo.nPixelStart, rColInfo.nPixelEnd, rRowInfo.nPixelEnd );
    }

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

Rectangle ScAccessiblePreviewHeaderCell::GetBoundingBox() const throw (uno::RuntimeException)
{
    FillTableInfo();

    if (mpTableInfo)
    {
        const ScPreviewColRowInfo& rColInfo = mpTableInfo->GetColInfo()[maCellPos.Col()];
        const ScPreviewColRowInfo& rRowInfo = mpTableInfo->GetRowInfo()[maCellPos.Row()];

        Rectangle aCellRect( rColInfo.nPixelStart, rRowInfo.nPixelStart, rColInfo.nPixelEnd, rRowInfo.nPixelEnd );
        uno::Reference<XAccessible> xAccParent = const_cast<ScAccessiblePreviewHeaderCell*>(this)->getAccessibleParent();
        if (xAccParent.is())
        {
            uno::Reference<XAccessibleContext> xAccParentContext = xAccParent->getAccessibleContext();
            uno::Reference<XAccessibleComponent> xAccParentComp (xAccParentContext, uno::UNO_QUERY);
            if (xAccParentComp.is())
            {
                Rectangle aParentRect (VCLRectangle(xAccParentComp->getBounds()));
                aCellRect.setX(aCellRect.getX() - aParentRect.getX());
                aCellRect.setY(aCellRect.getY() - aParentRect.getY());
            }
        }
        return aCellRect;
    }
    return Rectangle();
}

rtl::OUString SAL_CALL ScAccessiblePreviewHeaderCell::createAccessibleDescription() throw(uno::RuntimeException)
{
    rtl::OUString sDescription = String(ScResId(STR_ACC_HEADERCELL_DESCR));
    return sDescription;
}

rtl::OUString SAL_CALL ScAccessiblePreviewHeaderCell::createAccessibleName() throw(uno::RuntimeException)
{
    rtl::OUString sName = String(ScResId(STR_ACC_HEADERCELL_NAME));

    if ( mbColumnHeader )
    {
        if ( mbRowHeader )
        {
            //! name for corner cell?

//          sName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Column/Row Header"));
        }
        else
        {
            // name of column header
            sName += ScColToAlpha( maCellPos.Col() );
        }
    }
    else
    {
        // name of row header
        sName += rtl::OUString::valueOf( (sal_Int32) ( maCellPos.Row() + 1 ) );
    }

    return sName;
}

sal_Bool ScAccessiblePreviewHeaderCell::IsDefunc( const uno::Reference<XAccessibleStateSet>& rxParentStates )
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == NULL) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

void ScAccessiblePreviewHeaderCell::CreateTextHelper()
{
    if (!mpTextHelper)
    {
        ::std::auto_ptr < ScAccessibleTextData > pAccessiblePreviewHeaderCellTextData
            (new ScAccessiblePreviewHeaderCellTextData(mpViewShell, String(getAccessibleName()), maCellPos, mbColumnHeader, mbRowHeader));
        ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessiblePreviewHeaderCellTextData));

        mpTextHelper = new ::accessibility::AccessibleTextHelper(pEditSource );
        mpTextHelper->SetEventSource(this);
    }
}

void ScAccessiblePreviewHeaderCell::FillTableInfo() const
{
    if ( mpViewShell && !mpTableInfo )
    {
        Size aOutputSize;
        Window* pWindow = mpViewShell->GetWindow();
        if ( pWindow )
            aOutputSize = pWindow->GetOutputSizePixel();
        Point aPoint;
        Rectangle aVisRect( aPoint, aOutputSize );

        mpTableInfo = new ScPreviewTableInfo;
        mpViewShell->GetLocationData().GetTableInfo( aVisRect, *mpTableInfo );
    }
}
