/*************************************************************************
 *
 *  $RCSfile: propertysethelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 14:14:19 $
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

//_________________________________________________________________________________________________________________
//  my own includes

#include <classes/propertysethelper.hxx>
#include <threadhelp/transactionguard.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

//_________________________________________________________________________________________________________________
//  interface includes

//_________________________________________________________________________________________________________________
//  other includes

//_________________________________________________________________________________________________________________
//  namespace

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported definitions

//-----------------------------------------------------------------------------
PropertySetHelper::PropertySetHelper(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR             ,
                                           ::vos::IMutex*                                          pSolarMutex       ,
                                           sal_Bool                                                bReleaseLockOnCall)
    : ThreadHelpBase         (pSolarMutex                   )
    , TransactionBase        (                              )
    , m_xSMGR                (xSMGR                         )
    , m_lSimpleChangeListener(m_aLock.getShareableOslMutex())
    , m_lVetoChangeListener  (m_aLock.getShareableOslMutex())
    , m_bReleaseLockOnCall   (bReleaseLockOnCall            )
{
}

//-----------------------------------------------------------------------------
PropertySetHelper::~PropertySetHelper()
{
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::impl_addPropertyInfo(const css::beans::Property& aProperty)
    throw(css::beans::PropertyExistException,
          css::uno::Exception               )
{
    TransactionGuard aTransaction(m_aTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(aProperty.Name);
    if (pIt != m_lProps.end())
        throw css::beans::PropertyExistException();

    m_lProps[aProperty.Name] = aProperty;
    // <- SAFE
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::impl_removePropertyInfo(const ::rtl::OUString& sProperty)
    throw(css::beans::UnknownPropertyException,
          css::uno::Exception                 )
{
    TransactionGuard aTransaction(m_aTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    PropertySetHelper::TPropInfoHash::iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    m_lProps.erase(pIt);
    // <- SAFE
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::impl_enablePropertySet()
{
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::impl_disablePropertySet()
{
    TransactionGuard aTransaction(m_aTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::uno::XInterface > xThis(static_cast< css::beans::XPropertySet* >(this), css::uno::UNO_QUERY);
    css::lang::EventObject aEvent(xThis);

    m_lSimpleChangeListener.disposeAndClear(aEvent);
    m_lVetoChangeListener.disposeAndClear(aEvent);
    m_lProps.free();

    aWriteLock.unlock();
    // <- SAFE
}

//-----------------------------------------------------------------------------
sal_Bool PropertySetHelper::impl_existsVeto(const css::beans::PropertyChangeEvent& aEvent)
{
    /*  Dont use the lock here!
        The used helper is threadsafe and it lives for the whole lifetime of
        our own object.
    */
    ::cppu::OInterfaceContainerHelper* pVetoListener = m_lVetoChangeListener.getContainer(aEvent.PropertyName);
    if (! pVetoListener)
        return sal_False;

    ::cppu::OInterfaceIteratorHelper pListener(*pVetoListener);
    while (pListener.hasMoreElements())
    {
        try
        {
            css::uno::Reference< css::beans::XVetoableChangeListener > xListener(
                ((css::beans::XVetoableChangeListener*)pListener.next()),
                css::uno::UNO_QUERY_THROW);
            xListener->vetoableChange(aEvent);
        }
        catch(const css::uno::RuntimeException&)
            { pListener.remove(); }
        catch(const css::beans::PropertyVetoException&)
            { return sal_True; }
    }

    return sal_False;
}

//-----------------------------------------------------------------------------
void PropertySetHelper::impl_notifyChangeListener(const css::beans::PropertyChangeEvent& aEvent)
{
    /*  Dont use the lock here!
        The used helper is threadsafe and it lives for the whole lifetime of
        our own object.
    */
    ::cppu::OInterfaceContainerHelper* pSimpleListener = m_lSimpleChangeListener.getContainer(aEvent.PropertyName);
    if (! pSimpleListener)
        return;

    ::cppu::OInterfaceIteratorHelper pListener(*pSimpleListener);
    while (pListener.hasMoreElements())
    {
        try
        {
            css::uno::Reference< css::beans::XPropertyChangeListener > xListener(
                ((css::beans::XVetoableChangeListener*)pListener.next()),
                css::uno::UNO_QUERY_THROW);
            xListener->propertyChange(aEvent);
        }
        catch(const css::uno::RuntimeException&)
            { pListener.remove(); }
    }
}

//-----------------------------------------------------------------------------
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL PropertySetHelper::getPropertySetInfo()
    throw(css::uno::RuntimeException)
{
    TransactionGuard aTransaction(m_aTransactionManager, E_HARDEXCEPTIONS);

    css::uno::Reference< css::beans::XPropertySetInfo > xInfo(static_cast< css::beans::XPropertySetInfo* >(this), css::uno::UNO_QUERY_THROW);
    return xInfo;
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::setPropertyValue(const ::rtl::OUString& sProperty,
                                                  const css::uno::Any&   aValue   )
    throw(css::beans::UnknownPropertyException,
          css::beans::PropertyVetoException   ,
          css::lang::IllegalArgumentException ,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    // TODO look for e.g. readonly props and reject setProp() call!

    TransactionGuard aTransaction(m_aTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    css::beans::Property aPropInfo = pIt->second;

    sal_Bool bLocked = sal_True;
    if (m_bReleaseLockOnCall)
    {
        aWriteLock.unlock();
        bLocked = sal_False;
        // <- SAFE
    }

    css::uno::Any aCurrentValue = impl_getPropertyValue(aPropInfo.Name, aPropInfo.Handle);

    if (! bLocked)
    {
        // SAFE ->
        aWriteLock.lock();
        bLocked = sal_True;
    }

    sal_Bool bWillBeChanged = (aCurrentValue != aValue);
    if (! bWillBeChanged)
        return;

    css::beans::PropertyChangeEvent aEvent;
    aEvent.PropertyName   = aPropInfo.Name;
    aEvent.Further        = sal_False;
    aEvent.PropertyHandle = aPropInfo.Handle;
    aEvent.OldValue       = aCurrentValue;
    aEvent.NewValue       = aValue;

    if (m_bReleaseLockOnCall)
    {
        aWriteLock.unlock();
        bLocked = sal_False;
        // <- SAFE
    }

    if (impl_existsVeto(aEvent))
        throw css::beans::PropertyVetoException();

    impl_setPropertyValue(aPropInfo.Name, aPropInfo.Handle, aValue);

    impl_notifyChangeListener(aEvent);
}

//-----------------------------------------------------------------------------
css::uno::Any SAL_CALL PropertySetHelper::getPropertyValue(const ::rtl::OUString& sProperty)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_aTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    css::beans::Property aPropInfo = pIt->second;

    sal_Bool bLocked = sal_True;
    if (m_bReleaseLockOnCall)
    {
        aReadLock.unlock();
        bLocked = sal_False;
        // <- SAFE
    }

    css::uno::Any aValue = impl_getPropertyValue(aPropInfo.Name, aPropInfo.Handle);
    return aValue;
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::addPropertyChangeListener(const ::rtl::OUString&                                            sProperty,
                                                           const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_aTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.unlock();
    // <- SAFE

    m_lSimpleChangeListener.addInterface(sProperty, xListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::removePropertyChangeListener(const ::rtl::OUString&                                            sProperty,
                                                              const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_aTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.unlock();
    // <- SAFE

    m_lSimpleChangeListener.removeInterface(sProperty, xListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::addVetoableChangeListener(const ::rtl::OUString&                                            sProperty,
                                                           const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_aTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.unlock();
    // <- SAFE

    m_lVetoChangeListener.addInterface(sProperty, xListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::removeVetoableChangeListener(const ::rtl::OUString&                                            sProperty,
                                                              const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_aTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.unlock();
    // <- SAFE

    m_lVetoChangeListener.removeInterface(sProperty, xListener);
}

//-----------------------------------------------------------------------------
css::uno::Sequence< css::beans::Property > SAL_CALL PropertySetHelper::getProperties()
    throw(css::uno::RuntimeException)
{
    TransactionGuard aTransaction(m_aTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    sal_Int32                                        c     = (sal_Int32)m_lProps.size();
    css::uno::Sequence< css::beans::Property >       lProps(c);
    PropertySetHelper::TPropInfoHash::const_iterator pIt   ;

    for (  pIt  = m_lProps.begin();
           pIt != m_lProps.end()  ;
         ++pIt                    )
    {
        lProps[--c] = pIt->second;
    }

    return lProps;
    // <- SAFE
}

//-----------------------------------------------------------------------------
css::beans::Property SAL_CALL PropertySetHelper::getPropertyByName(const ::rtl::OUString& sName)
    throw(css::beans::UnknownPropertyException,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_aTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sName);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    return pIt->second;
    // <- SAFE
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL PropertySetHelper::hasPropertyByName(const ::rtl::OUString& sName)
    throw(css::uno::RuntimeException)
{
    TransactionGuard aTransaction(m_aTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    PropertySetHelper::TPropInfoHash::iterator pIt    = m_lProps.find(sName);
    sal_Bool                                   bExist = (pIt != m_lProps.end());

    return bExist;
    // <- SAFE
}

} // namespace framework
