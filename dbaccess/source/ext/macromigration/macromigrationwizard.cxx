/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macromigrationwizard.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:04:31 $
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

#include "dbmm_module.hxx"
#include "macromigrationdialog.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <svtools/genericunodialog.hxx>

//........................................................................
namespace dbmm
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
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::lang::IllegalArgumentException;
    /** === end UNO using === **/

    //====================================================================
    //= MacroMigrationDialogService
    //====================================================================
    class MacroMigrationDialogService;
    typedef ::svt::OGenericUnoDialog                                                MacroMigrationDialogService_Base;
    typedef ::comphelper::OPropertyArrayUsageHelper< MacroMigrationDialogService >  MacroMigrationDialogService_PBase;

    class MacroMigrationDialogService
                :public MacroMigrationDialogService_Base
                ,public MacroMigrationDialogService_PBase
                ,public MacroMigrationModuleClient
    {
    public:
        MacroMigrationDialogService( const Reference< XComponentContext >& _rxContext );

        // XTypeProvider
        virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments ) throw(com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual Reference< XPropertySetInfo >  SAL_CALL getPropertySetInfo() throw(RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // helper for factories
        static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& _rxContext );
        static ::rtl::OUString SAL_CALL getImplementationName_static() throw(RuntimeException);
        static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static() throw(RuntimeException);

    protected:
        ~MacroMigrationDialogService();

    protected:
        virtual Dialog* createDialog( Window* _pParent );
        virtual void destroyDialog();

    private:
        ::comphelper::ComponentContext          m_aContext;
        Reference< XOfficeDatabaseDocument >    m_xDocument;
    };

    //====================================================================
    //= MacroMigrationDialogService
    //====================================================================
    //--------------------------------------------------------------------
    MacroMigrationDialogService::MacroMigrationDialogService( const Reference< XComponentContext >& _rxContext )
        :MacroMigrationDialogService_Base( _rxContext )
        ,m_aContext( _rxContext )
    {
        m_bNeedInitialization = true;
    }

    //--------------------------------------------------------------------
    MacroMigrationDialogService::~MacroMigrationDialogService()
    {
        // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
        // so this virtual-method-call the base class does does not work, we're already dead then ...
        if ( m_pDialog )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pDialog )
                destroyDialog();
        }
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL MacroMigrationDialogService::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *(new MacroMigrationDialogService( _rxContext ) );
    }

    //--------------------------------------------------------------------
    Dialog* MacroMigrationDialogService::createDialog( Window* _pParent )
    {
        return new MacroMigrationDialog( _pParent, m_aContext, m_xDocument );
    }

    //--------------------------------------------------------------------
    void MacroMigrationDialogService::destroyDialog()
    {
        MacroMigrationDialogService_Base::destroyDialog();
    }

    //--------------------------------------------------------------------
    Sequence< sal_Int8 > SAL_CALL MacroMigrationDialogService::getImplementationId() throw(RuntimeException)
    {
        static ::cppu::OImplementationId* pId = NULL;
        if ( !pId )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pId )
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL MacroMigrationDialogService::getImplementationName_static() throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.dbaccess.macromigration.MacroMigrationDialogService" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL MacroMigrationDialogService::getSupportedServiceNames_static() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices(1);
        aServices[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.application.MacroMigrationWizard" ) );
        return aServices;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL MacroMigrationDialogService::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL MacroMigrationDialogService::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    void SAL_CALL MacroMigrationDialogService::initialize( const Sequence< Any >& _rArguments ) throw(Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_bInitialized )
            throw AlreadyInitializedException( ::rtl::OUString(), *this );

        if ( _rArguments.getLength() != 1 )
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid number of initialization arguments. Expected 1." ) ),
                    // TODO: resoource
                *this,
                1
            );

        m_xDocument.set( _rArguments[0], UNO_QUERY );
        if ( !m_xDocument.is() )
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No database document found in the initializatin arguments." ) ),
                    // TODO: resource
                *this,
                1
            );

        m_bInitialized = true;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL MacroMigrationDialogService::getPropertySetInfo() throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL MacroMigrationDialogService::getInfoHelper()
    {
        return *const_cast< MacroMigrationDialogService* >( this )->getArrayHelper();
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* MacroMigrationDialogService::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //--------------------------------------------------------------------
    void createRegistryInfo_MacroMigrationDialogService()
    {
        static OAutoRegistration< MacroMigrationDialogService > aAutoRegistration;
    }

//........................................................................
} // namespace dbmm
//........................................................................
