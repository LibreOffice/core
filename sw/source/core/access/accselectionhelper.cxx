 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accselectionhelper.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:24:02 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif
#ifndef _ACCSELECTIONHELPER_HXX_
#include <accselectionhelper.hxx>
#endif

#ifndef _ACCCONTEXT_HXX
#include <acccontext.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif

#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _FESH_HXX
#include "fesh.hxx"
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>        // for SolarMutex
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;
using ::com::sun::star::accessibility::XAccessibleSelection;



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
    const SwFrmOrObj aChild = rContext.GetChild( nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // we can only select fly frames, so we ignore (should: return
    // false) all other attempts at child selection
    sal_Bool bRet = sal_False;
    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell != NULL )
    {
        const SdrObject *pObj = aChild.GetSdrObject();
        if( pObj )
        {
            bRet = rContext.Select( const_cast< SdrObject *>( pObj ), 0==aChild.GetSwFrm());
        }
    }
    // no frame shell, or no frame, or no fly frame -> can't select

    // return bRet;
}

sal_Bool SwAccessibleSelectionHelper::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Get the respective child as SwFrm (also do index checking), ...
    const SwFrmOrObj aChild = rContext.GetChild( nChildIndex );
    if( !aChild.IsValid() )
        throwIndexOutOfBoundsException();

    // ... and compare to the currently selected frame
    sal_Bool bRet = sal_False;
    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell )
    {
        if( aChild.GetSwFrm() != 0 )
        {
            bRet = (pFEShell->GetCurrFlyFrm() == aChild.GetSwFrm());
        }
        else
        {
            bRet = pFEShell->IsObjSelected( *aChild.GetSdrObject() );
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
        ::std::list< SwFrmOrObj > aChildren;
        rContext.GetChildren( aChildren );

        ::std::list< SwFrmOrObj >::const_iterator aIter = aChildren.begin();
        ::std::list< SwFrmOrObj >::const_iterator aEndIter = aChildren.end();
        while( aIter != aEndIter )
        {
            const SwFrmOrObj& rChild = *aIter;
            const SdrObject *pObj = rChild.GetSdrObject();
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
        const SwFlyFrm *pFlyFrm = pFEShell->GetCurrFlyFrm();
        if( pFlyFrm )
        {
            if( rContext.GetParent(pFlyFrm, rContext.IsInPagePreview()) ==
                rContext.GetFrm() )
            {
                nCount = 1;
            }
        }
        else
        {
            sal_uInt16 nSelObjs = pFEShell->IsObjSelected();
            if( nSelObjs > 0 )
            {
                ::std::list< SwFrmOrObj > aChildren;
                rContext.GetChildren( aChildren );

                ::std::list< SwFrmOrObj >::const_iterator aIter =
                    aChildren.begin();
                ::std::list< SwFrmOrObj >::const_iterator aEndIter =
                    aChildren.end();
                while( aIter != aEndIter && nCount < nSelObjs )
                {
                    const SwFrmOrObj& rChild = *aIter;
                    if( rChild.GetSdrObject() && !rChild.GetSwFrm() &&
                        rContext.GetParent(rChild, rContext.IsInPagePreview())
                           == rContext.GetFrm() &&
                         pFEShell->IsObjSelected( *rChild.GetSdrObject() ) )
                    {
                        nCount++;
                    }
                    ++aIter;
                }
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

    SwFrmOrObj aChild;
    const SwFlyFrm *pFlyFrm = pFEShell->GetCurrFlyFrm();
    if( pFlyFrm )
    {
        if( 0 == nSelectedChildIndex &&
            rContext.GetParent(pFlyFrm, rContext.IsInPagePreview()) ==
                rContext.GetFrm() )
        {
            aChild = pFlyFrm;
        }
    }
    else
    {
        sal_uInt16 nSelObjs = pFEShell->IsObjSelected();
        if( 0 == nSelObjs || nSelectedChildIndex >= nSelObjs )
            throwIndexOutOfBoundsException();

        ::std::list< SwFrmOrObj > aChildren;
        rContext.GetChildren( aChildren );

        ::std::list< SwFrmOrObj >::const_iterator aIter = aChildren.begin();
        ::std::list< SwFrmOrObj >::const_iterator aEndIter = aChildren.end();
        while( aIter != aEndIter && !aChild.IsValid() )
        {
            const SwFrmOrObj& rChild = *aIter;
            if( rChild.GetSdrObject() && !rChild.GetSwFrm() &&
                rContext.GetParent(rChild, rContext.IsInPagePreview()) ==
                    rContext.GetFrm() &&
                pFEShell->IsObjSelected( *rChild.GetSdrObject() ) )
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
    else
    {
        ::vos::ORef < ::accessibility::AccessibleShape > xChildImpl(
                rContext.GetMap()->GetContextImpl( aChild.GetSdrObject(),
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
        nChildIndex >= rContext.GetChildCount() )
        throwIndexOutOfBoundsException();
}
