/*************************************************************************
 *
 *  $RCSfile: svxrectctaccessiblecontext.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:35 $
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


#include "svxrectctaccessiblecontext.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#include "dialogs.hrc"
#include "accessibility.hrc"
#include "dlgctrl.hxx"

#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif


using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;


#define MAX_NUM_OF_CHILDS   9
#define NOCHILDSELECTED     -1


DBG_NAME( SvxRectCtlAccessibleContext )


//=====  internal  ============================================================

namespace
{
    struct ChildIndexToPointData
    {
        short       nResIdName;
        short       nResIdDescr;
        RECT_POINT  ePoint;
    };
}


static const ChildIndexToPointData* IndexToPoint( long nIndex, sal_Bool bAngleControl )
{
    DBG_ASSERT( nIndex < ( bAngleControl? 8 : 9 ) && nIndex >= 0, "-IndexToPoint(): invalid child index! You have been warned..." );

    // angles are counted reverse counter clock wise
    static const ChildIndexToPointData  pAngleData[] =
    {                                                   // index
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_A000,   RID_SVXSTR_RECTCTL_ACC_CHLD_A000,   RP_RM },    //  0
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_A045,   RID_SVXSTR_RECTCTL_ACC_CHLD_A045,   RP_RT },    //  1
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_A090,   RID_SVXSTR_RECTCTL_ACC_CHLD_A090,   RP_MT },    //  2
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_A135,   RID_SVXSTR_RECTCTL_ACC_CHLD_A135,   RP_LT },    //  3
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_A180,   RID_SVXSTR_RECTCTL_ACC_CHLD_A180,   RP_LM },    //  4
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_A225,   RID_SVXSTR_RECTCTL_ACC_CHLD_A225,   RP_LB },    //  5
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_A270,   RID_SVXSTR_RECTCTL_ACC_CHLD_A270,   RP_MB },    //  6
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_A315,   RID_SVXSTR_RECTCTL_ACC_CHLD_A315,   RP_RB }     //  7
    };

    // corners are counted from left to right and top to bottom
    static const ChildIndexToPointData  pCornerData[] =
    {                                                                   // index
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_LT, RID_SVXSTR_RECTCTL_ACC_CHLD_LT, RP_LT },    //  0
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_MT, RID_SVXSTR_RECTCTL_ACC_CHLD_MT, RP_MT },    //  1
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_RT, RID_SVXSTR_RECTCTL_ACC_CHLD_RT, RP_RT },    //  2
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_LM, RID_SVXSTR_RECTCTL_ACC_CHLD_LM, RP_LM },    //  3
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_MM, RID_SVXSTR_RECTCTL_ACC_CHLD_MM, RP_MM },    //  4
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_RM, RID_SVXSTR_RECTCTL_ACC_CHLD_RM, RP_RM },    //  5
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_LB, RID_SVXSTR_RECTCTL_ACC_CHLD_LB, RP_LB },    //  6
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_MB, RID_SVXSTR_RECTCTL_ACC_CHLD_MB, RP_MB },    //  7
        {   RID_SVXSTR_RECTCTL_ACC_CHLD_RB, RID_SVXSTR_RECTCTL_ACC_CHLD_RB, RP_RB }     //  8
    };

    return ( bAngleControl? pAngleData : pCornerData ) + nIndex;
}


static long PointToIndex( RECT_POINT ePoint, sal_Bool bAngleControl )
{
    long    nRet;
    if( bAngleControl )
    {   // angle control
        // angles are counted reverse counter clock wise
        switch( ePoint )
        {
            case RP_LT: nRet = 3;               break;
            case RP_MT: nRet = 2;               break;
            case RP_RT: nRet = 1;               break;
            case RP_LM: nRet = 4;               break;
            case RP_MM: nRet = NOCHILDSELECTED; break;
            case RP_RM: nRet = 0;               break;
            case RP_LB: nRet = 5;               break;
            case RP_MB: nRet = 6;               break;
            case RP_RB: nRet = 7;               break;
        }
    }
    else
    {   // corner control
        // corners are counted from left to right and top to bottom
        DBG_ASSERT( RP_LT == 0 && RP_MT == 1 && RP_RT == 2 && RP_LM == 3 && RP_MM == 4 && RP_RM == 5 &&
                    RP_LB == 6 && RP_MB == 7 && RP_RB == 8, "*PointToIndex(): unexpected enum value!" );

        nRet = ( long ) ePoint;
    }

    return nRet;
}


SvxRectCtlAccessibleContext::SvxRectCtlAccessibleContext(
    const Reference< XAccessible >&     rxParent,
    SvxRectCtl&                         rRepr,
    const OUString*                     pName,
    const OUString*                     pDesc ) :

    SvxRectCtlAccessibleContext_Base( m_aMutex ),
    mxParent( rxParent ),
    mpRepr( &rRepr ),
    mnSelectedChild( NOCHILDSELECTED ),
    mbAngleMode( rRepr.GetNumOfChilds() == 8 ),
    mnClientId( 0 )
{
    DBG_CTOR( SvxRectCtlAccessibleContext, NULL );

    if( pName )
        msName = *pName;
    else
    {
        ::vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
        msName = SVX_RESSTR( mbAngleMode? RID_SVXSTR_RECTCTL_ACC_ANGL_NAME : RID_SVXSTR_RECTCTL_ACC_CORN_NAME );
    }

    if( pDesc )
        msDescription = *pDesc;
    else
    {
        ::vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
        msDescription = SVX_RESSTR( mbAngleMode? RID_SVXSTR_RECTCTL_ACC_ANGL_DESCR : RID_SVXSTR_RECTCTL_ACC_CORN_DESCR );
    }

    mpChilds = new SvxRectCtlChildAccessibleContext*[ MAX_NUM_OF_CHILDS ];

    SvxRectCtlChildAccessibleContext**  p = mpChilds;
    for( int i = MAX_NUM_OF_CHILDS ; i ; --i, ++p )
        *p = NULL;
}


SvxRectCtlAccessibleContext::~SvxRectCtlAccessibleContext()
{
    DBG_DTOR( SvxRectCtlAccessibleContext, NULL );

    if( IsAlive() )
    {
        osl_incrementInterlockedCount( &m_refCount );
        dispose();      // set mpRepr = NULL & release all childs
    }
}

//=====  XAccessible  =========================================================

Reference< XAccessibleContext > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleContext( void ) throw( RuntimeException )
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL SvxRectCtlAccessibleContext::contains( const awt::Point& rPoint ) throw( RuntimeException )
{
    // no guard -> done in getBounds()
//  return GetBoundingBox().IsInside( VCLPoint( rPoint ) );
    return Rectangle( Point( 0, 0 ), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

Reference< XAccessible > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleAt( const awt::Point& rPoint ) throw( RuntimeException )
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    Reference< XAccessible >    xRet;

    long                        nChild = PointToIndex( mpRepr->GetApproxRPFromPixPt( rPoint ), mbAngleMode );

    if( nChild != NOCHILDSELECTED )
        xRet = getAccessibleChild( nChild );

    return xRet;
}

awt::Rectangle SAL_CALL SvxRectCtlAccessibleContext::getBounds() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    return AWTRectangle( GetBoundingBox() );
}

awt::Point SAL_CALL SvxRectCtlAccessibleContext::getLocation() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    return AWTPoint( GetBoundingBox().TopLeft() );
}

awt::Point SAL_CALL SvxRectCtlAccessibleContext::getLocationOnScreen() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBoxOnScreen()
    return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL SvxRectCtlAccessibleContext::getSize() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    return AWTSize( GetBoundingBox().GetSize() );
}

sal_Bool SAL_CALL SvxRectCtlAccessibleContext::isShowing() throw( RuntimeException )
{
    return sal_True;
}

sal_Bool SAL_CALL SvxRectCtlAccessibleContext::isVisible() throw( RuntimeException )
{
    ::osl::MutexGuard           aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return mpRepr->IsVisible();
}

sal_Bool SAL_CALL SvxRectCtlAccessibleContext::isFocusTraversable() throw( RuntimeException )
{
    return sal_True;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL SvxRectCtlAccessibleContext::getAccessibleChildCount( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return mpRepr->GetNumOfChilds();
}

Reference< XAccessible > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleChild( sal_Int32 nIndex )
    throw( RuntimeException, lang::IndexOutOfBoundsException )
{
    checkChildIndex( nIndex );

    Reference< XAccessible >    xChild = mpChilds[ nIndex ];
    if( !xChild.is() )
    {
        ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );

        ::osl::MutexGuard   aGuard( m_aMutex );

        ThrowExceptionIfNotAlive();

        xChild = mpChilds[ nIndex ];

        if( !xChild.is() )
        {
            const ChildIndexToPointData*    p = IndexToPoint( nIndex, mbAngleMode );
            UniString       tmp = SVX_RESSTR( p->nResIdName );
            OUString        aName( tmp );
                        tmp = SVX_RESSTR( p->nResIdDescr );
            OUString        aDescr( tmp );

            Rectangle       aFocusRect( mpRepr->CalculateFocusRectangle( p->ePoint ) );

            Rectangle       aBoundingBoxOnScreen( mpRepr->OutputToScreenPixel( aFocusRect.TopLeft() ), aFocusRect.GetSize() );

            SvxRectCtlChildAccessibleContext*   pChild = new SvxRectCtlChildAccessibleContext(
                                                    this, *mpRepr, aName, aDescr, aFocusRect, nIndex );
            xChild = mpChilds[ nIndex ] = pChild;
            pChild->acquire();

            // set actual state
            if( mnSelectedChild == nIndex )
                pChild->setStateChecked( sal_True );
        }
    }

    return xChild;
}

Reference< XAccessible > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleParent( void ) throw( RuntimeException )
{
    return mxParent;
}

sal_Int32 SAL_CALL SvxRectCtlAccessibleContext::getAccessibleIndexInParent( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    //  Use a simple but slow solution for now.  Optimize later.

    //  Iterate over all the parent's children and search for this object.
    if( mxParent.is() )
    {
        Reference< XAccessibleContext >     xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            sal_Int32                       nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int32 i = 0 ; i < nChildCount ; ++i )
            {
                Reference< XAccessible >    xChild( xParentContext->getAccessibleChild( i ) );
                if( xChild.get() == ( XAccessible* ) this )
                    return i;
            }
        }
   }

   //   Return -1 to indicate that this object's parent does not know about the
   //   object.
   return -1;
}

sal_Int16 SAL_CALL SvxRectCtlAccessibleContext::getAccessibleRole( void ) throw( RuntimeException )
{
    return AccessibleRole::PANEL;
}

OUString SAL_CALL SvxRectCtlAccessibleContext::getAccessibleDescription( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msDescription;
}

OUString SAL_CALL SvxRectCtlAccessibleContext::getAccessibleName( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return msName;
}

/** Return empty reference to indicate that the relation set is not
    supported.
*/
Reference< XAccessibleRelationSet > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleRelationSet( void ) throw( RuntimeException )
{
    return Reference< XAccessibleRelationSet >();
}

Reference< XAccessibleStateSet > SAL_CALL SvxRectCtlAccessibleContext::getAccessibleStateSet( void ) throw( RuntimeException )
{
    ::osl::MutexGuard                       aGuard( m_aMutex );
    utl::AccessibleStateSetHelper*          pStateSetHelper = new utl::AccessibleStateSetHelper;

    if( IsAlive() )
    {
        // pStateSetHelper->AddState( AccessibleStateType::ENABLED );
        pStateSetHelper->AddState( AccessibleStateType::FOCUSABLE );
        if( mpRepr->HasFocus() )
            pStateSetHelper->AddState( AccessibleStateType::FOCUSED );
        pStateSetHelper->AddState( AccessibleStateType::OPAQUE );

        if( isShowing() )
            pStateSetHelper->AddState( AccessibleStateType::SHOWING );

        if( isVisible() )
            pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}

lang::Locale SAL_CALL SvxRectCtlAccessibleContext::getLocale( void ) throw( IllegalAccessibleComponentStateException, RuntimeException )
{
    ::osl::MutexGuard                           aGuard( m_aMutex );
    if( mxParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }

    //  No parent.  Therefore throw exception to indicate this cluelessness.
    throw IllegalAccessibleComponentStateException();
}

void SAL_CALL SvxRectCtlAccessibleContext::addEventListener( const Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( m_aMutex );
        if (!mnClientId)
            mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}

void SAL_CALL SvxRectCtlAccessibleContext::removeEventListener( const Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
            mnClientId = 0;
        }
    }
}

void SAL_CALL SvxRectCtlAccessibleContext::addFocusListener( const Reference< awt::XFocusListener >& xListener )
    throw( RuntimeException )
{
    if( xListener.is() )
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        ThrowExceptionIfNotAlive();

        Reference< awt::XWindow >   xWindow = VCLUnoHelper::GetInterface( mpRepr );
        if( xWindow.is() )
            xWindow->addFocusListener( xListener );
    }
}

void SAL_CALL SvxRectCtlAccessibleContext::removeFocusListener( const Reference< awt::XFocusListener >& xListener )
    throw (RuntimeException)
{
    if( xListener.is() )
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        ThrowExceptionIfNotAlive();

        Reference< awt::XWindow >   xWindow = VCLUnoHelper::GetInterface( mpRepr );
        if( xWindow.is() )
            xWindow->removeFocusListener( xListener );
    }
}

void SAL_CALL SvxRectCtlAccessibleContext::grabFocus() throw( RuntimeException )
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    mpRepr->GrabFocus();
}

Any SAL_CALL SvxRectCtlAccessibleContext::getAccessibleKeyBinding() throw( RuntimeException )
{
    // here is no implementation, because here are no KeyBindings for every object
    return Any();
}

sal_Int32 SvxRectCtlAccessibleContext::getForeground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();

    return mpRepr->GetControlForeground().GetColor();
}
sal_Int32 SvxRectCtlAccessibleContext::getBackground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();

    return mpRepr->GetControlBackground().GetColor();
}

//=====  XServiceInfo  ========================================================

OUString SAL_CALL SvxRectCtlAccessibleContext::getImplementationName( void ) throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.ui.SvxRectCtlAccessibleContext" ) );
}

sal_Bool SAL_CALL SvxRectCtlAccessibleContext::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    Sequence< OUString >    aSupportedServices( getSupportedServiceNames() );
    int                     nLength = aSupportedServices.getLength();
    const OUString*         pStr = aSupportedServices.getConstArray();

    for( int i = nLength ; i ; --i, ++pStr )
    {
        if( sServiceName == *pStr )
            return sal_True;
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL SvxRectCtlAccessibleContext::getSupportedServiceNames( void ) throw( RuntimeException )
{
    const OUString sServiceName( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.accessibility.AccessibleContext" ) );
    return Sequence< OUString >( &sServiceName, 1 );
}

//=====  XTypeProvider  =======================================================

Sequence< sal_Int8 > SAL_CALL SvxRectCtlAccessibleContext::getImplementationId( void ) throw( RuntimeException )
{
    return getUniqueId();
}

//=====  XAccessibleSelection =============================================

void SAL_CALL SvxRectCtlAccessibleContext::selectAccessibleChild( sal_Int32 nIndex ) throw( lang::IndexOutOfBoundsException, RuntimeException )
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );

    ::osl::MutexGuard   aGuard( m_aMutex );

    checkChildIndex( nIndex );

    ThrowExceptionIfNotAlive();

    const ChildIndexToPointData*    pData = IndexToPoint( nIndex, mbAngleMode );

    DBG_ASSERT( pData,
        "SvxRectCtlAccessibleContext::selectAccessibleChild(): this is an impossible state! Or at least should be..." );

    // this does all wich is needed, including the change of the child's state!
    mpRepr->SetActualRP( pData->ePoint );
}

sal_Bool SAL_CALL SvxRectCtlAccessibleContext::isAccessibleChildSelected( sal_Int32 nIndex ) throw( lang::IndexOutOfBoundsException, RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    checkChildIndex( nIndex );

    return nIndex == mnSelectedChild;
}

void SAL_CALL SvxRectCtlAccessibleContext::clearAccessibleSelection() throw( RuntimeException )
{
    DBG_ASSERT( sal_False, "SvxRectCtlAccessibleContext::clearAccessibleSelection() is not possible!" );
}

void SAL_CALL SvxRectCtlAccessibleContext::selectAllAccessible() throw( RuntimeException )
{
    // guard in selectAccessibleChild()!

    selectAccessibleChild( 0 );     // default per definition
}

sal_Int32 SAL_CALL SvxRectCtlAccessibleContext::getSelectedAccessibleChildCount() throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    return mnSelectedChild == NOCHILDSELECTED? 0 : 1;
}

Reference< XAccessible > SAL_CALL SvxRectCtlAccessibleContext::getSelectedAccessibleChild( sal_Int32 nIndex )
    throw( lang::IndexOutOfBoundsException, RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    checkChildIndexOnSelection( nIndex );

    return getAccessibleChild( mnSelectedChild );
}

void SAL_CALL SvxRectCtlAccessibleContext::deselectSelectedAccessibleChild( sal_Int32 nIndex ) throw( lang::IndexOutOfBoundsException, RuntimeException )
{
    OUString    aMessage( RTL_CONSTASCII_USTRINGPARAM( "deselectSelectedAccessibleChild is not possible in this context" ) );

    DBG_ASSERT( sal_False, "SvxRectCtlAccessibleContext::deselectSelectedAccessibleChild() is not possible!" );

    throw lang::IndexOutOfBoundsException( aMessage, *this );   // never possible
}

//=====  internals ========================================================

void SvxRectCtlAccessibleContext::checkChildIndex( long nIndex ) throw( lang::IndexOutOfBoundsException )
{
    if( nIndex < 0 || nIndex >= getAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();
}

void SvxRectCtlAccessibleContext::checkChildIndexOnSelection( long nIndex ) throw( lang::IndexOutOfBoundsException )
{
    if( nIndex || mnSelectedChild == NOCHILDSELECTED )
        // in our case only for the first (0) _selected_ child this is a valid request
        throw lang::IndexOutOfBoundsException();
}

void SvxRectCtlAccessibleContext::selectChild( long nNew )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    if( nNew != mnSelectedChild )
    {
        long    nNumOfChilds = getAccessibleChildCount();
        if( nNew < nNumOfChilds )
        {   // valid index
            SvxRectCtlChildAccessibleContext*   pChild;
            if( mnSelectedChild != NOCHILDSELECTED )
            {   // deselect old selected child if one is selected
                pChild = mpChilds[ mnSelectedChild ];
                if( pChild )
                    pChild->setStateChecked( sal_False );
            }

            // select new child
            mnSelectedChild = nNew;

            if( nNew != NOCHILDSELECTED )
            {
                pChild = mpChilds[ nNew ];
                if( pChild )
                    pChild->setStateChecked( sal_True );
            }
        }
        else
            mnSelectedChild = NOCHILDSELECTED;
    }
}

void SvxRectCtlAccessibleContext::selectChild( RECT_POINT eButton )
{
    // no guard -> is done in next selectChild
    selectChild( PointToIndex( eButton, mbAngleMode ) );
}

void SvxRectCtlAccessibleContext::setName( const OUString& rName )
{
    Any                     aPreVal, aPostVal;
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        aPreVal <<= msName;
        aPostVal <<= rName;

        msName = rName;
    }

    const Reference< XInterface >   xSource( *this );
    CommitChange( AccessibleEventObject( xSource, AccessibleEventId::ACCESSIBLE_NAME_EVENT, aPreVal, aPostVal ) );
}

void SvxRectCtlAccessibleContext::setDescription( const OUString& rDescr )
{
    Any                     aPreVal, aPostVal;
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        aPreVal <<= msDescription;
        aPostVal <<= rDescr;

        msDescription = rDescr;
    }

    const Reference< XInterface >   xSource( *this );
    CommitChange( AccessibleEventObject( xSource, AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT, aPreVal, aPostVal ) );
}

void SvxRectCtlAccessibleContext::CommitChange( const AccessibleEventObject& rEvent )
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, rEvent );
}

void SAL_CALL SvxRectCtlAccessibleContext::disposing()
{
    if( !rBHelper.bDisposed )
    {
        {
            ::osl::MutexGuard   aGuard( m_aMutex );
            mpRepr = NULL;      // object dies with representation

            SvxRectCtlChildAccessibleContext**  p = mpChilds;
            for( int i = MAX_NUM_OF_CHILDS ; i ; --i, ++p )
            {
                SvxRectCtlChildAccessibleContext*   pChild = *p;
                if( pChild )
                {
                    pChild->dispose();
                    pChild->release();
                    *p = NULL;
                }
            }

            delete[] mpChilds;
            mpChilds = NULL;
        }

        {
            ::osl::MutexGuard   aGuard( m_aMutex );

            // Send a disposing to all listeners.
            if ( mnClientId )
            {
                comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
                mnClientId =  0;
            }

            mxParent = Reference< XAccessible >();
        }
    }
}

Rectangle SvxRectCtlAccessibleContext::GetBoundingBoxOnScreen( void ) throw( RuntimeException )
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return Rectangle( mpRepr->GetParent()->OutputToScreenPixel( mpRepr->GetPosPixel() ), mpRepr->GetSizePixel() );
}

Rectangle SvxRectCtlAccessibleContext::GetBoundingBox( void ) throw( RuntimeException )
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();

    return Rectangle( mpRepr->GetPosPixel(), mpRepr->GetSizePixel() );
}

Sequence< sal_Int8 > SvxRectCtlAccessibleContext::getUniqueId( void )
{
    static OImplementationId*   pId = 0;
    if( !pId )
    {
        MutexGuard                      aGuard( Mutex::getGlobalMutex() );
        if( !pId)
        {
            static OImplementationId    aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

void SvxRectCtlAccessibleContext::ThrowExceptionIfNotAlive( void ) throw( lang::DisposedException )
{
    if( IsNotAlive() )
        throw lang::DisposedException();
}

// -------------------------------------------------------------------------------------------------


DBG_NAME( SvxRectCtlChildAccessibleContext )


SvxRectCtlChildAccessibleContext::SvxRectCtlChildAccessibleContext(
    const Reference<XAccessible>&   rxParent,
    const Window&                       rParentWindow,
    const ::rtl::OUString&              rName,
    const ::rtl::OUString&              rDescription,
    const Rectangle&                    rBoundingBox,
    long                                nIndexInParent ) :

    SvxRectCtlChildAccessibleContext_Base( maMutex ),
    mxParent(rxParent),
    mrParentWindow( rParentWindow ),
    mpBoundingBox( new Rectangle( rBoundingBox ) ),
    msName( rName ),
    msDescription( rDescription ),
    mbIsChecked( sal_False ),
    mnIndexInParent( nIndexInParent ),
    mnClientId( 0 )
{
    DBG_CTOR( SvxRectCtlChildAccessibleContext, NULL );
}


SvxRectCtlChildAccessibleContext::~SvxRectCtlChildAccessibleContext()
{
    DBG_DTOR( SvxRectCtlChildAccessibleContext, NULL );

    if( IsAlive() )
    {
        osl_incrementInterlockedCount( &m_refCount );
        dispose();      // set mpRepr = NULL & release all childs
    }
}

//=====  XAccessible  =========================================================

Reference< XAccessibleContext> SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleContext( void ) throw( RuntimeException )
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL SvxRectCtlChildAccessibleContext::contains( const awt::Point& rPoint ) throw( RuntimeException )
{
    // no guard -> done in getBounds()
//  return GetBoundingBox().IsInside( VCLPoint( rPoint ) );
    return Rectangle( Point( 0, 0 ), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

Reference< XAccessible > SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleAt( const awt::Point& rPoint ) throw( RuntimeException )
{
    return Reference< XAccessible >();
}

awt::Rectangle SAL_CALL SvxRectCtlChildAccessibleContext::getBounds() throw( RuntimeException )
{
    // no guard -> done in getBoundingBox()
    return AWTRectangle( GetBoundingBox() );
}

awt::Point SAL_CALL SvxRectCtlChildAccessibleContext::getLocation() throw( RuntimeException )
{
    // no guard -> done in getBoundingBox()
    return AWTPoint( GetBoundingBox().TopLeft() );
}

awt::Point SAL_CALL SvxRectCtlChildAccessibleContext::getLocationOnScreen() throw( RuntimeException )
{
    // no guard -> done in getBoundingBoxOnScreen()
    return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL SvxRectCtlChildAccessibleContext::getSize() throw( RuntimeException )
{
    // no guard -> done in getBoundingBox()
    return AWTSize( GetBoundingBox().GetSize() );
}

sal_Bool SAL_CALL SvxRectCtlChildAccessibleContext::isShowing() throw( RuntimeException )
{
    return sal_True;
}

sal_Bool SAL_CALL SvxRectCtlChildAccessibleContext::isVisible() throw( RuntimeException )
{
    ::osl::MutexGuard                   aGuard( maMutex );

    ThrowExceptionIfNotAlive();

    return mxParent.is()? ( static_cast< SvxRectCtlAccessibleContext* >( mxParent.get() ) )->isVisible() : sal_False;
}

sal_Bool SAL_CALL SvxRectCtlChildAccessibleContext::isFocusTraversable() throw( RuntimeException )
{
    return sal_False;
}

void SAL_CALL SvxRectCtlChildAccessibleContext::addFocusListener( const Reference< awt::XFocusListener >& xListener )
    throw( RuntimeException )
{
}

void SAL_CALL SvxRectCtlChildAccessibleContext::removeFocusListener( const Reference< awt::XFocusListener >& xListener )
    throw (RuntimeException)
{
}

void SAL_CALL SvxRectCtlChildAccessibleContext::grabFocus() throw( RuntimeException )
{
}

Any SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleKeyBinding() throw( RuntimeException )
{
    // here is no implementation, because here are no KeyBindings for every object
    return Any();
}
sal_Int32 SvxRectCtlChildAccessibleContext::getForeground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( maMutex );
    ThrowExceptionIfNotAlive();
    return mrParentWindow.GetControlForeground().GetColor();
}
sal_Int32 SvxRectCtlChildAccessibleContext::getBackground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard   aGuard( maMutex );

    ThrowExceptionIfNotAlive();
    return mrParentWindow.GetControlBackground().GetColor();
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleChildCount( void ) throw( RuntimeException )
{
    return 0;
}

Reference< XAccessible > SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleChild( sal_Int32 nIndex ) throw ( RuntimeException, lang::IndexOutOfBoundsException )
{
    throw lang::IndexOutOfBoundsException();
}

Reference< XAccessible > SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleParent( void ) throw( RuntimeException )
{
    return mxParent;
}

sal_Int32 SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleIndexInParent( void ) throw( RuntimeException )
{
   return mnIndexInParent;
}

sal_Int16 SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleRole( void ) throw( RuntimeException )
{
    return AccessibleRole::RADIOBUTTON;
}

OUString SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleDescription( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( maMutex );
    return msDescription;
}

OUString SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleName( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( maMutex );
    return msName;
}

/** Return empty reference to indicate that the relation set is not
    supported.
*/
Reference<XAccessibleRelationSet> SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleRelationSet( void ) throw( RuntimeException )
{
    return Reference< XAccessibleRelationSet >();
}

Reference< XAccessibleStateSet > SAL_CALL SvxRectCtlChildAccessibleContext::getAccessibleStateSet( void ) throw( RuntimeException )
{
    ::osl::MutexGuard                       aGuard( maMutex );
    utl::AccessibleStateSetHelper*          pStateSetHelper = new utl::AccessibleStateSetHelper;

    if( IsAlive() )
    {
        if( mbIsChecked )
        {
            pStateSetHelper->AddState( AccessibleStateType::CHECKED );
//          pStateSetHelper->AddState( AccessibleStateType::SELECTED );
        }

        pStateSetHelper->AddState( AccessibleStateType::ENABLED );
        pStateSetHelper->AddState( AccessibleStateType::OPAQUE );
        pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
        pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}

lang::Locale SAL_CALL SvxRectCtlChildAccessibleContext::getLocale( void ) throw( IllegalAccessibleComponentStateException, RuntimeException )
{
    ::osl::MutexGuard                       aGuard( maMutex );
    if( mxParent.is() )
    {
        Reference< XAccessibleContext >     xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }

    //  No locale and no parent.  Therefore throw exception to indicate this
    //  cluelessness.
    throw IllegalAccessibleComponentStateException();
}

void SAL_CALL SvxRectCtlChildAccessibleContext::addEventListener( const Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( maMutex );
        if (!mnClientId)
            mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}




void SAL_CALL SvxRectCtlChildAccessibleContext::removeEventListener( const Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( maMutex );

        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
            mnClientId = 0;
        }
    }
}

//=====  XAccessibleValue  ================================================

Any SAL_CALL SvxRectCtlChildAccessibleContext::getCurrentValue() throw( RuntimeException )
{
    ThrowExceptionIfNotAlive();

    Any aRet;
    aRet <<= ( mbIsChecked? 1.0 : 0.0 );
    return aRet;
}

sal_Bool SAL_CALL SvxRectCtlChildAccessibleContext::setCurrentValue( const Any& aNumber ) throw( RuntimeException )
{
    return sal_False;
}

Any SAL_CALL SvxRectCtlChildAccessibleContext::getMaximumValue() throw( RuntimeException )
{
    Any aRet;
    aRet <<= 1.0;
    return aRet;
}

Any SAL_CALL SvxRectCtlChildAccessibleContext::getMinimumValue() throw( RuntimeException )
{
    Any aRet;
    aRet <<= 0.0;
    return aRet;
}

//=====  XServiceInfo  ========================================================

OUString SAL_CALL SvxRectCtlChildAccessibleContext::getImplementationName( void ) throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.ui.SvxRectCtlChildAccessibleContext" ) );
}

sal_Bool SAL_CALL SvxRectCtlChildAccessibleContext::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    ::osl::MutexGuard   aGuard( maMutex );
    Sequence< OUString >    aSupportedServices ( getSupportedServiceNames() );
    int                         nLength = aSupportedServices.getLength();
    for( int i = 0 ; i < aSupportedServices.getLength() ; ++i )
    {
        if( sServiceName == aSupportedServices[ i ] )
            return sal_True;
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL SvxRectCtlChildAccessibleContext::getSupportedServiceNames( void ) throw( RuntimeException )
{
    const OUString sServiceName (RTL_CONSTASCII_USTRINGPARAM ("drafts.com.sun.star.accessibility.AccessibleContext"));
    return Sequence< OUString >( &sServiceName, 1 );
}

//=====  XTypeProvider  =======================================================

Sequence< sal_Int8 > SAL_CALL SvxRectCtlChildAccessibleContext::getImplementationId( void ) throw( RuntimeException )
{
    static OImplementationId*   pId = 0;
    if( !pId )
    {
        MutexGuard                      aGuard( Mutex::getGlobalMutex() );
        if( !pId)
        {
            static OImplementationId    aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

//=====  internal  ============================================================

void SvxRectCtlChildAccessibleContext::CommitChange( const AccessibleEventObject& rEvent )
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, rEvent );
}

void SAL_CALL SvxRectCtlChildAccessibleContext::disposing()
{
    if( !rBHelper.bDisposed )
    {
        ::osl::MutexGuard   aGuard( maMutex );

        // Send a disposing to all listeners.
        if ( mnClientId )
        {
            comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
            mnClientId =  0;
        }

        mxParent = Reference< XAccessible >();

        delete mpBoundingBox;
    }
}

void SvxRectCtlChildAccessibleContext::ThrowExceptionIfNotAlive( void ) throw( lang::DisposedException )
{
    if( IsNotAlive() )
        throw lang::DisposedException();
}

Rectangle SvxRectCtlChildAccessibleContext::GetBoundingBoxOnScreen( void ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( maMutex );

    // no ThrowExceptionIfNotAlive() because its done in GetBoundingBox()
    Rectangle           aRect( GetBoundingBox() );

    return Rectangle( mrParentWindow.OutputToScreenPixel( aRect.TopLeft() ), aRect.GetSize() );
}

Rectangle SvxRectCtlChildAccessibleContext::GetBoundingBox( void ) throw( RuntimeException )
{
    // no guard neccessary, because no one changes mpBoundingBox after creating it
    ThrowExceptionIfNotAlive();

    return *mpBoundingBox;
}

void SvxRectCtlChildAccessibleContext::setStateChecked( sal_Bool bChecked )
{
    if( mbIsChecked != bChecked )
    {
        mbIsChecked = bChecked;

        const Reference< XInterface >   xSource( *this );

        Any                             aOld;
        Any                             aNew;
        Any&                            rMod = bChecked? aNew : aOld;

        rMod <<= AccessibleStateType::CHECKED;

        CommitChange( AccessibleEventObject( xSource, AccessibleEventId::ACCESSIBLE_STATE_EVENT, aNew, aOld ) );
    }
}

