/*************************************************************************
 *
 *  $RCSfile: ftpresultsetbase.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: abi $ $Date: 2002-07-31 15:13:33 $
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

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_LISTACTIONTYPE_HPP_
#include <com/sun/star/ucb/ListActionType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSOURCEINITIALIZATION_HPP_
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#endif
#ifndef _UCBHELPER_RESULTSETMETADATA_HXX
#include <ucbhelper/resultsetmetadata.hxx>
#endif
#ifndef _FTP_FTPRESULTSETBASE_HXX_
#include "ftpresultsetbase.hxx"
#endif

using namespace ftp;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


ResultSetBase::ResultSetBase(
    const uno::Reference< lang::XMultiServiceFactory >&  xMSF,
    const uno::Reference< XContentProvider >&  xProvider,
    sal_Int32 nOpenMode,
    const uno::Sequence< beans::Property >& seq,
    const uno::Sequence< NumberedSortingInfo >& seqSort )
    : m_xMSF( xMSF ),
      m_xProvider( xProvider ),
      m_nRow( -1 ),
      m_nWasNull( true ),
      m_nOpenMode( nOpenMode ),
      m_bRowCountFinal( true ),
      m_sProperty( seq ),
      m_sSortingInfo( seqSort ),
      m_pDisposeEventListeners( 0 ),
      m_pRowCountListeners( 0 ),
      m_pIsFinalListeners( 0 )
{
}

ResultSetBase::~ResultSetBase()
{
    delete m_pIsFinalListeners;
    delete m_pRowCountListeners;
    delete m_pDisposeEventListeners;
}


// XInterface

void SAL_CALL
ResultSetBase::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
ResultSetBase::release(
    void )
    throw()
{
    OWeakObject::release();
}



uno::Any SAL_CALL
ResultSetBase::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface(
        rType,
        SAL_STATIC_CAST( lang::XComponent*, this),
        SAL_STATIC_CAST( sdbc::XRow*, this),
        SAL_STATIC_CAST( sdbc::XResultSet*, this),
        SAL_STATIC_CAST( sdbc::XResultSetMetaDataSupplier*, this),
        SAL_STATIC_CAST( beans::XPropertySet*, this ),
        SAL_STATIC_CAST( XContentAccess*, this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



// XComponent


void SAL_CALL
ResultSetBase::addEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
ResultSetBase::removeEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}



void SAL_CALL
ResultSetBase::dispose()
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    lang::EventObject aEvt;
    aEvt.Source = static_cast< lang::XComponent * >( this );

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }
    if( m_pRowCountListeners && m_pRowCountListeners->getLength() )
    {
        m_pRowCountListeners->disposeAndClear( aEvt );
    }
    if( m_pIsFinalListeners && m_pIsFinalListeners->getLength() )
    {
        m_pIsFinalListeners->disposeAndClear( aEvt );
    }
}



//  XResultSet

sal_Bool SAL_CALL
ResultSetBase::next(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    sal_Bool test;
    if( ++m_nRow < m_aItems.size() )
        test = true;
    else
        test = false;
    return test;
}


sal_Bool SAL_CALL
ResultSetBase::isBeforeFirst(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    return m_nRow == -1;
}


sal_Bool SAL_CALL
ResultSetBase::isAfterLast(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    return m_nRow >= m_aItems.size();   // Cannot happen, if m_aFolder.isOpen()
}


sal_Bool SAL_CALL
ResultSetBase::isFirst(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    return m_nRow == 0;
}


sal_Bool SAL_CALL
ResultSetBase::isLast(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( m_nRow ==  m_aItems.size() - 1 )
        return true;
    else
        return false;
}


void SAL_CALL
ResultSetBase::beforeFirst(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    m_nRow = -1;
}


void SAL_CALL
ResultSetBase::afterLast(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    m_nRow = m_aItems.size();
}


sal_Bool SAL_CALL
ResultSetBase::first(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    m_nRow = -1;
    return next();
}


sal_Bool SAL_CALL
ResultSetBase::last(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    m_nRow = m_aItems.size() - 1;
    return true;
}


sal_Int32 SAL_CALL
ResultSetBase::getRow(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    // Test, whether behind last row
    if( -1 == m_nRow || m_nRow >= m_aItems.size() )
        return 0;
    else
        return m_nRow+1;
}


sal_Bool SAL_CALL ResultSetBase::absolute( sal_Int32 row )
    throw( sdbc::SQLException, uno::RuntimeException)
{
    if( row >= 0 )
        m_nRow = row - 1;
    else
    {
        last();
        m_nRow += ( row + 1 );
        if( m_nRow < -1 )
            m_nRow = -1;
    }

    return 0<= m_nRow && m_nRow < m_aItems.size();
}




sal_Bool SAL_CALL
ResultSetBase::relative(
    sal_Int32 row )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( isAfterLast() || isBeforeFirst() )
        throw sdbc::SQLException();

    if( row > 0 )
        while( row-- )
            next();
    else if( row < 0 )
        while( row++ && m_nRow > - 1 )
            previous();

    return 0 <= m_nRow && m_nRow < m_aItems.size();
}



sal_Bool SAL_CALL
ResultSetBase::previous(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( m_nRow > m_aItems.size() )
        m_nRow = m_aItems.size();  // Correct Handling of afterLast
    if( 0 <= m_nRow ) -- m_nRow;

    return 0 <= m_nRow && m_nRow < m_aItems.size();
}


void SAL_CALL
ResultSetBase::refreshRow(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
}


sal_Bool SAL_CALL
ResultSetBase::rowUpdated(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    return false;
}

sal_Bool SAL_CALL
ResultSetBase::rowInserted(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    return false;
}

sal_Bool SAL_CALL
ResultSetBase::rowDeleted(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    return false;
}


uno::Reference< uno::XInterface > SAL_CALL
ResultSetBase::getStatement(
    void  )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    uno::Reference< uno::XInterface > test( 0 );
    return test;
}


// XCloseable

void SAL_CALL
ResultSetBase::close(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
}


rtl::OUString SAL_CALL
ResultSetBase::queryContentIdentifierString(
    void )
    throw( uno::RuntimeException )
{
    if( 0 <= m_nRow && m_nRow < m_aItems.size() )
        return m_aPath[m_nRow];
    else
        return rtl::OUString();
}


uno::Reference< XContentIdentifier > SAL_CALL
ResultSetBase::queryContentIdentifier(
    void
)
    throw(
        uno::RuntimeException
    )
{
    if( 0 <= m_nRow && m_nRow < m_aItems.size() )
    {
        if(!m_aIdents[m_nRow].is()) {
            rtl::OUString url = queryContentIdentifierString();
            if(url.getLength() )
                m_aIdents[m_nRow] =
                    uno::Reference< XContentIdentifier >(
                        new ::ucb::ContentIdentifier(m_xMSF,url) );
        }
        return m_aIdents[m_nRow];
    }

    return uno::Reference<XContentIdentifier>();
}


uno::Reference< XContent > SAL_CALL
ResultSetBase::queryContent(
    void )
    throw( uno::RuntimeException )
{
    if( 0 <= m_nRow && m_nRow < m_aItems.size() )
        return m_xProvider->queryContent(queryContentIdentifier());
    else
        return uno::Reference< XContent >();
}



class XPropertySetInfoImpl
    : public cppu::OWeakObject,
      public beans::XPropertySetInfo
{
public:

    XPropertySetInfoImpl( const uno::Sequence< beans::Property >& aSeq )
        : m_aSeq( aSeq )
    {
    }

    void SAL_CALL acquire( void )
        throw()
    {
        OWeakObject::acquire();
    }


    void SAL_CALL release( void )
        throw()
    {
        OWeakObject::release();
    }

    uno::Any SAL_CALL queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
    {
        uno::Any aRet = cppu::queryInterface(
            rType,
            SAL_STATIC_CAST( beans::XPropertySetInfo*, this ) );
        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }

    uno::Sequence< beans::Property > SAL_CALL getProperties()
        throw( uno::RuntimeException )
    {
        return m_aSeq;
    }

    beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName )
        throw( beans::UnknownPropertyException,
               uno::RuntimeException)
    {
        for( int i = 0; i < m_aSeq.getLength(); ++i )
            if( aName == m_aSeq[i].Name )
                return m_aSeq[i];
        throw beans::UnknownPropertyException();
    }

    sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name )
        throw( uno::RuntimeException )
    {
        for( int i = 0; i < m_aSeq.getLength(); ++i )
            if( Name == m_aSeq[i].Name )
                return true;
        return false;
    }

private:

    uno::Sequence< beans::Property > m_aSeq;
};



// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL
ResultSetBase::getPropertySetInfo()
    throw( uno::RuntimeException)
{
    uno::Sequence< beans::Property > seq(2);
    seq[0].Name = rtl::OUString::createFromAscii( "RowCount" );
    seq[0].Handle = -1;
    seq[0].Type = getCppuType( static_cast< sal_Int32* >(0) );
    seq[0].Attributes = beans::PropertyAttribute::READONLY;

    seq[1].Name = rtl::OUString::createFromAscii( "IsRowCountFinal" );
    seq[1].Handle = -1;
    seq[1].Type = getCppuType( static_cast< sal_Bool* >(0) );
    seq[1].Attributes = beans::PropertyAttribute::READONLY;

    //t
    return uno::Reference< beans::XPropertySetInfo > (
        new XPropertySetInfoImpl( seq ) );
}



void SAL_CALL ResultSetBase::setPropertyValue(
    const rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    if( aPropertyName == rtl::OUString::createFromAscii( "IsRowCountFinal" ) ||
        aPropertyName == rtl::OUString::createFromAscii( "RowCount" ) )
        return;

    throw beans::UnknownPropertyException();
}


uno::Any SAL_CALL ResultSetBase::getPropertyValue(
    const rtl::OUString& PropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    if( PropertyName == rtl::OUString::createFromAscii( "IsRowCountFinal" ) )
    {
        uno::Any aAny;
        aAny <<= m_bRowCountFinal;
        return aAny;
    }
    else if ( PropertyName == rtl::OUString::createFromAscii( "RowCount" ) )
    {
        uno::Any aAny;
        sal_Int32 count = m_aItems.size();
        aAny <<= count;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException();
}


void SAL_CALL ResultSetBase::addPropertyChangeListener(
    const rtl::OUString& aPropertyName,
    const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    if( aPropertyName == rtl::OUString::createFromAscii( "IsRowCountFinal" ) )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( ! m_pIsFinalListeners )
            m_pIsFinalListeners =
                new cppu::OInterfaceContainerHelper( m_aMutex );

        m_pIsFinalListeners->addInterface( xListener );
    }
    else if ( aPropertyName == rtl::OUString::createFromAscii( "RowCount" ) )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( ! m_pRowCountListeners )
            m_pRowCountListeners =
                new cppu::OInterfaceContainerHelper( m_aMutex );
        m_pRowCountListeners->addInterface( xListener );
    }
    else
        throw beans::UnknownPropertyException();
}


void SAL_CALL ResultSetBase::removePropertyChangeListener(
    const rtl::OUString& aPropertyName,
    const uno::Reference< beans::XPropertyChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    if( aPropertyName == rtl::OUString::createFromAscii( "IsRowCountFinal" ) &&
        m_pIsFinalListeners )
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_pIsFinalListeners->removeInterface( aListener );
    }
    else if ( aPropertyName == rtl::OUString::createFromAscii( "RowCount" ) &&
              m_pRowCountListeners )
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_pRowCountListeners->removeInterface( aListener );
    }
    else
        throw beans::UnknownPropertyException();
}


void SAL_CALL ResultSetBase::addVetoableChangeListener(
    const rtl::OUString& PropertyName,
    const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
}


void SAL_CALL ResultSetBase::removeVetoableChangeListener(
    const rtl::OUString& PropertyName,
    const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
}



// XResultSetMetaDataSupplier
uno::Reference< sdbc::XResultSetMetaData > SAL_CALL
ResultSetBase::getMetaData(
    void )
    throw( sdbc::SQLException,
           uno::RuntimeException )
{
    ::ucb::ResultSetMetaData* p =
          new ::ucb::ResultSetMetaData(
              m_xMSF, m_sProperty );
    return uno::Reference< sdbc::XResultSetMetaData >( p );
}




