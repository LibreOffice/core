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

#include <svxrectctaccessiblecontext.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <sal/log.hxx>
#include <vcl/settings.hxx>
#include <svx/strings.hrc>
#include <svx/dlgctrl.hxx>
#include <svx/dialmgr.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <unotools/accessiblerelationsethelper.hxx>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

using namespace ::com::sun::star::lang;

#define MAX_NUM_OF_CHILDREN   9
#define NOCHILDSELECTED     -1

// internal
namespace
{
    struct ChildIndexToPointData
    {
        const char* pResIdName;
        const char* pResIdDescr;
        RectPoint  ePoint;
    };
}


static const ChildIndexToPointData* IndexToPoint( long nIndex )
{
    DBG_ASSERT( nIndex < 9 && nIndex >= 0, "-IndexToPoint(): invalid child index! You have been warned..." );

    // corners are counted from left to right and top to bottom
    static const ChildIndexToPointData  pCornerData[] =
    {                                                                   // index
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_LT, RID_SVXSTR_RECTCTL_ACC_CHLD_LT, RectPoint::LT },    //  0
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_MT, RID_SVXSTR_RECTCTL_ACC_CHLD_MT, RectPoint::MT },    //  1
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_RT, RID_SVXSTR_RECTCTL_ACC_CHLD_RT, RectPoint::RT },    //  2
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_LM, RID_SVXSTR_RECTCTL_ACC_CHLD_LM, RectPoint::LM },    //  3
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_MM, RID_SVXSTR_RECTCTL_ACC_CHLD_MM, RectPoint::MM },    //  4
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_RM, RID_SVXSTR_RECTCTL_ACC_CHLD_RM, RectPoint::RM },    //  5
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_LB, RID_SVXSTR_RECTCTL_ACC_CHLD_LB, RectPoint::LB },    //  6
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_MB, RID_SVXSTR_RECTCTL_ACC_CHLD_MB, RectPoint::MB },    //  7
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_RB, RID_SVXSTR_RECTCTL_ACC_CHLD_RB, RectPoint::RB }     //  8
    };

    return pCornerData + nIndex;
}


static long PointToIndex( RectPoint ePoint )
{
    long    nRet( static_cast<long>(ePoint) );
    // corner control
    // corners are counted from left to right and top to bottom
    DBG_ASSERT( int(RectPoint::LT) == 0 && int(RectPoint::MT) == 1 && int(RectPoint::RT) == 2 && int(RectPoint::LM) == 3 && int(RectPoint::MM) == 4 && int(RectPoint::RM) == 5 &&
                int(RectPoint::LB) == 6 && int(RectPoint::MB) == 7 && int(RectPoint::RB) == 8, "*PointToIndex(): unexpected enum value!" );

    nRet = static_cast<long>(ePoint);

    return nRet;
}

SvxRectCtlAccessibleContext::SvxRectCtlAccessibleContext(SvxRectCtl* pRepr)
    : mpRepr(pRepr)
    , mnSelectedChild(NOCHILDSELECTED)
{
    {
        ::SolarMutexGuard aSolarGuard;
        msName = SvxResId( RID_SVXSTR_RECTCTL_ACC_CORN_NAME );
        msDescription = SvxResId( RID_SVXSTR_RECTCTL_ACC_CORN_DESCR );
    }

    mvChildren.resize(MAX_NUM_OF_CHILDREN);
}

SvxRectCtlAccessibleContext::~SvxRectCtlAccessibleContext()
{
    ensureDisposed();
}

IMPLEMENT_FORWARD_XINTERFACE2( SvxRectCtlAccessibleContext, OAccessibleSelectionHelper, OAccessibleHelper_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxRectCtlAccessibleContext, OAccessibleSelectionHelper, OAccessibleHelper_Base )

Reference< XAccessible > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleAtPoint( const awt::Point& rPoint )
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    Reference< XAccessible >    xRet;

    long nChild = mpRepr ? PointToIndex(mpRepr->GetApproxRPFromPixPt(rPoint)) : NOCHILDSELECTED;

    if (nChild != NOCHILDSELECTED)
        xRet = getAccessibleChild( nChild );

    return xRet;
}

// XAccessibleContext
sal_Int32 SAL_CALL SvxRectCtlAccessibleContext::getAccessibleChildCount()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return SvxRectCtl::NO_CHILDREN;
}

Reference< XAccessible > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleChild( sal_Int32 nIndex )
{
    checkChildIndex( nIndex );

    Reference< XAccessible > xChild(mvChildren[ nIndex ].get());
    if( !xChild.is() )
    {
        ::SolarMutexGuard aSolarGuard;

        ::osl::MutexGuard   aGuard( m_aMutex );

        xChild = mvChildren[ nIndex ].get();

        if (!xChild.is() && mpRepr)
        {
            const ChildIndexToPointData*    p = IndexToPoint( nIndex );
            OUString aName(SvxResId(p->pResIdName));
            OUString aDescr(SvxResId(p->pResIdDescr));

            tools::Rectangle       aFocusRect( mpRepr->CalculateFocusRectangle( p->ePoint ) );

            SvxRectCtlChildAccessibleContext*   pChild = new SvxRectCtlChildAccessibleContext(this, aName,
                    aDescr, aFocusRect, nIndex );
            mvChildren[ nIndex ] = pChild;
            xChild = pChild;

            // set actual state
            if( mnSelectedChild == nIndex )
                pChild->setStateChecked( true );
        }
    }

    return xChild;
}

Reference< XAccessible > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleParent()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (mpRepr)
        return mpRepr->getAccessibleParent();
    return uno::Reference<css::accessibility::XAccessible>();
}

sal_Int16 SAL_CALL SvxRectCtlAccessibleContext::getAccessibleRole()
{
    return AccessibleRole::PANEL;
}

OUString SAL_CALL SvxRectCtlAccessibleContext::getAccessibleDescription()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msDescription + " Please use arrow key to selection.";
}

OUString SAL_CALL SvxRectCtlAccessibleContext::getAccessibleName()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msName;
}

/** Return empty reference to indicate that the relation set is not
    supported.
*/
Reference< XAccessibleRelationSet > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleRelationSet()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    if (mpRepr)
        return mpRepr->get_accessible_relation_set();
    return uno::Reference<css::accessibility::XAccessibleRelationSet>();
}

Reference< XAccessibleStateSet > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleStateSet()
{
    ::osl::MutexGuard                       aGuard( m_aMutex );
    utl::AccessibleStateSetHelper*          pStateSetHelper = new utl::AccessibleStateSetHelper;

    if (mpRepr)
    {
        pStateSetHelper->AddState( AccessibleStateType::ENABLED );
        pStateSetHelper->AddState( AccessibleStateType::FOCUSABLE );
        if( mpRepr->HasFocus() )
            pStateSetHelper->AddState( AccessibleStateType::FOCUSED );
        pStateSetHelper->AddState( AccessibleStateType::OPAQUE );

        pStateSetHelper->AddState( AccessibleStateType::SHOWING );

        if( mpRepr->IsVisible() )
            pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}

void SAL_CALL SvxRectCtlAccessibleContext::grabFocus()
{
    ::SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    if (mpRepr)
        mpRepr->GrabFocus();
}

sal_Int32 SvxRectCtlAccessibleContext::getForeground()
{
    ::SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    //see SvxRectCtl::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetLabelTextColor());
}

sal_Int32 SvxRectCtlAccessibleContext::getBackground(  )
{
    ::SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    //see SvxRectCtl::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetDialogColor());
}

// XAccessibleSelection
void SvxRectCtlAccessibleContext::implSelect(sal_Int32 nIndex, bool bSelect)
{
    ::SolarMutexGuard aSolarGuard;

    ::osl::MutexGuard   aGuard( m_aMutex );

    checkChildIndex( nIndex );

    const ChildIndexToPointData*    pData = IndexToPoint( nIndex );

    DBG_ASSERT(pData, "SvxRectCtlAccessibleContext::selectAccessibleChild(): this is an impossible state! Or at least should be...");

    if (mpRepr)
    {
        if (bSelect)
        {
            // this does all what is needed, including the change of the child's state!
            mpRepr->SetActualRP( pData->ePoint );
        }
        else
        {
            SAL_WARN( "svx", "SvxRectCtlAccessibleContext::clearAccessibleSelection() is not possible!" );
        }
    }
}

bool SvxRectCtlAccessibleContext::implIsSelected( sal_Int32 nIndex )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    checkChildIndex( nIndex );

    return nIndex == mnSelectedChild;
}

// internals
void SvxRectCtlAccessibleContext::checkChildIndex( long nIndex )
{
    if( nIndex < 0 || nIndex >= getAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();
}

void SvxRectCtlAccessibleContext::FireChildFocus( RectPoint eButton )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    long nNew = PointToIndex( eButton );
    long nNumOfChildren = getAccessibleChildCount();
    if( nNew < nNumOfChildren )
    {
        // select new child
        mnSelectedChild = nNew;
        if( nNew != NOCHILDSELECTED )
        {
            if( mvChildren[ nNew ].is() )
                mvChildren[ nNew ]->FireFocusEvent();
        }
        else
        {
            Any                             aOld;
            Any                             aNew;
            aNew <<= AccessibleStateType::FOCUSED;
            NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOld, aNew);
        }
    }
    else
        mnSelectedChild = NOCHILDSELECTED;
}

void SvxRectCtlAccessibleContext::selectChild( long nNew )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    if( nNew == mnSelectedChild )
        return;

    long    nNumOfChildren = getAccessibleChildCount();
    if( nNew < nNumOfChildren )
    {   // valid index
        if( mnSelectedChild != NOCHILDSELECTED )
        {   // deselect old selected child if one is selected
            SvxRectCtlChildAccessibleContext* pChild = mvChildren[ mnSelectedChild ].get();
            if( pChild )
                pChild->setStateChecked( false );
        }

        // select new child
        mnSelectedChild = nNew;

        if( nNew != NOCHILDSELECTED )
        {
            if( mvChildren[ nNew ].is() )
                mvChildren[ nNew ]->setStateChecked( true );
        }
    }
    else
        mnSelectedChild = NOCHILDSELECTED;
}

void SvxRectCtlAccessibleContext::selectChild(RectPoint eButton )
{
    // no guard -> is done in next selectChild
    selectChild(PointToIndex( eButton ));
}

void SAL_CALL SvxRectCtlAccessibleContext::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OAccessibleSelectionHelper::disposing();
    for (auto & rxChild : mvChildren)
    {
        if( rxChild.is() )
            rxChild->dispose();
    }
    mvChildren.clear();
    mpRepr = nullptr;
}

awt::Rectangle SvxRectCtlAccessibleContext::implGetBounds()
{
    ::SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    awt::Rectangle aRet;

    if (mpRepr)
    {
        const Point   aOutPos;
        Size          aOutSize(mpRepr->GetOutputSizePixel());

        aRet.X = aOutPos.X();
        aRet.Y = aOutPos.Y();
        aRet.Width = aOutSize.Width();
        aRet.Height = aOutSize.Height();
    }

    return aRet;
}

SvxRectCtlChildAccessibleContext::SvxRectCtlChildAccessibleContext(
    const Reference<XAccessible>&   rxParent,
    const OUString&              rName,
    const OUString&              rDescription,
    const tools::Rectangle& rBoundingBox,
    long nIndexInParent )
    : msDescription( rDescription )
    , msName( rName )
    , mxParent(rxParent)
    , maBoundingBox( rBoundingBox )
    , mnIndexInParent( nIndexInParent )
    , mbIsChecked( false )
{
}

SvxRectCtlChildAccessibleContext::~SvxRectCtlChildAccessibleContext()
{
    ensureDisposed();
}

Reference< XAccessible > SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleAtPoint( const awt::Point& /*rPoint*/ )
{
    return Reference< XAccessible >();
}

void SAL_CALL SvxRectCtlChildAccessibleContext::grabFocus()
{
}

sal_Int32 SvxRectCtlChildAccessibleContext::getForeground(  )
{
    ::SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    //see SvxRectCtl::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetLabelTextColor());
}

sal_Int32 SvxRectCtlChildAccessibleContext::getBackground(  )
{
    ::SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard   aGuard( m_aMutex );

    //see SvxRectCtl::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetDialogColor());
}

// XAccessibleContext
sal_Int32 SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleChildCount()
{
    return 0;
}

Reference< XAccessible > SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleChild( sal_Int32 /*nIndex*/ )
{
    throw lang::IndexOutOfBoundsException();
}

Reference< XAccessible > SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleParent()
{
    return mxParent;
}

sal_Int16 SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleRole()
{
    return AccessibleRole::RADIO_BUTTON;
}

OUString SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleDescription()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msDescription;
}

OUString SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleName()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msName;
}

/** Return empty reference to indicate that the relation set is not
    supported.
*/
Reference<XAccessibleRelationSet> SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleRelationSet()
{
    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    uno::Reference< css::accessibility::XAccessibleRelationSet > xSet = pRelationSetHelper;
    if( mxParent.is() )
      {
        uno::Sequence< uno::Reference< uno::XInterface > > aSequence { mxParent };
        pRelationSetHelper->AddRelation( css::accessibility::AccessibleRelation( css::accessibility::AccessibleRelationType::MEMBER_OF, aSequence ) );

    }

    return xSet;
}

Reference< XAccessibleStateSet > SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleStateSet()
{
    ::osl::MutexGuard                       aGuard( m_aMutex );
    utl::AccessibleStateSetHelper*          pStateSetHelper = new utl::AccessibleStateSetHelper;

    if (!rBHelper.bDisposed)
    {
        if( mbIsChecked )
        {
            pStateSetHelper->AddState( AccessibleStateType::CHECKED );
        }

        pStateSetHelper->AddState( AccessibleStateType::ENABLED );
        pStateSetHelper->AddState( AccessibleStateType::SENSITIVE );
        pStateSetHelper->AddState( AccessibleStateType::OPAQUE );
        pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
        pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}

// XAccessibleValue
Any SAL_CALL SvxRectCtlChildAccessibleContext::getCurrentValue()
{
    Any aRet;
    aRet <<= ( mbIsChecked? 1.0 : 0.0 );
    return aRet;
}

sal_Bool SAL_CALL SvxRectCtlChildAccessibleContext::setCurrentValue( const Any& /*aNumber*/ )
{
    return false;
}

Any SAL_CALL SvxRectCtlChildAccessibleContext::getMaximumValue()
{
    Any aRet;
    aRet <<= 1.0;
    return aRet;
}

Any SAL_CALL SvxRectCtlChildAccessibleContext::getMinimumValue()
{
    Any aRet;
    aRet <<= 0.0;
    return aRet;
}


// XAccessibleAction


sal_Int32 SvxRectCtlChildAccessibleContext::getAccessibleActionCount( )
{
    return 1;
}


sal_Bool SvxRectCtlChildAccessibleContext::doAccessibleAction ( sal_Int32 nIndex )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    Reference<XAccessibleSelection> xSelection( mxParent, UNO_QUERY);

    xSelection->selectAccessibleChild(mnIndexInParent);

    return true;
}


OUString SvxRectCtlChildAccessibleContext::getAccessibleActionDescription ( sal_Int32 nIndex )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return "select";
}


Reference< XAccessibleKeyBinding > SvxRectCtlChildAccessibleContext::getAccessibleActionKeyBinding( sal_Int32 nIndex )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessibleKeyBinding >();
}

void SAL_CALL SvxRectCtlChildAccessibleContext::disposing()
{
    OAccessibleComponentHelper::disposing();
    mxParent.clear();
}

awt::Rectangle SvxRectCtlChildAccessibleContext::implGetBounds(  )
{
    // no guard necessary, because no one changes maBoundingBox after creating it
    return AWTRectangle(maBoundingBox);
}

void SvxRectCtlChildAccessibleContext::setStateChecked( bool bChecked )
{
    if( mbIsChecked == bChecked )
        return;

    mbIsChecked = bChecked;

    Any                             aOld;
    Any                             aNew;
    Any&                            rMod = bChecked? aNew : aOld;

    //Send the STATE_CHANGED(Focused) event to accessible
    rMod <<= AccessibleStateType::FOCUSED;
    NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOld, aNew);

    rMod <<= AccessibleStateType::CHECKED;

    NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOld, aNew);
}

void SvxRectCtlChildAccessibleContext::FireFocusEvent()
{
    Any                             aOld;
    Any                             aNew;
    aNew <<= AccessibleStateType::FOCUSED;
    NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOld, aNew);
}

IMPLEMENT_FORWARD_XINTERFACE2( SvxRectCtlChildAccessibleContext, OAccessibleComponentHelper, OAccessibleHelper_Base_3 )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxRectCtlChildAccessibleContext, OAccessibleComponentHelper, OAccessibleHelper_Base_3 )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
