/*************************************************************************
 *
 *  $RCSfile: resultset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-31 09:50:50 $
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

#ifndef _UCBHELPER_RESULTSET_HXX
#define _UCBHELPER_RESULTSET_HXX

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_RESULTSETEXCEPTION_HPP_
#include <com/sun/star/ucb/ResultSetException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

namespace ucb {

//=========================================================================

#define RESULTSET_SERVICE_NAME  "com.sun.star.ucb.ContentResultSet"

//=========================================================================

class ResultSetDataSupplier;
struct ResultSet_Impl;

/**
 * This is an implementation of the service com.sun.star.ucb.ContentResultSet.
 * It can be used to implement the method XDynamicResultSet::getStaticResultSet,
 * which needs to be implemented for instance to implement the command "open"
 * at folder objects. This class uses a user supplied ResultSetDataSupplier
 * object to request data on demand.
 *
 * @see ResultSetDataSupplier
 */
class ResultSet :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::lang::XComponent,
                public com::sun::star::ucb::XContentAccess,
                public com::sun::star::sdbc::XResultSet,
                public com::sun::star::sdbc::XResultSetMetaDataSupplier,
                public com::sun::star::sdbc::XRow,
                public com::sun::star::sdbc::XCloseable,
                public com::sun::star::beans::XPropertySet
{
    ResultSet_Impl* m_pImpl;

public:
    /**
      * Construction.
      *
      * @param rxSMgr is a Service Manager.
      * @param rProperties is a sequence of properties for that the resultset
      *        shall be able to obtain the values.
      * @param rDataSupplier is a supplier for the resultset data.
      */
    ResultSet(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const com::sun::star::uno::Sequence<
                com::sun::star::beans::Property >& rProperties,
            const vos::ORef< ResultSetDataSupplier >& rDataSupplier );
    virtual ~ResultSet();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XContentAccess
    virtual rtl::OUString SAL_CALL
#if SUPD>611
    queryContentIdentifierString()
#else
    queryContentIdentfierString()
#endif
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    queryContent()
        throw( com::sun::star::uno::RuntimeException );

    // XResultSetMetaDataSupplier
    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSetMetaData > SAL_CALL
    getMetaData()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    // XResultSet
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

    // XRow
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

    // XCloseable
    virtual void SAL_CALL
    close()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    // XPropertySet
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

    /////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    /////////////////////////////////////////////////////////////////////

    /**
      * This method propagates property value changes to all registered
      * listeners.
      *
      * @param rEvt is a property change event.
      */
    void propertyChanged(
                const com::sun::star::beans::PropertyChangeEvent& rEvt );

    /**
      * This method should be called by the data supplier for the result set
      * to indicate that there were new data obtained from the data source.
      *
      * @param nOld is the old count of rows.
      * @param nnew is the new count of rows.
      */
    void rowCountChanged( sal_uInt32 nOld, sal_uInt32 nNew );

    /**
      * This method should be called by the data supplier for the result set
      * to indicate that there were all rows obtained from the data source.
      */
    void rowCountFinal();

    /**
      * This method returns a sequence containing all properties ( not the
      * values! ) of the result set.
      *
      * @return a sequence of properties.
      */
    const com::sun::star::uno::Sequence< com::sun::star::beans::Property >&
    getProperties();
};

//=========================================================================

/**
 * This is the base class for an object that supplies data to a result set
 *
 * @see ResultSet
 */
class ResultSetDataSupplier : public vos::OReference
{
    friend class ResultSet;

    // No ref, otherwise we get a cyclic reference between supplier and set!
    // Will be set from ResultSet ctor.
    ResultSet* m_pResultSet;

public:
    ResultSetDataSupplier() : m_pResultSet( 0 ) {}

    /**
     * This method returns the resultset this supplier belongs to.
     *
     * @return the resultset for that the supplier supplies data.
     */
    vos::ORef< ResultSet > getResultSet() const { return m_pResultSet; }

    /**
     * This method returns the identifier string of the content at the
     * specified index.
     *
     * @param nIndex is the zero-based index within the logical data array
     *               of the supplier.
     * @return the content's identifier string.
     */
    virtual rtl::OUString queryContentIdentifierString( sal_uInt32 nIndex ) = 0;

    /**
     * This method returns the identifier of the content at the specified index.
     *
     * @param nIndex is the zero-based index within the logical data array
     *               of the supplier.
     * @return the content's identifier.
     */
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >
    queryContentIdentifier( sal_uInt32 nIndex ) = 0;

    /**
     * This method returns the the content at the specified index.
     *
     * @param nIndex is the zero-based index within the logical data array
     *               of the supplier.
     * @return the content.
     */
    virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent >
    queryContent( sal_uInt32 nIndex ) = 0;

    /**
     * This method returns whether there is a content at the specified index.
     *
     * @param nIndex is the zero-based index within the logical data array
     *               of the supplier.
     * @return true, if there is a content at the given index.
     */
    virtual sal_Bool getResult( sal_uInt32 nIndex ) = 0;

    /**
     * This method returns the total count of objects in the logical data array
     * of the supplier. The implementation of this method may be very
     * "expensive", because it can be necessary to obtain all data in order
     * to determine the count. Therefor the ResultSet implementation calls
     * it very seldom.
     *
     * @return the total count of objects.
     */
    virtual sal_uInt32 totalCount() = 0;

    /**
     * This method returns the count of objects obtained so far. There is no
     * for the implemetation to obtain all objects at once. It can obtain
     * all data on demand.
     *
     * The implementation should call m_pResultSet->rowCountChanged(...)
     * everytime it has inserted a new entry in its logical result array.
     *
     * @return the count of objects obtained so far.
     */
    virtual sal_uInt32 currentCount() = 0;

    /**
     * This method returns whether the value returned by currentCount() is
     * "final". This is the case, if that there was all data obtained by the
     * supplier and the current count won't increase any more.
     *
     * The implementation should call m_pResultSet->rowCountFinal(...) if
     * it has inserted all entries in its logical result array.
     *
     * @return true, if the the value returned by currentCount() won't change
               anymore.
     */
    virtual sal_Bool isCountFinal() = 0;

    /**
     * This method returns an object for accessing the property values at
     * the specified index. The implementation may use the helper class
     * ucb::PropertyValueSet to provide the return value.
     *
     * @param nIndex is the zero-based index within the logical data array
     *               of the supplier.
     * @return the object for accessing the property values.
     */
    virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
    queryPropertyValues( sal_uInt32 nIndex  ) = 0;

    /**
     * This method is called to instruct the supplier to release the (possibly
     * presnt) property values at the given index.
     *
     * @param nIndex is the zero-based index within the logical data array
     *               of the supplier.
     */
    virtual void releasePropertyValues( sal_uInt32 nIndex ) = 0;

    /**
     * This method will be called by the resultset implementation in order
     * to instruct the data supplier to release all resources it has
     * allocated so far. In case the supplier is collecting data
     * asynchronously, that process must be stopped.
     */
    virtual void close() = 0;

    /**
     * This method will be called by the resultset implementation in order
     * check, whether an error has occured while collecting data. The
     * implementation of this method must throw an exception in that case.
     *
     * Note: An exception thrown to indicate an error must always be thrown
     * by the thread that created the data supplier. If the supplier collects
     * data asynchronously ( i.e. in a separate thread ) and an error
     * occures, throwing of the appropriate exception must be deferred
     * until validate() is called by the ResultSet implementation from
     * inside the main thread.
     * In case data are obtained synchronously, the ResultSetException can
     * be thrown directly.
     *
     * @exception ResultSetException thrown, if an error has occured
     */
    virtual void validate()
        throw( com::sun::star::ucb::ResultSetException ) = 0;
};

}

#endif /* !_UCBHELPER_RESULTSET_HXX */
