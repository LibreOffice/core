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
#include "precompiled_sw.hxx"


#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <accselectionhelper.hxx>

#include <acccontext.hxx>
#include <accmap.hxx>
#include <svx/AccessibleShape.hxx>
#include <viewsh.hxx>
#include <fesh.hxx>
#include <vcl/svapp.hxx>        // for SolarMutex
#include <tools/debug.hxx>
#include <flyfrm.hxx>


#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <fmtanchr.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;
using ::com::sun::star::accessibility::XAccessibleSelection;

using namespace ::sw::access;

SwAccessibleSelectionHelper::SwAccessibleSelectionHelper(
    SwAccessibleContext& rCtxt ) :
        rContext( rCtxt )
{
}

SwAccessibleSelectionHelper::~SwAccessibleSelectionHelper()
{
}

SwFEShell* SwAccessibleSelectionHelper::GetFEShell()
{
    DBG_ASSERT( rContext.GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = rContext.GetMap()->GetShell();
    DBG_ASSERT( pViewShell != NULL,
                "No view shell? Then what are you looking at?" );

    SwFEShell* pFEShell = NULL;
    if( pViewShell->ISA( SwFEShell ) )
    {
        pFEShell = static_cast<SwFEShell*>( pViewShell );
    }

    return pFEShell;
}

void SwAccessibleSelectionHelper::throwIndexOutOfBoundsException()
        throw ( lang::IndexOutOfBoundsException )
{
    Reference < XAccessibleContext > xThis( &rContext );
    Reference < XAccessibleSelection >xSelThis( xThis, UNO_QUERY );
    lang::IndexOutOfBoundsException aExcept(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("index out of bounds") ),
                xSelThis );                                     \
    throw aExcept;
}


//=====  XAccessibleSelection  ============================================

void SwAccessibleSelectionHelper::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Get the respective child as SwFrm (also do index checking), ...
    const SwAccessibleChild aChild = rContext.GetChild( *(rContext.GetMap()),
                                                        nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // we can only select fly frames, so we ignore (should: return
    // false) all other attempts at child selection
    sal_Bool bRet = sal_False;
    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell != NULL )
    {
        const SdrObject *pObj = aChild.GetDrawObject();
        if( pObj )
        {
            bRet = rContext.Select( const_cast< SdrObject *>( pObj ), 0==aChild.GetSwFrm());
        }
    }
    // no frame shell, or no frame, or no fly frame -> can't select

    // return bRet;
}

//When the selected state of the SwFrmOrObj is setted, return true.
static sal_Bool lcl_getSelectedState(const SwAccessibleChild& aChild,
                                     SwAccessibleContext* pContext,
                                     SwAccessibleMap* pMap)
{
    Reference< XAccessible > xAcc;
    if ( aChild.GetSwFrm() )
    {
        xAcc = pMap->GetContext( aChild.GetSwFrm(), sal_False );
    }
    else if ( aChild.GetDrawObject() )
    {
        xAcc = pMap->GetContext( aChild.GetDrawObject(), pContext, sal_False );
    }

    if( xAcc.is() )
    {
        Reference< XAccessibleContext > pRContext = xAcc->getAccessibleContext();
        if(!pRContext.is())
            return sal_False;
        Reference<XAccessibleStateSet> pRStateSet = pRContext->getAccessibleStateSet();
        if( pRStateSet.is() )
        {
            Sequence<short> pStates = pRStateSet->getStates();
            long count = pStates.getLength();
            for( int i = 0; i < count; i++ )
            {
                if( pStates[i] == AccessibleStateType::SELECTED)
                    return sal_True;
            }
        }
    }
    return sal_False;
}

sal_Bool SwAccessibleSelectionHelper::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Get the respective child as SwFrm (also do index checking), ...
    const SwAccessibleChild aChild = rContext.GetChild( *(rContext.GetMap()),
                                                        nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // ... and compare to the currently selected frame
    sal_Bool bRet = sal_False;
    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell )
    {
        if ( aChild.GetSwFrm() != 0 )
        {
            bRet = (pFEShell->GetCurrFlyFrm() == aChild.GetSwFrm());
        }
        else if ( aChild.GetDrawObject() )
        {
            bRet = pFEShell->IsObjSelected( *aChild.GetDrawObject() );
        }
        //If the SwFrmOrObj is not selected directly in the UI, we should check whether it is selected in the selection cursor.
        if( !bRet )
        {
            if( lcl_getSelectedState( aChild, &rContext, rContext.GetMap() ) == sal_True)
                bRet = sal_True;
        }
    }

    return bRet;
}

void SwAccessibleSelectionHelper::clearAccessibleSelection(  )
    throw ( RuntimeException )
{
    // return sal_False     // we can't deselect
}

void SwAccessibleSelectionHelper::selectAllAccessibleChildren(  )
    throw ( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // We can select only one. So iterate over the children to find
    // the first we can select, and select it.

    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell )
    {
        ::std::list< SwAccessibleChild > aChildren;
        rContext.GetChildren( *(rContext.GetMap()), aChildren );

        ::std::list< SwAccessibleChild >::const_iterator aIter = aChildren.begin();
        ::std::list< SwAccessibleChild >::const_iterator aEndIter = aChildren.end();
        while( aIter != aEndIter )
        {
            const SwAccessibleChild& rChild = *aIter;
            const SdrObject* pObj = rChild.GetDrawObject();
            const SwFrm* pFrm = rChild.GetSwFrm();
            if( pObj && !(pFrm != 0 && pFEShell->IsObjSelected()) )
            {
                rContext.Select( const_cast< SdrObject *>( pObj ), 0==pFrm );
                if( pFrm )
                    break;
            }
            ++aIter;
        }
    }
}

sal_Int32 SwAccessibleSelectionHelper::getSelectedAccessibleChildCount(  )
    throw ( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_Int32 nCount = 0;
    // Only one frame can be selected at a time, and we only frames
    // for selectable children.
    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell != 0 )
    {
        const SwFlyFrm* pFlyFrm = pFEShell->GetCurrFlyFrm();
        if( pFlyFrm )
        {
            //if( rContext.GetParent( SwAccessibleChild(pFlyFrm), rContext.IsInPagePreview()) ==
            //        rContext.GetFrm() )
                nCount = 1;
        }
        else
        {
            sal_uInt16 nSelObjs = pFEShell->IsObjSelected();
            if( nSelObjs > 0 )
            {
                ::std::list< SwAccessibleChild > aChildren;
                rContext.GetChildren( *(rContext.GetMap()), aChildren );

                ::std::list< SwAccessibleChild >::const_iterator aIter =
                    aChildren.begin();
                ::std::list< SwAccessibleChild >::const_iterator aEndIter =
                    aChildren.end();
                while( aIter != aEndIter && nCount < nSelObjs )
                {
                    const SwAccessibleChild& rChild = *aIter;
                    if( rChild.GetDrawObject() && !rChild.GetSwFrm() &&
                        rContext.GetParent(rChild, rContext.IsInPagePreview())
                           == rContext.GetFrm() &&
                        pFEShell->IsObjSelected( *rChild.GetDrawObject() ) )
                    {
                        nCount++;
                    }
                    ++aIter;
                }
            }
        }
        //If the SwFrmOrObj is not selected directly in the UI,
        //we should check whether it is selected in the selection cursor.
        if( nCount == 0 )
        {
            ::std::list< SwAccessibleChild > aChildren;
            rContext.GetChildren( *(rContext.GetMap()), aChildren );
            ::std::list< SwAccessibleChild >::const_iterator aIter =
                aChildren.begin();
            ::std::list< SwAccessibleChild >::const_iterator aEndIter =
                aChildren.end();
            while( aIter != aEndIter )
            {
                const SwAccessibleChild& aChild = *aIter;
                if( lcl_getSelectedState( aChild, &rContext, rContext.GetMap() ) )
                    nCount++;
                ++aIter;
            }
        }
    }
    return nCount;
}

Reference<XAccessible> SwAccessibleSelectionHelper::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Since the index is relative to the selected children, and since
    // there can be at most one selected frame child, the index must
    // be 0, and a selection must exist, otherwise we have to throw an
    // lang::IndexOutOfBoundsException
    SwFEShell* pFEShell = GetFEShell();
    if( 0 == pFEShell )
        throwIndexOutOfBoundsException();

    SwAccessibleChild aChild;
    const SwFlyFrm *pFlyFrm = pFEShell->GetCurrFlyFrm();
    if( pFlyFrm )
    {
        if( 0 == nSelectedChildIndex )
        {
            if(rContext.GetParent( SwAccessibleChild(pFlyFrm), rContext.IsInPagePreview()) == rContext.GetFrm() )
            {
                aChild = pFlyFrm;
            }
            else
            {
                const SwFrmFmt *pFrmFmt = pFlyFrm->GetFmt();
                if (pFrmFmt)
                {
                    const SwFmtAnchor& pAnchor = pFrmFmt->GetAnchor();
                    if( pAnchor.GetAnchorId() == FLY_AS_CHAR )
                    {
                        const SwFrm  *pParaFrm =  rContext.GetParent( SwAccessibleChild(pFlyFrm), rContext.IsInPagePreview() );
                        aChild  = pParaFrm;
                    }
                }
            }
        }
    }
    else
    {
        sal_uInt16 nSelObjs = pFEShell->IsObjSelected();
        if( 0 == nSelObjs || nSelectedChildIndex >= nSelObjs )
            throwIndexOutOfBoundsException();

        ::std::list< SwAccessibleChild > aChildren;
        rContext.GetChildren( *(rContext.GetMap()), aChildren );

        ::std::list< SwAccessibleChild >::const_iterator aIter = aChildren.begin();
        ::std::list< SwAccessibleChild >::const_iterator aEndIter = aChildren.end();
        while( aIter != aEndIter && !aChild.IsValid() )
        {
            const SwAccessibleChild& rChild = *aIter;
            if( rChild.GetDrawObject() && !rChild.GetSwFrm() &&
                rContext.GetParent(rChild, rContext.IsInPagePreview()) ==
                    rContext.GetFrm() &&
                pFEShell->IsObjSelected( *rChild.GetDrawObject() ) )
            {
                if( 0 == nSelectedChildIndex )
                    aChild = rChild;
                else
                    --nSelectedChildIndex;
            }
            ++aIter;
        }
    }

    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    DBG_ASSERT( rContext.GetMap() != NULL, "We need the map." );
    Reference< XAccessible > xChild;
    if( aChild.GetSwFrm() )
    {
        ::vos::ORef < SwAccessibleContext > xChildImpl(
                rContext.GetMap()->GetContextImpl( aChild.GetSwFrm(),
                sal_True ) );
        if( xChildImpl.isValid() )
        {
            xChildImpl->SetParent( &rContext );
            xChild = xChildImpl.getBodyPtr();
        }
    }
    else if ( aChild.GetDrawObject() )
    {
        ::vos::ORef < ::accessibility::AccessibleShape > xChildImpl(
                rContext.GetMap()->GetContextImpl( aChild.GetDrawObject(),
                                          &rContext, sal_True )  );
        if( xChildImpl.isValid() )
            xChild = xChildImpl.getBodyPtr();
    }
    return xChild;
}

// --> OD 2004-11-16 #111714# - index has to be treated as global child index.
void SwAccessibleSelectionHelper::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException )
{
    // return sal_False     // we can't deselect
    if( nChildIndex < 0 ||
        nChildIndex >= rContext.GetChildCount( *(rContext.GetMap()) ) )
        throwIndexOutOfBoundsException();
}
