/*************************************************************************
 *
 *  $RCSfile: contentresultsetwrapper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:52:35 $
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

#ifndef _CONTENT_RESULTSET_WRAPPER_HXX
#define _CONTENT_RESULTSET_WRAPPER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

//=========================================================================

class ContentResultSetWrapperListener;
class ContentResultSetWrapper
                : public cppu::OWeakObject
                , public com::sun::star::lang::XComponent
                , public com::sun::star::sdbc::XCloseable
                , public com::sun::star::sdbc::XResultSetMetaDataSupplier
                , public com::sun::star::beans::XPropertySet
                , public com::sun::star::ucb::XContentAccess
                , public com::sun::star::sdbc::XResultSet
                , public com::sun::star::sdbc::XRow
{
protected:

    //--------------------------------------------------------------------------
    //class PropertyChangeListenerContainer_Impl.

    struct equalStr_Impl
    {
            bool operator()( const rtl::OUString& s1, const rtl::OUString& s2 ) const
          {
            return !!( s1 == s2 );
        }
    };

    struct hashStr_Impl
    {
        size_t operator()( const rtl::OUString& rName ) const
        {
            return rName.hashCode();
        }
    };

    typedef cppu::OMultiTypeInterfaceContainerHelperVar
    < rtl::OUString , hashStr_Impl , equalStr_Impl >
    PropertyChangeListenerContainer_Impl;
    //--------------------------------------------------------------------------
    // class ReacquireableGuard

    class ReacquireableGuard
    {
    protected:
        vos::OMutex* pT;
    public:

        ReacquireableGuard(vos::OMutex * pT) : pT(pT)
        {
            pT->acquire();
        }

        ReacquireableGuard(vos::OMutex& t) : pT(&t)
        {
            pT->acquire();
        }

        /** Releases mutex. */
        ~ReacquireableGuard()
        {
            if (pT)
                pT->release();
        }

        /** Releases mutex. */
        void clear()
        {
            if(pT)
            {
                pT->release();
                pT = NULL;
            }
        }

        /** Reacquire mutex. */
        void reacquire()
        {
            if(pT)
            {
                pT->acquire();
            }
        }
    };

    //-----------------------------------------------------------------
    //members

    //my Mutex
    vos::OMutex             m_aMutex;

    //different Interfaces from Origin:
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xResultSetOrigin;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
                            m_xRowOrigin; //XRow-interface from m_xOrigin
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentAccess >
                            m_xContentAccessOrigin; //XContentAccess-interface from m_xOrigin
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >
                            m_xPropertySetOrigin; //XPropertySet-interface from m_xOrigin

    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >
                            m_xPropertySetInfo;

    sal_Int32               m_nForwardOnly;

private:
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >
                            m_xMyListenerImpl;
    ContentResultSetWrapperListener*
                            m_pMyListenerImpl;

    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSetMetaData >
                            m_xMetaDataFromOrigin; //XResultSetMetaData from m_xOrigin

    //management of listeners
    sal_Bool                m_bDisposed; ///Dispose call ready.
    sal_Bool                m_bInDispose;///In dispose call
    osl::Mutex              m_aContainerMutex;
    cppu::OInterfaceContainerHelper*
                            m_pDisposeEventListeners;
    PropertyChangeListenerContainer_Impl*
                            m_pPropertyChangeListeners;
    PropertyChangeListenerContainer_Impl*
                            m_pVetoableChangeListeners;

    //-----------------------------------------------------------------
    //methods:
private:
    PropertyChangeListenerContainer_Impl* SAL_CALL
    impl_getPropertyChangeListenerContainer();

    PropertyChangeListenerContainer_Impl* SAL_CALL
    impl_getVetoableChangeListenerContainer();

protected:
    //-----------------------------------------------------------------

    ContentResultSetWrapper( com::sun::star::uno::Reference<
                        com::sun::star::sdbc::XResultSet > xOrigin );

    virtual ~ContentResultSetWrapper();

    void SAL_CALL impl_init();
    void SAL_CALL impl_deinit();

    //--

    virtual void SAL_CALL impl_initPropertySetInfo(); //helping XPropertySet

    void SAL_CALL
    impl_EnsureNotDisposed()
        throw( com::sun::star::lang::DisposedException,
               com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_notifyPropertyChangeListeners(
            const com::sun::star::beans::PropertyChangeEvent& rEvt );

    void SAL_CALL
    impl_notifyVetoableChangeListeners(
            const com::sun::star::beans::PropertyChangeEvent& rEvt )
            throw( com::sun::star::beans::PropertyVetoException,
                   com::sun::star::uno::RuntimeException );

    sal_Bool SAL_CALL impl_isForwardOnly();

public:

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XComponent
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    dispose() throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
        com::sun::star::lang::XEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
        com::sun::star::lang::XEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    //XCloseable
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    close()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    //XResultSetMetaDataSupplier
    //-----------------------------------------------------------------
    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSetMetaData > SAL_CALL
    getMetaData()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XPropertySet
    //-----------------------------------------------------------------
    virtual com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo()
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    setPropertyValue( const rtl::OUString& aPropertyName,
                      const com::sun::star::uno::Any& aValue )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::PropertyVetoException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Any SAL_CALL
    getPropertyValue( const rtl::OUString& PropertyName )
        throw( com::sun::star::beans::UnknownPropertyException,
        com::sun::star::lang::WrappedTargetException,
        com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    addPropertyChangeListener( const rtl::OUString& aPropertyName,
                               const com::sun::star::uno::Reference<
                                       com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    removePropertyChangeListener( const rtl::OUString& aPropertyName,
                                  const com::sun::star::uno::Reference<
                                      com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    addVetoableChangeListener( const rtl::OUString& PropertyName,
                               const com::sun::star::uno::Reference<
                                       com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    removeVetoableChangeListener( const rtl::OUString& PropertyName,
                                  const com::sun::star::uno::Reference<
                                      com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // own methods
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        impl_disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_propertyChange( const com::sun::star::beans::PropertyChangeEvent& evt )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_vetoableChange( const com::sun::star::beans::PropertyChangeEvent& aEvent )
        throw( com::sun::star::beans::PropertyVetoException,
               com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XContentAccess
    //-----------------------------------------------------------------
    virtual rtl::OUString SAL_CALL
    queryContentIdentfierString()
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier()
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    queryContent()
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XResultSet
    //-----------------------------------------------------------------
    virtual sal_Bool SAL_CALL
    next()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isBeforeFirst()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isAfterLast()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isFirst()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isLast()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    beforeFirst()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    afterLast()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    first()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    last()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    getRow()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    absolute( sal_Int32 row )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    relative( sal_Int32 rows )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    previous()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    refreshRow()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowUpdated()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowInserted()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowDeleted()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::uno::XInterface > SAL_CALL
    getStatement()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XRow
    //-----------------------------------------------------------------
    virtual sal_Bool SAL_CALL
    wasNull()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL
    getString( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const com::sun::star::uno::Reference<
                   com::sun::star::container::XNameAccess >& typeMap )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
};

//=========================================================================

class ContentResultSetWrapperListener
        : public cppu::OWeakObject
        , public com::sun::star::beans::XPropertyChangeListener
        , public com::sun::star::beans::XVetoableChangeListener
{
protected:
    ContentResultSetWrapper*    m_pOwner;

public:
    ContentResultSetWrapperListener( ContentResultSetWrapper* pOwner );

    virtual ~ContentResultSetWrapperListener();

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    //XEventListener
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    //XPropertyChangeListener
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    propertyChange( const com::sun::star::beans::PropertyChangeEvent& evt )
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    //XVetoableChangeListener
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    vetoableChange( const com::sun::star::beans::PropertyChangeEvent& aEvent )
        throw( com::sun::star::beans::PropertyVetoException,
               com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // own methods:
    void SAL_CALL impl_OwnerDies();
};

#endif

