/*************************************************************************
 *
 *  $RCSfile: VCollection.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-09 12:06:36 $
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

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_ENUMHELPER_HXX_
#include "enumhelper.hxx"
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif

using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OCollection,"com.sun.star.sdbcx.VContainer" , "com.sun.star.sdbcx.Container")

// -------------------------------------------------------------------------
void OCollection::disposing(void)
{
    m_aRefreshListeners.disposeAndClear(EventObject(static_cast<XWeak*>(this)));

    ::osl::MutexGuard aGuard(m_rMutex);
    for( ObjectIter aIter = m_aNameMap.begin(); aIter != m_aNameMap.end(); ++aIter)
    {
        if((*aIter).second.is())
        {
            Reference< XComponent > xComp2((*aIter).second, UNO_QUERY);
            if(xComp2.is())
                xComp2->dispose();

            (*aIter).second = Reference< XNamed >();
        }
    }
    m_aNameMap.clear();
    m_aElements.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL OCollection::getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (Index < 0 || Index >= getCount())
        throw IndexOutOfBoundsException();

    ObjectIter aIter = m_aElements[Index];
    Reference< XNamed > xName = (*aIter).second;
    if(!(*aIter).second.is())
    {
        xName = createObject((*aIter).first);
        (*aIter).second = xName;
    }

    return makeAny( Reference< XPropertySet >(xName,UNO_QUERY));
}
// -------------------------------------------------------------------------
Any SAL_CALL OCollection::getByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    ObjectIter aIter = m_aNameMap.find(aName);
    if(aIter == m_aNameMap.end())
        return Any();
    Reference< XNamed > xName = (*aIter).second;
    if(!(*aIter).second.is())
    {
        xName = createObject(aName);
        (*aIter).second = xName;
    }

    return makeAny( Reference< XPropertySet >(xName,UNO_QUERY));
}
// -------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OCollection::getElementNames(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    sal_Int32 nLen = getCount();
    Sequence< ::rtl::OUString > aNameList(nLen);

    sal_Int32 i=0;
    ::rtl::OUString* pStringArray = aNameList.getArray();
    //  for( ::std::map< ::rtl::OUString, Object_BASE, ::utl::UStringLess> ::const_iterator aIter = m_aNameMap.begin(); aIter != m_aNameMap.end(); ++aIter)
    for(::std::vector< ObjectIter >::const_iterator aIter = m_aElements.begin(); aIter != m_aElements.end();++aIter)
        pStringArray[i++] = (*aIter)->first;

    return aNameList;
}
// -------------------------------------------------------------------------
void SAL_CALL OCollection::refresh(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    for( ::std::map< ::rtl::OUString, Object_BASE, ::utl::UStringLess>::iterator aIter = m_aNameMap.begin(); aIter != m_aNameMap.end(); ++aIter)
    {
        if((*aIter).second.is())
        {
            Reference< XComponent > xComp2((*aIter).second, UNO_QUERY);
            if(xComp2.is())
                xComp2->dispose();

            (*aIter).second = Reference< XNamed >();
        }
    }
    m_aNameMap.clear();
    m_aElements.clear();

    impl_refresh();
    EventObject aEvt(static_cast<XWeak*>(this));
    NOTIFY_LISTENERS(m_aRefreshListeners, XRefreshListener, refreshed, aEvt);
}
// -------------------------------------------------------------------------
// XDataDescriptorFactory
Reference< XPropertySet > SAL_CALL OCollection::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    return createEmptyObject();
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OCollection::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    Reference< XNamed > xName(descriptor,UNO_QUERY);
    if(xName.is())
    {
        if(m_aNameMap.find(xName->getName()) != m_aNameMap.end())
            throw ElementExistException(xName->getName(),*this);

        m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(xName->getName(),WeakReference< XNamed >(xName))));
    }
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OCollection::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    ObjectMap::iterator aIter = m_aNameMap.find(elementName);
    if( aIter == m_aNameMap.end())
        throw NoSuchElementException(elementName,*this);

    for(::std::vector< ObjectIter >::size_type i=0;i<m_aElements.size();++i)
    {
        if(m_aElements[i] == aIter)
        {
            m_aElements.erase(m_aElements.begin()+i);
            m_aNameMap.erase(aIter);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OCollection::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if(index <0 || index > getCount())
        throw IndexOutOfBoundsException(::rtl::OUString(),*this);

    m_aNameMap.erase(m_aElements[index]);
    m_aElements.erase(m_aElements.begin()+index);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCollection::findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ObjectIter aIter = m_aNameMap.find(columnName);
    if(aIter == m_aNameMap.end())
        throw ::com::sun::star::sdbc::SQLException();

    return m_aElements.size() - (m_aElements.end() - ::std::find(m_aElements.begin(),m_aElements.end(),aIter)) +1; // because cloumns start at one
}
// -------------------------------------------------------------------------
Reference< XEnumeration > SAL_CALL OCollection::createEnumeration(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return new connectivity::OEnumerationByIndex( static_cast< XIndexAccess*>(this));
}

