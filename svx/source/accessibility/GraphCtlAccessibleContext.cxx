/*************************************************************************
 *
 *  $RCSfile: GraphCtlAccessibleContext.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:45:52 $
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

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
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
#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif

//===== local includes ========================================================

#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#include "ShapeTypeHandler.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#include "AccessibleShapeInfo.hxx"
#endif
#ifndef _SVXGRAPHACCESSIBLECONTEXT_HXX
#include "GraphCtlAccessibleContext.hxx"
#endif
#ifndef _GRAPHCTL_HXX
#include "graphctl.hxx"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif
#ifndef _SVX_ACCESSIBILITY_HRC
#include "accessibility.hrc"
#endif
#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif
#ifndef SVX_UNOMOD_HXX
#include "unomod.hxx"
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

//=====  namespaces ===========================================================

using namespace ::vos;
using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;
using namespace ::accessibility;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

/** initialize this component and set default values */
SvxGraphCtrlAccessibleContext::SvxGraphCtrlAccessibleContext(
    const Reference< XAccessible >& rxParent,
    GraphCtrl&                              rRepr,
    const OUString*                         pName,
    const OUString*                         pDesc ) :

    SvxGraphCtrlAccessibleContext_Base( m_aMutex ),
    mxParent( rxParent ),
    mpControl( &rRepr ),
    mbDisposed( sal_False ),
    mnClientId( 0 ),
    mpModel (NULL),
    mpPage (NULL),
    mpView (NULL)
{
    if (mpControl != NULL)
    {
        mpModel = mpControl->GetSdrModel();
        if (mpModel != NULL)
            mpPage = (SdrPage*)mpModel->GetPage( 0 );
        mpView = mpControl->GetSdrView();

        if( mpModel == NULL || mpPage == NULL || mpView == NULL )
        {
            mbDisposed = true;
            // Set all the pointers to NULL just in case they are used as
            // a disposed flag.
            mpModel = NULL;
            mpPage = NULL;
            mpView = NULL;
        }
    }

    if( pName )
    {
        msName = *pName;
    }
    else
    {
        ::vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
        msName = SVX_RESSTR( RID_SVXSTR_GRAPHCTRL_ACC_NAME );
    }

    if( pDesc )
    {
        msDescription = *pDesc;
    }
    else
    {
        ::vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
        msDescription = SVX_RESSTR( RID_SVXSTR_GRAPHCTRL_ACC_DESCRIPTION );
    }

    maTreeInfo.SetSdrView( mpView );
    maTreeInfo.SetWindow( mpControl );
    maTreeInfo.SetViewForwarder( const_cast<SvxGraphCtrlAccessibleContext*>(this) );
}

//-----------------------------------------------------------------------------

/** on destruction, this component is disposed and all dispose listeners
    are called, except if this component was already disposed */
SvxGraphCtrlAccessibleContext::~SvxGraphCtrlAccessibleContext()
{
    disposing();
}

//-----------------------------------------------------------------------------

/** returns the XAccessible interface for a given SdrObject.
    Multiple calls for the same SdrObject return the same XAccessible.
*/
Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessible( const SdrObject* pObj )
{
    Reference<XAccessible> xAccessibleShape;

    if( pObj )
    {
        // see if we already created an XAccessible for the given SdrObject
        ShapesMapType::iterator iter = mxShapes.find( pObj );

        if( iter != mxShapes.end() )
        {
            // if we already have one, return it
            xAccessibleShape = (*iter).second;
        }
        else
        {
            // create a new one and remember in our internal map
            Reference< XShape > xShape( Reference< XShape >::query( (const_cast<SdrObject*>(pObj))->getUnoShape() ) );

            AccessibleShapeInfo aShapeInfo (xShape,mxParent);
            // Create accessible object that corresponds to the descriptor's shape.
            AccessibleShape* pAcc = ShapeTypeHandler::Instance().CreateAccessibleObject(
                aShapeInfo, maTreeInfo);
            xAccessibleShape = pAcc;
            if (pAcc != NULL)
            {
                pAcc->acquire();
                // Now that we acquired the new accessible shape we can
                // safely call its Init() method.
                pAcc->Init ();
            }
            mxShapes[pObj] = pAcc;

            // Create event and inform listeners of the object creation.
            CommitChange( AccessibleEventId::CHILD, makeAny( xAccessibleShape ), makeAny( Reference<XAccessible>() ) );
        }
    }

    return xAccessibleShape;
}

//=====  XAccessible  =========================================================

Reference< XAccessibleContext > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleContext( void ) throw( RuntimeException )
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL SvxGraphCtrlAccessibleContext::containsPoint( const awt::Point& rPoint ) throw( RuntimeException )
{
    // no guard -> done in getSize()
    awt::Size aSize (getSize());
    return (rPoint.X >= 0)
        && (rPoint.X < aSize.Width)
        && (rPoint.Y >= 0)
        && (rPoint.Y < aSize.Height);
}

//-----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleAtPoint( const awt::Point& rPoint ) throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    Reference< XAccessible > xAccessible;

    if( mpControl )
    {
        Point aPnt( rPoint.X, rPoint.Y );
        mpControl->PixelToLogic( aPnt );
        SdrObject * pObj = mpPage->CheckHit(aPnt, 1, NULL, false);
        if( pObj )
            xAccessible = getAccessible( pObj );
    }
    else
    {
        throw DisposedException();
    }

    return xAccessible;
}

//-----------------------------------------------------------------------------

awt::Rectangle SAL_CALL SvxGraphCtrlAccessibleContext::getBounds() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    Rectangle           aCoreBounds( GetBoundingBox() );
    awt::Rectangle      aBounds;
    aBounds.X = aCoreBounds.getX();
    aBounds.Y = aCoreBounds.getY();
    aBounds.Width = aCoreBounds.getWidth();
    aBounds.Height = aCoreBounds.getHeight();
    return aBounds;
}

//-----------------------------------------------------------------------------

awt::Point SAL_CALL SvxGraphCtrlAccessibleContext::getLocation() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    Rectangle   aRect( GetBoundingBox() );
    return awt::Point( aRect.getX(), aRect.getY() );
}

//-----------------------------------------------------------------------------

awt::Point SAL_CALL SvxGraphCtrlAccessibleContext::getLocationOnScreen() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBoxOnScreen()
    Rectangle   aRect( GetBoundingBoxOnScreen() );
    return awt::Point( aRect.getX(), aRect.getY() );
}

//-----------------------------------------------------------------------------

awt::Size SAL_CALL SvxGraphCtrlAccessibleContext::getSize() throw( RuntimeException )
{
    // no guard -> done in GetBoundingBox()
    Rectangle   aRect( GetBoundingBox() );
    return awt::Size( aRect.getWidth(), aRect.getHeight() );
}


//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleChildCount( void ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpPage )
        throw DisposedException();

    return mpPage->GetObjCount();
}

//-----------------------------------------------------------------------------

/** returns the SdrObject at index nIndex from the model of this graph */
SdrObject* SvxGraphCtrlAccessibleContext::getSdrObject( sal_Int32 nIndex )
    throw( RuntimeException, lang::IndexOutOfBoundsException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpPage )
        throw DisposedException();

    if( (nIndex < 0) || ( static_cast<sal_uInt32>(nIndex) >= mpPage->GetObjCount() ) )
        throw lang::IndexOutOfBoundsException();

    return mpPage->GetObj( nIndex );
}

//-----------------------------------------------------------------------------

/** sends an AccessibleEventObject to all added XAccessibleEventListeners */
void SvxGraphCtrlAccessibleContext::CommitChange (
    sal_Int16 nEventId,
    const uno::Any& rNewValue,
    const uno::Any& rOldValue)
{
    AccessibleEventObject aEvent (
        static_cast<uno::XWeak*>(this),
        nEventId,
        rNewValue,
        rOldValue);

    FireEvent (aEvent);
}

/** sends an AccessibleEventObject to all added XAccessibleEventListeners */
void SvxGraphCtrlAccessibleContext::FireEvent (const AccessibleEventObject& aEvent)
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, aEvent );
}

//-----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleChild( sal_Int32 nIndex )
    throw( RuntimeException, lang::IndexOutOfBoundsException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    return getAccessible( getSdrObject( nIndex ) );
}

//-----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleParent( void ) throw( RuntimeException )
{
    return mxParent;
}

//-----------------------------------------------------------------------------

sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleIndexInParent( void ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );
    //  Use a simple but slow solution for now.  Optimize later.

    //  Iterate over all the parent's children and search for this object.
    if( mxParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int32 i = 0 ; i < nChildCount ; ++i )
            {
                Reference< XAccessible > xChild( xParentContext->getAccessibleChild( i ) );
                if( xChild.is() )
                {
                    Reference< XAccessibleContext > xChildContext = xChild->getAccessibleContext();
                    if( xChildContext == ( XAccessibleContext* ) this )
                        return i;
                }
            }
        }
   }

   //   Return -1 to indicate that this object's parent does not know about the
   //   object.
   return -1;
}

//-----------------------------------------------------------------------------

sal_Int16 SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleRole( void ) throw( RuntimeException )
{
    return AccessibleRole::PANEL;
}

//-----------------------------------------------------------------------------

OUString SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleDescription( void ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );
    return msDescription;
}

//-----------------------------------------------------------------------------

OUString SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleName( void ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );
    return msName;
}

//-----------------------------------------------------------------------------

/** Return empty reference to indicate that the relation set is not
    supported.
*/
Reference< XAccessibleRelationSet > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleRelationSet( void ) throw( RuntimeException )
{
    return Reference< XAccessibleRelationSet >();
}

//-----------------------------------------------------------------------------

Reference< XAccessibleStateSet > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleStateSet( void ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;

    if ( rBHelper.bDisposed || mbDisposed )
    {
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
    }
    else
    {
        // pStateSetHelper->AddState( AccessibleStateType::ENABLED );
        pStateSetHelper->AddState( AccessibleStateType::FOCUSABLE );
        if( mpControl->HasFocus() )
            pStateSetHelper->AddState( AccessibleStateType::FOCUSED );
        pStateSetHelper->AddState( AccessibleStateType::OPAQUE );
        pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
    }

    return pStateSetHelper;
}

//-----------------------------------------------------------------------------

lang::Locale SAL_CALL SvxGraphCtrlAccessibleContext::getLocale( void ) throw( IllegalAccessibleComponentStateException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mxParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }

    //  No parent.  Therefore throw exception to indicate this cluelessness.
    throw IllegalAccessibleComponentStateException();
}

//=====  XAccessibleEventListener  ============================================

void SAL_CALL SvxGraphCtrlAccessibleContext::addEventListener( const Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException )
{
    if (xListener.is())
    {
        OGuard aGuard( Application::GetSolarMutex() );
        if (!mnClientId)
            mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}

//-----------------------------------------------------------------------------

void SAL_CALL SvxGraphCtrlAccessibleContext::removeEventListener( const Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException )
{
    if (xListener.is())
    {
        OGuard aGuard( Application::GetSolarMutex() );

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

//-----------------------------------------------------------------------------

void SAL_CALL SvxGraphCtrlAccessibleContext::addFocusListener( const Reference< awt::XFocusListener >& xListener )
    throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( xListener.is() )
    {
        Reference< ::com::sun::star::awt::XWindow > xWindow( VCLUnoHelper::GetInterface( mpControl ) );
        if( xWindow.is() )
            xWindow->addFocusListener( xListener );
    }
}

//-----------------------------------------------------------------------------

void SAL_CALL SvxGraphCtrlAccessibleContext::removeFocusListener( const Reference< awt::XFocusListener >& xListener )
    throw (RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( xListener.is() )
    {
        Reference< ::com::sun::star::awt::XWindow > xWindow = VCLUnoHelper::GetInterface( mpControl );
        if( xWindow.is() )
            xWindow->removeFocusListener( xListener );
    }
}

//-----------------------------------------------------------------------------

void SAL_CALL SvxGraphCtrlAccessibleContext::grabFocus() throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpControl )
        throw DisposedException();

    mpControl->GrabFocus();
}

//-----------------------------------------------------------------------------

Any SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleKeyBinding() throw( RuntimeException )
{
    // here is no implementation, because here are no KeyBindings for every object
    return Any();
}





sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getForeground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    svtools::ColorConfig aColorConfig;
    UINT32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}




sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getBackground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    UINT32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}


//=====  XServiceInfo  ========================================================

OUString SAL_CALL SvxGraphCtrlAccessibleContext::getImplementationName( void ) throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.ui.SvxGraphCtrlAccessibleContext" ) );
}

//-----------------------------------------------------------------------------

sal_Bool SAL_CALL SvxGraphCtrlAccessibleContext::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    Sequence< OUString >    aSupportedServices( getSupportedServiceNames() );
    int                     nLenght = aSupportedServices.getLength();

    for( int i = 0 ; i < nLenght ; ++i )
    {
        if( sServiceName == aSupportedServices[ i ] )
            return sal_True;
    }

    return sal_False;
}

//-----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL SvxGraphCtrlAccessibleContext::getSupportedServiceNames( void ) throw( RuntimeException )
{
    Sequence< OUString > aSNs( 3 );

    aSNs[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.accessibility.Accessible" ) );
    aSNs[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.accessibility.AccessibleContext" ) );
    aSNs[2] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.AccessibleGraphControl" ) );

    return aSNs;
}

//=====  XTypeProvider  =======================================================

Sequence<sal_Int8> SAL_CALL SvxGraphCtrlAccessibleContext::getImplementationId( void ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );
    return getUniqueId();
}

//=====  XServiceName  ========================================================

OUString SvxGraphCtrlAccessibleContext::getServiceName( void ) throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.accessibility.AccessibleContext" ) );
}

//=====  XAccessibleSelection =============================================

void SAL_CALL SvxGraphCtrlAccessibleContext::selectAccessibleChild( sal_Int32 nIndex ) throw( lang::IndexOutOfBoundsException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpView )
        throw DisposedException();

    SdrObject* pObj = getSdrObject( nIndex );

    if( pObj )
        mpView->MarkObj( pObj, mpView->GetPageViewPgNum(0));
}

//-----------------------------------------------------------------------------

sal_Bool SAL_CALL SvxGraphCtrlAccessibleContext::isAccessibleChildSelected( sal_Int32 nIndex ) throw( lang::IndexOutOfBoundsException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpView )
        throw DisposedException();

    return mpView->IsObjMarked( getSdrObject( nIndex ) );
}

//-----------------------------------------------------------------------------

void SAL_CALL SvxGraphCtrlAccessibleContext::clearAccessibleSelection() throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpView )
        throw DisposedException();

    mpView->UnmarkAllObj();
}

//-----------------------------------------------------------------------------

void SAL_CALL SvxGraphCtrlAccessibleContext::selectAllAccessibleChildren() throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpView )
        throw DisposedException();

    mpView->MarkAllObj();
}

//-----------------------------------------------------------------------------

sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getSelectedAccessibleChildCount() throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpView )
        throw DisposedException();

    const SdrMarkList& rList = mpView->GetMarkedObjectList();
    return rList.GetMarkCount();
}

//-----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getSelectedAccessibleChild( sal_Int32 nIndex )
    throw( lang::IndexOutOfBoundsException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    checkChildIndexOnSelection( nIndex );

    Reference< XAccessible > xAccessible;

    const SdrMarkList& rList = mpView->GetMarkedObjectList();
    SdrObject* pObj = rList.GetMark(nIndex)->GetObj();
    if( pObj )
        xAccessible = getAccessible( pObj );

    return xAccessible;
}

//-----------------------------------------------------------------------------

void SAL_CALL SvxGraphCtrlAccessibleContext::deselectAccessibleChild( sal_Int32 nIndex ) throw( lang::IndexOutOfBoundsException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    checkChildIndexOnSelection( nIndex );

    if( mpView )
    {
        const SdrMarkList& rList = mpView->GetMarkedObjectList();

        SdrObject* pObj = getSdrObject( nIndex );
        if( pObj )
        {
            SdrMarkList aRefList( rList );

            SdrPageView* pPV = mpView->GetPageViewPgNum(0);
            mpView->UnmarkAllObj( pPV );

            sal_uInt32 nCount = aRefList.GetMarkCount();
            sal_uInt32 nMark;
            for( nMark = 0; nMark < nCount; nMark++ )
            {
                if( aRefList.GetMark(nMark)->GetObj() != pObj )
                    mpView->MarkObj( aRefList.GetMark(nMark)->GetObj(), pPV );
            }
        }
    }
}

//=====  internals ========================================================

void SvxGraphCtrlAccessibleContext::checkChildIndex( long nIndex ) throw( lang::IndexOutOfBoundsException )
{
    if( nIndex < 0 || nIndex >= getAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();
}

//-----------------------------------------------------------------------------

void SvxGraphCtrlAccessibleContext::checkChildIndexOnSelection( long nIndex ) throw( lang::IndexOutOfBoundsException )
{
    if( nIndex < 0 || nIndex >= getSelectedAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();
}

//-----------------------------------------------------------------------------

void SvxGraphCtrlAccessibleContext::setName( const OUString& rName )
{
    OGuard aGuard( Application::GetSolarMutex() );

    msName = rName;
}

//-----------------------------------------------------------------------------

void SvxGraphCtrlAccessibleContext::setDescription( const OUString& rDescr )
{
    OGuard aGuard( Application::GetSolarMutex() );

    msDescription = rDescr;
}




/** Replace the model, page, and view pointers by the ones provided
    (explicitly and implicitly).
*/
void SvxGraphCtrlAccessibleContext::setModelAndView (
    SdrModel* pModel,
    SdrView* pView)
{
    OGuard aGuard (Application::GetSolarMutex());

    mpModel = pModel;
    if (mpModel != NULL)
        mpPage = (SdrPage*)mpModel->GetPage( 0 );
    mpView = pView;

    if (mpModel == NULL || mpPage == NULL || mpView == NULL)
    {
        mbDisposed = true;

        // Set all the pointers to NULL just in case they are used as
        // a disposed flag.
        mpModel = NULL;
        mpPage = NULL;
        mpView = NULL;
    }

    maTreeInfo.SetSdrView (mpView);
}



//-----------------------------------------------------------------------------

void SAL_CALL SvxGraphCtrlAccessibleContext::disposing()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mbDisposed )
        return;

    mbDisposed = sal_True;

    mpControl = NULL;       // object dies with representation
    mpView = NULL;
    mpPage = NULL;

    {
        ShapesMapType::iterator I;

        for (I=mxShapes.begin(); I!=mxShapes.end(); I++)
        {
            XAccessible* pAcc = (*I).second;
            Reference< XComponent > xComp( pAcc, UNO_QUERY );
            if( xComp.is() )
                xComp->dispose();

            (*I).second->release();
        }

        mxShapes.clear();
    }

    // Send a disposing to all listeners.
    if ( mnClientId )
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
        mnClientId =  0;
    }
}

//-----------------------------------------------------------------------------

Rectangle SvxGraphCtrlAccessibleContext::GetBoundingBoxOnScreen( void ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpControl )
        throw DisposedException();

    return Rectangle(
        mpControl->GetAccessibleParentWindow()->OutputToAbsoluteScreenPixel(
            mpControl->GetPosPixel() ),
        mpControl->GetSizePixel() );
}

//-----------------------------------------------------------------------------

/** Calculate the relative coordinates of the bounding box as difference
    between the absolute coordinates of the bounding boxes of this control
    and its parent in the accessibility tree.
*/
Rectangle SvxGraphCtrlAccessibleContext::GetBoundingBox( void ) throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    Rectangle aBounds ( 0, 0, 0, 0 );

    Window* pWindow = mpControl;
    if (pWindow != NULL)
    {
        aBounds = pWindow->GetWindowExtentsRelative (NULL);
        Window* pParent = pWindow->GetAccessibleParentWindow();
        if (pParent != NULL)
        {
            Rectangle aParentRect = pParent->GetWindowExtentsRelative (NULL);
            aBounds -= aParentRect.TopLeft();
        }
    }
    else
        throw DisposedException();

    return aBounds;
}

//-----------------------------------------------------------------------------

Sequence< sal_Int8 > SvxGraphCtrlAccessibleContext::getUniqueId( void )
{
    // no guard because it's private -> has to guarded when using it!
    static OImplementationId*   pId = 0;
    if( !pId )
    {
        OGuard aGuard( Application::GetSolarMutex() );
        if( !pId)
        {
            static OImplementationId    aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

//-----------------------------------------------------------------------------

void SvxGraphCtrlAccessibleContext::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint )
    {
        switch( pSdrHint->GetKind() )
        {
            case HINT_OBJCHG:
                {
                    ShapesMapType::iterator iter = mxShapes.find( pSdrHint->GetObject() );

                    if( iter != mxShapes.end() )
                    {
                        // if we already have one, return it
                        AccessibleShape* pShape = (*iter).second;

                        if( NULL != pShape )
                            pShape->CommitChange( AccessibleEventId::VISIBLE_DATA_CHANGED, uno::Any(), uno::Any() );
                    }
                }
                break;

            case HINT_OBJINSERTED:
                CommitChange( AccessibleEventId::CHILD, makeAny( getAccessible( pSdrHint->GetObject() ) ) , uno::Any());
                break;
            case HINT_OBJREMOVED:
                CommitChange( AccessibleEventId::CHILD, uno::Any(), makeAny( getAccessible( pSdrHint->GetObject() ) )  );
                break;
            case HINT_MODELCLEARED:
                dispose();
                break;
        }
    }
    else
    {
        const SfxSimpleHint* pSfxHint = PTR_CAST(SfxSimpleHint, &rHint );

        // ist unser SdDrawDocument gerade gestorben?
        if(pSfxHint && pSfxHint->GetId() == SFX_HINT_DYING)
        {
            dispose();
        }
    }
}

//=====  IAccessibleViewforwarder  ========================================

sal_Bool SvxGraphCtrlAccessibleContext::IsValid (void) const
{
    return sal_True;
}

//-----------------------------------------------------------------------------

Rectangle SvxGraphCtrlAccessibleContext::GetVisibleArea (void) const
{
    if( mpView )
        return mpView->GetVisibleArea(0);
    else
        return Rectangle();
}

//-----------------------------------------------------------------------------

Point SvxGraphCtrlAccessibleContext::LogicToPixel (const Point& rPoint) const
{
    if( mpControl )
    {
        Rectangle aBBox(mpControl->GetWindowExtentsRelative(NULL));
        return mpControl->LogicToPixel (rPoint) + aBBox.TopLeft();
    }
    else
    {
        return rPoint;
    }
}

//-----------------------------------------------------------------------------

Size SvxGraphCtrlAccessibleContext::LogicToPixel (const Size& rSize) const
{
    if( mpControl )
        return mpControl->LogicToPixel (rSize);
    else
        return rSize;
}

//-----------------------------------------------------------------------------

Point SvxGraphCtrlAccessibleContext::PixelToLogic (const Point& rPoint) const
{
    if( mpControl )
        return mpControl->PixelToLogic (rPoint);
    else
        return rPoint;
}

//-----------------------------------------------------------------------------

Size SvxGraphCtrlAccessibleContext::PixelToLogic (const Size& rSize) const
{
    if( mpControl )
        return mpControl->PixelToLogic (rSize);
    else
        return rSize;
}
