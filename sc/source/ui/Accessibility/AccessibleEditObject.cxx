/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "AccessibleEditObject.hxx"
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include "unoguard.hxx"
#include "AccessibleText.hxx"
#include "editsrc.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#include <rtl/uuid.h>
#include <tools/debug.hxx>
#include <svx/AccessibleTextHelper.hxx>
//IAccessibility2 Implementation 2009-----
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdmodel.hxx>
#include <sfx2/objsh.hxx>

#include "unonames.hxx"
#include "document.hxx"
#include "AccessibleDocument.hxx"
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::RuntimeException;
//-----IAccessibility2 Implementation 2009
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleEditObject::ScAccessibleEditObject(
        const uno::Reference<XAccessible>& rxParent,
        EditView* pEditView, Window* pWin, const rtl::OUString& rName,
        const rtl::OUString& rDescription, EditObjectType eObjectType)
    :
    ScAccessibleContextBase(rxParent, AccessibleRole::TEXT_FRAME),
    mpTextHelper(NULL),
    mpEditView(pEditView),
    mpWindow(pWin),
    meObjectType(eObjectType),
    mbHasFocus(sal_False)
{
    CreateTextHelper();
    SetName(rName);
    SetDescription(rDescription);
//IAccessibility2 Implementation 2009-----
    if( meObjectType == CellInEditMode)
    {
        const ScAccessibleDocument *pAccDoc = const_cast<ScAccessibleDocument*>(static_cast<ScAccessibleDocument*>(rxParent.get())) ;
        if (pAccDoc)
        {
            m_pScDoc = pAccDoc->GetDocument();
            m_curCellAddress =pAccDoc->GetCurCellAddress();
        }
        else
        {
            m_pScDoc=NULL;
        }
    }
    else
        m_pScDoc=NULL;
//-----IAccessibility2 Implementation 2009
}

ScAccessibleEditObject::~ScAccessibleEditObject()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_incrementInterlockedCount( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void SAL_CALL ScAccessibleEditObject::disposing()
{
    ScUnoGuard aGuard;
    if (mpTextHelper)
        DELETEZ(mpTextHelper);

    ScAccessibleContextBase::disposing();
}

void ScAccessibleEditObject::LostFocus()
{
    mbHasFocus = sal_False;
    if (mpTextHelper)
        mpTextHelper->SetFocus(sal_False);
    CommitFocusLost();
}

void ScAccessibleEditObject::GotFocus()
{
    mbHasFocus = sal_True;
    CommitFocusGained();
    if (mpTextHelper)
        mpTextHelper->SetFocus(sal_True);
}

//IAccessibility2 Implementation 2009-----
//=====  XInterface  ==========================================================

com::sun::star::uno::Any SAL_CALL
    ScAccessibleEditObject::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = ScAccessibleContextBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast< ::com::sun::star::accessibility::XAccessibleSelection* >(this)
            );
    return aReturn;
}
void SAL_CALL
    ScAccessibleEditObject::acquire (void)
    throw ()
{
    ScAccessibleContextBase::acquire ();
}
void SAL_CALL
    ScAccessibleEditObject::release (void)
    throw ()
{
    ScAccessibleContextBase::release ();
}
//-----IAccessibility2 Implementation 2009
    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleEditObject::getAccessibleAtPoint(
        const awt::Point& rPoint )
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

Rectangle ScAccessibleEditObject::GetBoundingBoxOnScreen(void) const
        throw (uno::RuntimeException)
{
    Rectangle aScreenBounds;

    if ( mpWindow )
    {
        if ( meObjectType == CellInEditMode )
        {
            if ( mpEditView && mpEditView->GetEditEngine() )
            {
                MapMode aMapMode( mpEditView->GetEditEngine()->GetRefMapMode() );
                aScreenBounds = mpWindow->LogicToPixel( mpEditView->GetOutputArea(), aMapMode );
                Point aCellLoc = aScreenBounds.TopLeft();
                Rectangle aWindowRect = mpWindow->GetWindowExtentsRelative( NULL );
                Point aWindowLoc = aWindowRect.TopLeft();
                Point aPos( aCellLoc.getX() + aWindowLoc.getX(), aCellLoc.getY() + aWindowLoc.getY() );
                aScreenBounds.SetPos( aPos );
            }
        }
        else
        {
            aScreenBounds = mpWindow->GetWindowExtentsRelative( NULL );
        }
    }

    return aScreenBounds;
}

Rectangle ScAccessibleEditObject::GetBoundingBox(void) const
        throw (uno::RuntimeException)
{
    Rectangle aBounds( GetBoundingBoxOnScreen() );

    if ( mpWindow )
    {
        uno::Reference< XAccessible > xThis( mpWindow->GetAccessible() );
        if ( xThis.is() )
        {
            uno::Reference< XAccessibleContext > xContext( xThis->getAccessibleContext() );
            if ( xContext.is() )
            {
                uno::Reference< XAccessible > xParent( xContext->getAccessibleParent() );
                if ( xParent.is() )
                {
                    uno::Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), uno::UNO_QUERY );
                    if ( xParentComponent.is() )
                    {
                        Point aScreenLoc = aBounds.TopLeft();
                        awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();
                        Point aPos( aScreenLoc.getX() - aParentScreenLoc.X, aScreenLoc.getY() - aParentScreenLoc.Y );
                        aBounds.SetPos( aPos );
                    }
                }
            }
        }
    }

    return aBounds;
}

    //=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessibleEditObject::getAccessibleChildCount(void)
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleEditObject::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleEditObject::getAccessibleStateSet(void)
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
    else
    {
        // all states are const, because this object exists only in one state
        pStateSet->AddState(AccessibleStateType::EDITABLE);
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::SENSITIVE);
        pStateSet->AddState(AccessibleStateType::MULTI_LINE);
        pStateSet->AddState(AccessibleStateType::MULTI_SELECTABLE);
        pStateSet->AddState(AccessibleStateType::SHOWING);
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

::rtl::OUString SAL_CALL
    ScAccessibleEditObject::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
//    DBG_ERRORFILE("Should never be called, because is set in the constructor.")
    return rtl::OUString();
}

::rtl::OUString SAL_CALL
    ScAccessibleEditObject::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    DBG_ERRORFILE("Should never be called, because is set in the constructor.");
    return rtl::OUString();
}

    ///=====  XAccessibleEventBroadcaster  =====================================

void SAL_CALL
    ScAccessibleEditObject::addEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
        throw (uno::RuntimeException)
{
    if (!mpTextHelper)
        CreateTextHelper();

    mpTextHelper->AddEventListener(xListener);

    ScAccessibleContextBase::addEventListener(xListener);
}

void SAL_CALL
    ScAccessibleEditObject::removeEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
        throw (uno::RuntimeException)
{
    if (!mpTextHelper)
        CreateTextHelper();

    mpTextHelper->RemoveEventListener(xListener);

    ScAccessibleContextBase::removeEventListener(xListener);
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleEditObject::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleEditObject"));
}

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleEditObject::getImplementationId(void)
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

sal_Bool ScAccessibleEditObject::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || !getAccessibleParent().is() ||
         (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

void ScAccessibleEditObject::CreateTextHelper()
{
    if (!mpTextHelper)
    {
        ::std::auto_ptr < ScAccessibleTextData > pAccessibleTextData;
        if (meObjectType == CellInEditMode || meObjectType == EditControl)
        {
            pAccessibleTextData.reset
                (new ScAccessibleEditObjectTextData(mpEditView, mpWindow));
        }
        else
        {
            pAccessibleTextData.reset
                (new ScAccessibleEditLineTextData(NULL, mpWindow));
        }

        ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessibleTextData));
        mpTextHelper = new ::accessibility::AccessibleTextHelper(pEditSource );
        mpTextHelper->SetEventSource(this);
        mpTextHelper->SetFocus(mbHasFocus);

        // #i54814# activate cell in edit mode
        if( meObjectType == CellInEditMode )
        {
            // do not activate cell object, if top edit line is active
            const ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
            if( pInputHdl && !pInputHdl->IsTopMode() )
            {
                mpTextHelper->GetEditSource().GetBroadcaster().Broadcast(SdrBaseHint(HINT_BEGEDIT));
            }
        }
    }
}
//IAccessibility2 Implementation 2009-----
sal_Int32 SAL_CALL ScAccessibleEditObject::getForeground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return GetFgBgColor(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CCOLOR)));
}

sal_Int32 SAL_CALL ScAccessibleEditObject::getBackground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return GetFgBgColor(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLBACK)));
}
sal_Int32 ScAccessibleEditObject::GetFgBgColor( const rtl::OUString &strPropColor)
{
    ScUnoGuard aGuard;
    sal_Int32 nColor(0);
    if (m_pScDoc)
    {
        SfxObjectShell* pObjSh = m_pScDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(m_curCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(m_curCellAddress.Col(), m_curCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(strPropColor);
                                aAny >>= nColor;
                            }
                        }
                    }
                }
            }
        }
    }
    return nColor;
}
//=====  XAccessibleSelection  ============================================
//--------------------------------------------------------------------------------
void SAL_CALL ScAccessibleEditObject::selectAccessibleChild( sal_Int32 )
throw ( IndexOutOfBoundsException, RuntimeException )
{
}
//----------------------------------------------------------------------------------
sal_Bool SAL_CALL ScAccessibleEditObject::isAccessibleChildSelected( sal_Int32 nChildIndex )
throw ( IndexOutOfBoundsException,
       RuntimeException )
{
    uno::Reference<XAccessible> xAcc = getAccessibleChild( nChildIndex );
    uno::Reference<XAccessibleContext> xContext;
    if( xAcc.is() )
        xContext = xAcc->getAccessibleContext();
    if( xContext.is() )
    {
        if( xContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
        {
            uno::Reference< ::com::sun::star::accessibility::XAccessibleText >
                xText(xAcc, uno::UNO_QUERY);
            if( xText.is() )
            {
                if( xText->getSelectionStart() >= 0 ) return sal_True;
            }
        }
    }
    return sal_False;
}
//---------------------------------------------------------------------
void SAL_CALL ScAccessibleEditObject::clearAccessibleSelection(  )
throw ( RuntimeException )
{
}
//-------------------------------------------------------------------------
void SAL_CALL ScAccessibleEditObject::selectAllAccessibleChildren(  )
throw ( RuntimeException )
{
}
//----------------------------------------------------------------------------
sal_Int32 SAL_CALL ScAccessibleEditObject::getSelectedAccessibleChildCount()
throw ( RuntimeException )
{
    sal_Int32 nCount = 0;
    sal_Int32 TotalCount = getAccessibleChildCount();
    for( sal_Int32 i = 0; i < TotalCount; i++ )
        if( isAccessibleChildSelected(i) ) nCount++;
    return nCount;
}
//--------------------------------------------------------------------------------------
uno::Reference<XAccessible> SAL_CALL ScAccessibleEditObject::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
throw ( IndexOutOfBoundsException, RuntimeException)
{
    if ( nSelectedChildIndex > getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    sal_Int32 i1, i2;
    for( i1 = 0, i2 = 0; i1 < getAccessibleChildCount(); i1++ )
        if( isAccessibleChildSelected(i1) )
        {
            if( i2 == nSelectedChildIndex )
                return getAccessibleChild( i1 );
            i2++;
        }
    return uno::Reference<XAccessible>();
}
//----------------------------------------------------------------------------------
void SAL_CALL ScAccessibleEditObject::deselectAccessibleChild(
                                                            sal_Int32 )
                                                            throw ( IndexOutOfBoundsException,
                                                            RuntimeException )
{
}
uno::Reference< XAccessibleRelationSet > ScAccessibleEditObject::getAccessibleRelationSet(  )
    throw (uno::RuntimeException)
{
       ScUnoGuard aGuard;
    Window* pWindow = mpWindow;
    utl::AccessibleRelationSetHelper* rRelationSet = new utl::AccessibleRelationSetHelper;
    uno::Reference< XAccessibleRelationSet > rSet = rRelationSet;
    if ( pWindow )
    {
        Window *pLabeledBy = pWindow->GetAccessibleRelationLabeledBy();
        if ( pLabeledBy && pLabeledBy != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pLabeledBy->GetAccessible();
            rRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType::LABELED_BY, aSequence ) );
        }
        Window* pMemberOf = pWindow->GetAccessibleRelationMemberOf();
        if ( pMemberOf && pMemberOf != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pMemberOf->GetAccessible();
            rRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType::MEMBER_OF, aSequence ) );
        }
        return rSet;
    }
    return uno::Reference< XAccessibleRelationSet >();
}
//-----IAccessibility2 Implementation 2009

