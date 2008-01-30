/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dataaccessdescriptor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:32:55 $
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
#include "precompiled_dbaccess.hxx"

#include "dbastrings.hrc"
#include "module_dba.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdb/XDataAccessDescriptorFactory.hpp>
/** === end UNO includes === **/

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::sdb::XDataAccessDescriptorFactory;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::PropertyValue;
    /** === end UNO using === **/

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;
    namespace CommandType = ::com::sun::star::sdb::CommandType;

    //====================================================================
    //= DataAccessDescriptor
    //====================================================================
    typedef ::comphelper::OMutexAndBroadcastHelper  DataAccessDescriptor_MutexBase;

    typedef ::cppu::WeakImplHelper1             <   XServiceInfo
                                                >   DataAccessDescriptor_TypeBase;

    typedef ::comphelper::OPropertyContainer        DataAccessDescriptor_PropertyBase;

    class DataAccessDescriptor  :public DataAccessDescriptor_MutexBase
                                ,public DataAccessDescriptor_TypeBase
                                ,public DataAccessDescriptor_PropertyBase
                                ,public ::comphelper::OPropertyArrayUsageHelper< DataAccessDescriptor >
    {
    public:
        DataAccessDescriptor( const ::comphelper::ComponentContext& _rContext );

        // UNO
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

    protected:
        ~DataAccessDescriptor();

    protected:
        // XPropertySet
        virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    private:
        ::comphelper::ComponentContext  m_aContext;

        // </properties>
        ::rtl::OUString             m_sDataSourceName;
        ::rtl::OUString             m_sDatabaseLocation;
        ::rtl::OUString             m_sConnectionResource;
        Sequence< PropertyValue >   m_aConnectionInfo;
        Reference< XConnection >    m_xActiveConnection;
        ::rtl::OUString             m_sCommand;
        sal_Int32                   m_nCommandType;
        ::rtl::OUString             m_sFilter;
        ::rtl::OUString             m_sOrder;
        ::rtl::OUString             m_sHavingClause;
        ::rtl::OUString             m_sGroupBy;
        sal_Bool                    m_bEscapeProcessing;
        Reference< XResultSet >     m_xResultSet;
        Sequence< Any >             m_aSelection;
        sal_Bool                    m_bBookmarkSelection;
        ::rtl::OUString             m_sColumnName;
        Reference< XPropertySet >   m_xColumn;
        // </properties>
    };

#define REGISTER_PROPERTY( propname, member ) \
    registerProperty( PROPERTY_##propname, PROPERTY_ID_##propname, PropertyAttribute::BOUND, &member, ::getCppuType( &member ) )

    //--------------------------------------------------------------------
    DataAccessDescriptor::DataAccessDescriptor( const ::comphelper::ComponentContext& _rContext )
        :DataAccessDescriptor_MutexBase()
        ,DataAccessDescriptor_TypeBase()
        ,DataAccessDescriptor_PropertyBase( m_aBHelper )
        ,m_aContext( _rContext )
        ,m_sDataSourceName()
        ,m_sDatabaseLocation()
        ,m_sConnectionResource()
        ,m_aConnectionInfo()
        ,m_xActiveConnection()
        ,m_sCommand()
        ,m_nCommandType( CommandType::COMMAND )
        ,m_sFilter()
        ,m_sOrder()
        ,m_sHavingClause()
        ,m_sGroupBy()
        ,m_bEscapeProcessing( sal_True )
        ,m_xResultSet()
        ,m_aSelection()
        ,m_bBookmarkSelection( sal_True )
        ,m_sColumnName()
        ,m_xColumn()
    {
        REGISTER_PROPERTY( DATASOURCENAME,      m_sDataSourceName );
        REGISTER_PROPERTY( DATABASE_LOCATION,   m_sDatabaseLocation );
        REGISTER_PROPERTY( CONNECTION_RESOURCE, m_sConnectionResource );
        REGISTER_PROPERTY( CONNECTION_INFO,     m_aConnectionInfo );
        REGISTER_PROPERTY( ACTIVE_CONNECTION,   m_xActiveConnection );
        REGISTER_PROPERTY( COMMAND,             m_sCommand );
        REGISTER_PROPERTY( COMMAND_TYPE,        m_nCommandType );
        REGISTER_PROPERTY( FILTER,              m_sFilter );
        REGISTER_PROPERTY( ORDER,               m_sOrder );
        REGISTER_PROPERTY( HAVING_CLAUSE,       m_sHavingClause );
        REGISTER_PROPERTY( GROUP_BY,            m_sGroupBy );
        REGISTER_PROPERTY( ESCAPE_PROCESSING,   m_bEscapeProcessing );
        REGISTER_PROPERTY( RESULT_SET,          m_xResultSet );
        REGISTER_PROPERTY( SELECTION,           m_aSelection );
        REGISTER_PROPERTY( BOOKMARK_SELECTION,  m_bBookmarkSelection );
        REGISTER_PROPERTY( COLUMN_NAME,         m_sColumnName );
        REGISTER_PROPERTY( COLUMN,              m_xColumn );
    }

    //--------------------------------------------------------------------
    DataAccessDescriptor::~DataAccessDescriptor()
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( DataAccessDescriptor, DataAccessDescriptor_TypeBase, DataAccessDescriptor_PropertyBase );

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( DataAccessDescriptor, DataAccessDescriptor_TypeBase, DataAccessDescriptor_PropertyBase );

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DataAccessDescriptor::getImplementationName() throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.dba.DataAccessDescriptor" ) );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL DataAccessDescriptor::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( getSupportedServiceNames() );
        const ::rtl::OUString* pStart = aServices.getConstArray();
        const ::rtl::OUString* pEnd = aServices.getConstArray() + aServices.getLength();
        return ::std::find( pStart, pEnd, rServiceName ) != pEnd;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DataAccessDescriptor::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices(1);
        aServices[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.DataAccessDescriptor" ) );
        return aServices;
    }

    //-------------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL DataAccessDescriptor::getPropertySetInfo() throw(RuntimeException)
    {
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //-------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& DataAccessDescriptor::getInfoHelper()
    {
        return *getArrayHelper();
    }

    //------------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* DataAccessDescriptor::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //====================================================================
    //= DataAccessDescriptorFactory
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   XServiceInfo
                                    ,   XDataAccessDescriptorFactory
                                    >   DataAccessDescriptorFactory_Base;
    class DataAccessDescriptorFactory : public DataAccessDescriptorFactory_Base
    {
    public:
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

        // XServiceInfo - static versions
        static Sequence< ::rtl::OUString >  getSupportedServiceNames_static(void) throw( RuntimeException );
        static ::rtl::OUString              getImplementationName_static(void) throw( RuntimeException );
        static Reference< XInterface >      Create(const Reference< XComponentContext >& _rxContext);
        static ::rtl::OUString              getSingletonName_static();

        // XDataAccessDescriptorFactory
        virtual Reference< XPropertySet > SAL_CALL createDataAccessDescriptor(  ) throw (RuntimeException);

    protected:
        DataAccessDescriptorFactory( const Reference< XComponentContext >& _rxContext );
        ~DataAccessDescriptorFactory();

    private:
        ::comphelper::ComponentContext  m_aContext;
    };

    //--------------------------------------------------------------------
    DataAccessDescriptorFactory::DataAccessDescriptorFactory( const Reference< XComponentContext >& _rxContext )
        :m_aContext( _rxContext )
    {
    }

    //--------------------------------------------------------------------
    DataAccessDescriptorFactory::~DataAccessDescriptorFactory()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString DataAccessDescriptorFactory::getSingletonName_static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.DataAccessDescriptorFactory" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > DataAccessDescriptorFactory::getSupportedServiceNames_static() throw( RuntimeException )
    {
        Sequence< ::rtl::OUString > aServices(1);
        aServices[0] = getSingletonName_static();
        return aServices;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString DataAccessDescriptorFactory::getImplementationName_static() throw( RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.dba.DataAccessDescriptorFactory" ) );
    }

    //--------------------------------------------------------------------
    Reference< XInterface > DataAccessDescriptorFactory::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new DataAccessDescriptorFactory( _rxContext ) );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DataAccessDescriptorFactory::getImplementationName() throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL DataAccessDescriptorFactory::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( getSupportedServiceNames_static() );
        const ::rtl::OUString* pStart = aServices.getConstArray();
        const ::rtl::OUString* pEnd = aServices.getConstArray() + aServices.getLength();
        return ::std::find( pStart, pEnd, rServiceName ) != pEnd;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DataAccessDescriptorFactory::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > SAL_CALL DataAccessDescriptorFactory::createDataAccessDescriptor(  ) throw (RuntimeException)
    {
        return new DataAccessDescriptor( m_aContext );
    }

//........................................................................
} // namespace dbaccess
//........................................................................

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_DataAccessDescriptorFactory()
{
    static ::dba::OSingletonRegistration< ::dbaccess::DataAccessDescriptorFactory > aAutoRegistration;
}
