/*************************************************************************
 *
 *  $RCSfile: RowSetBase.hxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:01:00 $
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
#ifndef DBACCESS_CORE_API_ROWSETBASE_HXX
#define DBACCESS_CORE_API_ROWSETBASE_HXX

#ifndef _CPPUHELPER_IMPLBASE10_HXX_
#include <cppuhelper/implbase10.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef COMPHELPER_PROPERTYSTATECONTAINER_HXX
#include <comphelper/propertystatecontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#include "RowSetRow.hxx"
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef DBACCESS_ROWSETCACHEITERATOR_HXX
#include "RowSetCacheIterator.hxx"
#endif
#include <functional>



namespace com { namespace sun { namespace star {
    namespace sdb { struct RowChangeEvent; }
    namespace lang { struct Locale; }
} } }

namespace dbaccess
{
    class OEmptyCollection;

    typedef ::cppu::ImplHelper10<               ::com::sun::star::sdbcx::XRowLocate,
                                                ::com::sun::star::sdbc::XRow,
                                                ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                                ::com::sun::star::sdbc::XWarningsSupplier,
                                                ::com::sun::star::sdbc::XColumnLocate,
                                                ::com::sun::star::sdbcx::XColumnsSupplier,
                                                ::com::sun::star::lang::XServiceInfo,
                                                ::com::sun::star::sdbc::XRowSet,
                                                ::com::sun::star::sdbc::XCloseable,
                                                ::com::sun::star::lang::XUnoTunnel> ORowSetBase_BASE;

    class ORowSetCache;
    class ORowSetDataColumns;
    class ORowSetCacheIterator;
    class ORowSetDataColumn;
    class ORowSetBase : public ORowSetBase_BASE,
                        public ::comphelper::OPropertyStateContainer,
                        public ::comphelper::OPropertyArrayUsageHelper<ORowSetBase> // this class hold the static property info
    {
    protected:
        typedef ::std::vector<ORowSetDataColumn*>   TDataColumns;
        ::osl::Mutex*                           m_pMutex;           // this the mutex form the rowset itself
        ::osl::Mutex                            m_aRowCountMutex, // mutex for rowcount changes
                                                // we need a extra mutex for columns to prevend deadlock when setting new values
                                                // for a row
                                                m_aColumnsMutex;

        ::com::sun::star::uno::Any              m_aBookmark;
        ORowSetCacheIterator                    m_aCurrentRow;      // contains the actual fetched row
        TORowSetOldRowHelperRef                 m_aOldRow;
        TDataColumns                            m_aDataColumns;     // holds the columns as m_pColumns but know the implementation class
        connectivity::ORowSetValue              m_aEmptyValue;      // only for error case

        ::cppu::OWeakObject*                    m_pMySelf;          // set by derived classes
        ORowSetCache*                           m_pCache;           // the cache is used by the rowset and his clone (shared)
        ORowSetDataColumns*                     m_pColumns;         // represent the select columns
        ::cppu::OBroadcastHelper&               m_rBHelper;         // must be set from the derived classes
        // is used when the formatkey for database types is set
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes>   m_xNumberFormatTypes;
        OEmptyCollection*                                                               m_pEmptyCollection;

        sal_Int32                               m_nRowCount;        // contains the current count of rows which have been fetched
        sal_Int32                               m_nLastColumnIndex; // the last column ask for, used for wasNull()
        sal_Int32                               m_nPosition;        // is set only when a row was deleted
        sal_Int32                               m_nResultSetType;   // fetch property
        sal_Int32                               m_nResultSetConcurrency;
        sal_Bool                                m_bClone;           // I'm clone or not
        sal_Bool                                m_bRowCountFinal;
        sal_Bool                                m_bIgnoreResult ;
        sal_Bool                                m_bBeforeFirst  : 1;
        sal_Bool                                m_bAfterLast    : 1;

    protected:
        ORowSetBase(::cppu::OBroadcastHelper    &_rBHelper,::osl::Mutex* _pMutex);

        // fire a notification for all that are listening on column::VALUE property
        void firePropertyChange(const ORowSetRow& _rOldRow);
        virtual void fireRowcount() { }                             // fire if rowcount changed
        virtual sal_Bool notifyAllListenersRowBeforeChange(::osl::ResettableMutexGuard& _rGuard,const ::com::sun::star::sdb::RowChangeEvent &rEvt)
            {return sal_True; }                                                     // fire if rowcount changed
        virtual void notifyAllListenersRowChanged(::osl::ResettableMutexGuard& _rGuard,const ::com::sun::star::sdb::RowChangeEvent &rEvt)
            {}                                                      // notify row changed
        virtual sal_Bool notifyAllListenersCursorBeforeMove(::osl::ResettableMutexGuard& _rGuard) {return sal_True; }       // notify row changed

        virtual void notifyAllListenersCursorMoved(::osl::ResettableMutexGuard& _rGuard) { }            // notify cursor moved
        virtual void notifyAllListeners(::osl::ResettableMutexGuard& _rGuard) { }                       // notify all that rowset changed

        // cancel the insertion, if necessary (means if we're on the insert row)
        virtual void        doCancelModification( ) = 0;
        // return <TRUE/> if and only if we're using the insert row (means: we're updating _or_ inserting)
        virtual sal_Bool    isModification( ) = 0;
        // return <TRUE/> if and only if the current row is modified
        // TODO: isn't this the same as isModification?
        virtual sal_Bool    isModified( ) = 0;
        // return <TRUE/> if and only if the current row is the insert row
        virtual sal_Bool    isNew( ) = 0;
        // notify the change of a boolean property
        void fireProperty( sal_Int32 _nProperty, sal_Bool _bNew, sal_Bool _bOld );

    // OPropertyStateContainer
        virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const;
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue,sal_Int32 nHandle) const;
        // postions the cache which the currently bookmark m_aBookmark
        void positionCache();
        // returns a value of a column of the current row
        const connectivity::ORowSetValue& getValue(sal_Int32 columnIndex);
        // sets the current and the bookmark
        void setCurrentRow(sal_Bool _bMoved,const ORowSetRow& _rOldValues,::osl::ResettableMutexGuard& _rGuard);
        void checkPositioningAllowed() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // checks  if the cache is null
        void checkCache();
        // sets the bookmark to Any()
        // m_aCurrentRow to end of matrix
        // m_aOldRow to NULL
        void movementFailed();

        ORowSetRow getOldRow(sal_Bool _bWasNew);
        /** move the cache the postion defined by the member functor
            @param  _aCheckFunctor
                Return <TRUE/> when we already stand on the row we want to.
            @param  _aMovementFunctor
                The mehtod used to move.
            @return
                <TRUE/> if movement was successful.
        */
        sal_Bool SAL_CALL move( ::std::mem_fun_t<sal_Bool,ORowSetBase>& _aCheckFunctor,
                                ::std::mem_fun_t<sal_Bool,ORowSetCache>& _aMovementFunctor);

        /** same meaning as isFirst. Only need by mem_fun
            @return
                <TRUE/> if so.
        */
        sal_Bool isOnFirst();
        /** same meaning as isLast. Only need by mem_fun
            @return
                <TRUE/> if so.
        */
        sal_Bool isOnLast();

    public:
        virtual ~ORowSetBase();

    // OComponentHelper
        virtual void SAL_CALL disposing(void);

    // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
        {
            return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
        }

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XWarningsSupplier
        virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XResultSetMetaDataSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XColumnLocate
        virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XColumnsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XResultSet
        virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAfterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL beforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL afterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL first(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL last(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL previous(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRowSet
        virtual void SAL_CALL execute(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {   // when not implemented by derived classes then throw
            throw ::com::sun::star::sdbc::SQLException();
        }
        virtual void SAL_CALL addRowSetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >& listener ) throw(::com::sun::star::uno::RuntimeException)
        {   // when not implemented by derived classes then throw
            throw ::com::sun::star::sdbc::SQLException();
        }
        virtual void SAL_CALL removeRowSetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >& listener ) throw(::com::sun::star::uno::RuntimeException)
        {   // when not implemented by derived classes then throw
            throw ::com::sun::star::sdbc::SQLException();
        }

        // ==========================================================
        // granular access control
        struct GrantNotifierAccess { friend class ORowSetNotifier; private: GrantNotifierAccess () { } };

        // cancel the insertion, if necessary (means if we're on the insert row)
        inline  void        doCancelModification( const GrantNotifierAccess& ) { doCancelModification(); }
        inline  sal_Bool    isModification( const GrantNotifierAccess& ) { return isModification(); }
        inline  sal_Bool    isModified( const GrantNotifierAccess& ) { return isModified(); }
        inline  sal_Bool    isNew( const GrantNotifierAccess& ) { return isNew(); }
        inline  void        fireProperty( sal_Int32 _nProperty, sal_Bool _bNew, sal_Bool _bOld, const GrantNotifierAccess& )
        {
            fireProperty( _nProperty, _bNew, _bOld );
        }
    };

    // ========================================================================
    /** eases the handling of the doCancelModification and notifyCancelInsert methods

        <p>The class can only be used on the stack, within a method of ORowSetBase (or derivees)</p>
    */
    class ORowSetNotifier
    {
    private:
        ORowSetBase*    m_pRowSet;
            // not aquired! This is not necessary because this class here is to be used on the stack within
            // a method of ORowSetBase (or derivees)
        sal_Bool        m_bWasNew;
        sal_Bool        m_bWasModified;
#ifdef DBG_UTIL
        sal_Bool        m_bNotifyCalled;
#endif

    public:
        /** constructs the object, and cancels the insertion

            @see ORowSetBase::doCancelModification
        */
        ORowSetNotifier( ORowSetBase* m_pRowSet );

        // destructs the object. <member>fire</member> has to be called before.
        ~ORowSetNotifier( );

        /** notifies the insertion

            <p>This has <em>not</em> been put into the destructor by intention!<br/>

            The destructor is called during stack unwinding in case of an exception, so if we would do
            listener notification there, this would have the potential of another exception during stack
            unwinding, which would terminate the application.</p>

            @see ORowSetBase::notifyCancelInsert
        */
        void    fire();
    };

} // end of namespace

#endif // DBACCESS_CORE_API_ROWSETBASE_HXX

