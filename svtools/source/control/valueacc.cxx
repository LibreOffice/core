/*************************************************************************
 *
 *  $RCSfile: valueacc.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 15:49:19 $
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

#define _SV_VALUESET_CXX
#define private public

#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include "valueset.hxx"
#include "valueimp.hxx"

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

using namespace ::com::sun::star;

// ----------------
// - ValueSetItem -
// ----------------

ValueSetItem::ValueSetItem( ValueSet& rParent ) :
    mrParent( rParent ),
    mnId( 0 ),
    mnBits( 0 ),
    mpData( NULL ),
    mpxAcc( NULL )
{
}

// -----------------------------------------------------------------------

ValueSetItem::~ValueSetItem()
{
    if( mpxAcc )
    {
        static_cast< ValueItemAcc* >( mpxAcc->get() )->ParentDestroyed();
        delete mpxAcc;
    }
}

// -----------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > ValueSetItem::GetAccessible()
{
    if( !mpxAcc )
        mpxAcc = new uno::Reference< accessibility::XAccessible >( new ValueItemAcc( this ) );

    return *mpxAcc;
}

// -----------------------------------------------------------------------

void ValueSetItem::ClearAccessible()
{
    if( mpxAcc )
        delete mpxAcc, mpxAcc = NULL;
}


// ---------------
// - ValueSetAcc -
// ---------------

ValueSetAcc::ValueSetAcc( ValueSet* pParent ) :
    ValueSetAccComponentBase (m_aMutex),
    mpParent( pParent )
{
}

// -----------------------------------------------------------------------------

ValueSetAcc::~ValueSetAcc()
{
}

// -----------------------------------------------------------------------

void ValueSetAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( nEventId )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator  aIter( aTmpListeners.begin() );
        accessibility::AccessibleEventObject                                                        aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<uno::XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        while( aIter != aTmpListeners.end() )
        {
            try
            {
                (*aIter)->notifyEvent( aEvtObject );
            }
            catch( uno::Exception& )
            {
            }

            aIter++;
        }
    }
}

// -----------------------------------------------------------------------------

const uno::Sequence< sal_Int8 >& ValueSetAcc::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq;

    if( !aSeq.getLength() )
    {
        static osl::Mutex           aCreateMutex;
        osl::Guard< osl::Mutex >    aGuard( aCreateMutex );

        aSeq.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aSeq.getArray() ), 0, sal_True );
    }

    return aSeq;
}

// -----------------------------------------------------------------------------

ValueSetAcc* ValueSetAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? ( (ValueSetAcc*)(void*) xUnoTunnel->getSomething( ValueSetAcc::getUnoTunnelId() ) ) : NULL );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        return NULL;
    }
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ValueSetAcc::getAccessibleContext()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return this;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    const vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ThrowIfDisposed();

    sal_Int32 nCount = 0;
    if ((mpParent->GetStyle() & ~WB_NONEFIELD) != 0)
        nCount += 1;
    nCount += mpParent->ImplGetVisibleItemCount();
    return nCount;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard                               aSolarGuard( Application::GetSolarMutex() );
    uno::Reference< accessibility::XAccessible >    xRet;
    ValueSetItem* pItem = getItem (i);

    if( pItem )
        xRet = pItem->GetAccessible();
    else
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleParent()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard                               aSolarGuard( Application::GetSolarMutex() );
    Window*                                         pParent = mpParent->GetParent();
    uno::Reference< accessibility::XAccessible >    xRet;

    if( pParent )
        xRet = pParent->GetAccessible();

    return xRet;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getAccessibleIndexInParent()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard       aSolarGuard( Application::GetSolarMutex() );
    Window*                 pParent = mpParent->GetParent();
    sal_Int32               nRet = 0;

    if( pParent )
    {
        sal_Bool bFound = sal_False;

        for( USHORT i = 0, nCount = pParent->GetChildCount(); ( i < nCount ) && !bFound; i++ )
        {
            if( pParent->GetChild( i ) == mpParent )
            {
                nRet = i;
                bFound = sal_True;
            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL ValueSetAcc::getAccessibleRole()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return accessibility::AccessibleRole::LIST;
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL ValueSetAcc::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    String              aRet( RTL_CONSTASCII_USTRINGPARAM( "ValueSet" ) );

    return aRet;
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL ValueSetAcc::getAccessibleName()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    String              aRet;

    if( mpParent )
        aRet = mpParent->GetText();

    if( !aRet.Len() )
        aRet = getAccessibleDescription();

    return aRet;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueSetAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ValueSetAcc::getAccessibleStateSet()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    ::utl::AccessibleStateSetHelper* pStateSet = new ::utl::AccessibleStateSetHelper();

    // Set some states.
    pStateSet->AddState (accessibility::AccessibleStateType::ENABLED);
    pStateSet->AddState (accessibility::AccessibleStateType::SHOWING);
    pStateSet->AddState (accessibility::AccessibleStateType::VISIBLE);
    pStateSet->AddState (accessibility::AccessibleStateType::MANAGES_DESCENDANTS);

    return pStateSet;
}

// -----------------------------------------------------------------------------

lang::Locale SAL_CALL ValueSetAcc::getLocale()
    throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard                               aSolarGuard( Application::GetSolarMutex() );
    const ::rtl::OUString                           aEmptyStr;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( aEmptyStr, aEmptyStr, aEmptyStr );

    if( xParent.is() )
    {
        uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale ();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::addEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

    if( rxListener.is() )
    {
           ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter = mxEventListeners.begin();
        sal_Bool bFound = sal_False;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
                bFound = sal_True;
            else
                aIter++;
        }

        if (!bFound)
            mxEventListeners.push_back( rxListener );
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::removeEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (m_aMutex);

    if( rxListener.is() )
    {
           ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter = mxEventListeners.begin();
        sal_Bool bFound = sal_False;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
            {
                mxEventListeners.erase( aIter );
                bFound = sal_True;
            }
            else
                aIter++;
        }
    }
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL ValueSetAcc::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard                               aSolarGuard( Application::GetSolarMutex() );
    const USHORT                                    nItemId = mpParent->GetItemId( Point( aPoint.X, aPoint.Y ) );
    uno::Reference< accessibility::XAccessible >    xRet;

    if( VALUESET_ITEM_NOTFOUND != nItemId )
    {
        const USHORT nItemPos = mpParent->GetItemPos( nItemId );

        if( VALUESET_ITEM_NONEITEM != nItemPos )
        {
            ValueSetItem* pItem = mpParent->mpItemList->GetObject( nItemPos );

            if( ( pItem->meType != VALUESETITEM_SPACE ) && !pItem->maRect.IsEmpty() )
               xRet = pItem->GetAccessible();
        }
    }

    return xRet;
}

// -----------------------------------------------------------------------------

awt::Rectangle SAL_CALL ValueSetAcc::getBounds()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    const Point         aOutPos( mpParent->GetPosPixel() );
    const Size          aOutSize( mpParent->GetOutputSizePixel() );
    awt::Rectangle      aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ValueSetAcc::getLocation()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ValueSetAcc::getLocationOnScreen()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    const Point         aScreenPos( mpParent->OutputToAbsoluteScreenPixel( Point() ) );
    awt::Point          aRet;

    aRet.X = aScreenPos.X();
    aRet.Y = aScreenPos.Y();

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Size SAL_CALL ValueSetAcc::getSize()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::grabFocus()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    mpParent->GrabFocus();
}

// -----------------------------------------------------------------------------

uno::Any SAL_CALL ValueSetAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return uno::Any();
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getForeground(  )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    UINT32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getBackground(  )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    UINT32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::selectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    ValueSetItem* pItem = getItem (nChildIndex);

    if(pItem != NULL)
    {
        mpParent->SelectItem( pItem->mnId );
        mpParent->Select ();
    }
    else
        throw lang::IndexOutOfBoundsException();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL ValueSetAcc::isAccessibleChildSelected( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    ValueSetItem* pItem = getItem (nChildIndex);
    sal_Bool            bRet = sal_False;

    if (pItem != NULL)
        bRet = mpParent->IsItemSelected( pItem->mnId );
    else
        throw lang::IndexOutOfBoundsException();

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::clearAccessibleSelection()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    mpParent->SetNoSelection();
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::selectAllAccessible()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    // unsupported due to single selection only
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueSetAcc::getSelectedAccessibleChildCount()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    sal_Int32           nRet = 0;

    for( USHORT i = 0, nCount = getItemCount(); i < nCount; i++ )
    {
        ValueSetItem* pItem = getItem (i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) )
            ++nRet;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueSetAcc::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard                               aSolarGuard( Application::GetSolarMutex() );
    uno::Reference< accessibility::XAccessible >    xRet;

    for( USHORT i = 0, nCount = getItemCount(), nSel = 0; ( i < nCount ) && !xRet.is(); i++ )
    {
        ValueSetItem* pItem = getItem(i);

        if( pItem && mpParent->IsItemSelected( pItem->mnId ) && ( nSelectedChildIndex == static_cast< sal_Int32 >( nSel++ ) ) )
            xRet = pItem->GetAccessible();
    }

    return xRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueSetAcc::deselectAccessibleChild( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    sal_Bool            bDone = sal_False;

    // Because of the single selection we can reset the whole selection when
    // the specified child is currently selected.
    if (isAccessibleChildSelected(nChildIndex))
        mpParent->SetNoSelection();
}

// -----------------------------------------------------------------------------

sal_Int64 SAL_CALL ValueSetAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == rtl_compareMemory( ValueSetAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}




void SAL_CALL ValueSetAcc::disposing (void)
{
    ::std::vector<uno::Reference<accessibility::XAccessibleEventListener> > aListenerListCopy;

    {
        // Make a copy of the list and clear the original.
        const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
        ::osl::MutexGuard aGuard (m_aMutex);
        aListenerListCopy = mxEventListeners;
        mxEventListeners.clear();

        // Reset the pointer to the parent.  It has to be the one who has
        // disposed us because he is dying.
        mpParent = NULL;
    }

    // Inform all listeners that this objects is disposing.
    ::std::vector<uno::Reference<accessibility::XAccessibleEventListener> >::const_iterator
          aListenerIterator (aListenerListCopy.begin());
    lang::EventObject aEvent (static_cast<accessibility::XAccessible*>(this));
    while (aListenerIterator != aListenerListCopy.end())
    {
        try
        {
            (*aListenerIterator)->disposing (aEvent);
        }
        catch( uno::Exception& )
        {
            // Ignore exceptions.
        }

        ++aListenerIterator;
    }
}


USHORT ValueSetAcc::getItemCount (void) const
{
    USHORT nCount = mpParent->ImplGetVisibleItemCount();
    // When the None-Item is visible then increase the number of items by
    // one.
    if ((mpParent->GetStyle() & WB_NONEFIELD) != 0)
        nCount += 1;
    return nCount;
}


ValueSetItem* ValueSetAcc::getItem (USHORT nIndex) const
{
    ValueSetItem* pItem = NULL;

    if ((mpParent->GetStyle() & WB_NONEFIELD) != 0)
        if (nIndex == 0)
            // When present the first item is the then allways visible none field.
            pItem = mpParent->ImplGetItem (VALUESET_ITEM_NONEITEM);
        else
            // Shift down the index to compensate for the none field.
            nIndex -= 1;
    if (pItem == NULL)
        pItem = mpParent->ImplGetVisibleItem (static_cast<USHORT>(nIndex));

    return pItem;
}




void ValueSetAcc::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("object has been already disposed")),
            static_cast<uno::XWeak*>(this));
    }
    else
        DBG_ASSERT (mpParent!=NULL, "ValueSetAcc not disposed but mpParent == NULL");
}



sal_Bool ValueSetAcc::IsDisposed (void)
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}




// ----------------
// - ValueItemAcc -
// ----------------

ValueItemAcc::ValueItemAcc( ValueSetItem* pParent ) :
    mpParent( pParent )
{
}

// -----------------------------------------------------------------------------

ValueItemAcc::~ValueItemAcc()
{
}

// -----------------------------------------------------------------------

void ValueItemAcc::FireAccessibleEvent( short nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    if( nEventId )
    {
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >                  aTmpListeners( mxEventListeners );
        ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator  aIter( aTmpListeners.begin() );
        accessibility::AccessibleEventObject                                                        aEvtObject;

        aEvtObject.EventId = nEventId;
        aEvtObject.Source = static_cast<uno::XWeak*>(this);
        aEvtObject.NewValue = rNewValue;
        aEvtObject.OldValue = rOldValue;

        while( aIter != aTmpListeners.end() )
        {
            (*aIter)->notifyEvent( aEvtObject );
            aIter++;
        }
    }
}

// -----------------------------------------------------------------------------

void ValueItemAcc::ParentDestroyed()
{
    const ::vos::OGuard aGuard( maMutex );
    mpParent = NULL;
}

// -----------------------------------------------------------------------------

const uno::Sequence< sal_Int8 >& ValueItemAcc::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq;

    if( !aSeq.getLength() )
    {
        static osl::Mutex           aCreateMutex;
        osl::Guard< osl::Mutex >    aGuard( aCreateMutex );

        aSeq.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aSeq.getArray() ), 0, sal_True );
    }

    return aSeq;
}

// -----------------------------------------------------------------------------

ValueItemAcc* ValueItemAcc::getImplementation( const uno::Reference< uno::XInterface >& rxData )
    throw()
{
    try
    {
        uno::Reference< lang::XUnoTunnel > xUnoTunnel( rxData, uno::UNO_QUERY );
        return( xUnoTunnel.is() ? ( (ValueItemAcc*)(void*) xUnoTunnel->getSomething( ValueItemAcc::getUnoTunnelId() ) ) : NULL );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        return NULL;
    }
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ValueItemAcc::getAccessibleContext()
    throw (uno::RuntimeException)
{
    return this;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueItemAcc::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    return 0;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException();
    return uno::Reference< accessibility::XAccessible >();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleParent()
    throw (uno::RuntimeException)
{
    const vos::OGuard                               aSolarGuard( Application::GetSolarMutex() );
    uno::Reference< accessibility::XAccessible >    xRet;

    if( mpParent )
        xRet = mpParent->mrParent.GetAccessible();

    return xRet;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueItemAcc::getAccessibleIndexInParent()
    throw (uno::RuntimeException)
{
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    sal_Int32           nRet = 0;

    if( mpParent )
    {
        sal_Bool bDone = sal_False;

        for( USHORT i = 0, nCount = mpParent->mrParent.ImplGetVisibleItemCount(); ( i < nCount ) && !bDone; i++ )
        {
            ValueSetItem* pItem = mpParent->mrParent.ImplGetVisibleItem( i );

            if( pItem && ( pItem->GetAccessible().get() == this ) )
            {
                nRet = i;
                bDone = sal_True;
            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL ValueItemAcc::getAccessibleRole()
    throw (uno::RuntimeException)
{
    return accessibility::AccessibleRole::LIST_ITEM;
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL ValueItemAcc::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    String              aRet( RTL_CONSTASCII_USTRINGPARAM( "ValueSet item" ) );

    return aRet;
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL ValueItemAcc::getAccessibleName()
    throw (uno::RuntimeException)
{
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    String              aRet;

    if( mpParent )
    {
        aRet = mpParent->maText;

        if( !aRet.Len() )
        {
            aRet = String( RTL_CONSTASCII_USTRINGPARAM( "Item " ) );
            aRet += String::CreateFromInt32( mpParent->mnId );
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ValueItemAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL ValueItemAcc::getAccessibleStateSet()
    throw (uno::RuntimeException)
{
    const vos::OGuard                   aSolarGuard( Application::GetSolarMutex() );
    ::utl::AccessibleStateSetHelper*    pStateSet = new ::utl::AccessibleStateSetHelper;

    if( mpParent )
    {
        pStateSet->AddState (accessibility::AccessibleStateType::ENABLED);
        pStateSet->AddState (accessibility::AccessibleStateType::SHOWING);
        pStateSet->AddState (accessibility::AccessibleStateType::VISIBLE);
        pStateSet->AddState (accessibility::AccessibleStateType::TRANSIENT);

        // SELECTABLE
        pStateSet->AddState( accessibility::AccessibleStateType::SELECTABLE );
        //      pStateSet->AddState( accessibility::AccessibleStateType::FOCUSABLE );

        // SELECTED
        if( mpParent->mrParent.GetSelectItemId() == mpParent->mnId )
        {
            pStateSet->AddState( accessibility::AccessibleStateType::SELECTED );
            //              pStateSet->AddState( accessibility::AccessibleStateType::FOCUSED );
        }
    }

    return pStateSet;
}

// -----------------------------------------------------------------------------

lang::Locale SAL_CALL ValueItemAcc::getLocale()
    throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    const vos::OGuard                               aSolarGuard( Application::GetSolarMutex() );
    const ::rtl::OUString                           aEmptyStr;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( aEmptyStr, aEmptyStr, aEmptyStr );

    if( xParent.is() )
    {
        uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueItemAcc::addEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    const ::vos::OGuard aGuard( maMutex );

    if( rxListener.is() )
    {
           ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::const_iterator aIter = mxEventListeners.begin();
        sal_Bool bFound = sal_False;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
                bFound = sal_True;
            else
                aIter++;
        }

        if (!bFound)
            mxEventListeners.push_back( rxListener );
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueItemAcc::removeEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    const ::vos::OGuard aGuard( maMutex );

    if( rxListener.is() )
    {
           ::std::vector< uno::Reference< accessibility::XAccessibleEventListener > >::iterator aIter = mxEventListeners.begin();
        sal_Bool bFound = sal_False;

        while( !bFound && ( aIter != mxEventListeners.end() ) )
        {
            if( *aIter == rxListener )
            {
                mxEventListeners.erase( aIter );
                bFound = sal_True;
            }
            else
                aIter++;
        }
    }
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL ValueItemAcc::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    const Point             aSize( aRect.Width, aRect.Height );
    const Point             aNullPoint, aTestPoint( aPoint.X, aPoint.Y );

    return Rectangle( aNullPoint, aSize ).IsInside( aTestPoint );
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL ValueItemAcc::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

// -----------------------------------------------------------------------------

awt::Rectangle SAL_CALL ValueItemAcc::getBounds()
    throw (uno::RuntimeException)
{
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    awt::Rectangle      aRet;

    if( mpParent )
    {
        Rectangle   aRect( mpParent->maRect );
        Point       aOrigin;
        Rectangle   aParentRect( aOrigin, mpParent->mrParent.GetOutputSizePixel() );

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ValueItemAcc::getLocation()
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Point SAL_CALL ValueItemAcc::getLocationOnScreen()
    throw (uno::RuntimeException)
{
    const vos::OGuard   aSolarGuard( Application::GetSolarMutex() );
    awt::Point          aRet;

    if( mpParent )
    {
        const Point aScreenPos( mpParent->mrParent.OutputToAbsoluteScreenPixel( mpParent->maRect.TopLeft() ) );

        aRet.X = aScreenPos.X();
        aRet.Y = aScreenPos.Y();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

awt::Size SAL_CALL ValueItemAcc::getSize()
    throw (uno::RuntimeException)
{
    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL ValueItemAcc::grabFocus()
    throw (uno::RuntimeException)
{
    // nothing to do
}

// -----------------------------------------------------------------------------

uno::Any SAL_CALL ValueItemAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    return uno::Any();
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueItemAcc::getForeground(  )
    throw (uno::RuntimeException)
{
    UINT32 nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL ValueItemAcc::getBackground(  )
    throw (uno::RuntimeException)
{
    UINT32 nColor;
    if (mpParent->meType == VALUESETITEM_COLOR)
        nColor = mpParent->maColor.GetColor();
    else
        nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// -----------------------------------------------------------------------------

sal_Int64 SAL_CALL ValueItemAcc::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw( uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) && ( 0 == rtl_compareMemory( ValueItemAcc::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
        nRet = reinterpret_cast< sal_Int64 >( this );
    else
        nRet = 0;

    return nRet;
}
