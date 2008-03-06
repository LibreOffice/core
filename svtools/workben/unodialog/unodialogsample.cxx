/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unodialogsample.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:29:51 $
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

#include "udlg_module.hxx"
#include "roadmapskeleton.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include "svtools/genericunodialog.hxx"

//........................................................................
namespace udlg
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
    /** === end UNO using === **/

    //====================================================================
    //= UnoDialogSkeleton
    //====================================================================
    class UnoDialogSkeleton;
    typedef ::svt::OGenericUnoDialog                                                UnoDialogSkeleton_Base;
    typedef ::comphelper::OPropertyArrayUsageHelper< UnoDialogSkeleton >  UnoDialogSkeleton_PBase;

    class UnoDialogSkeleton
                :public UnoDialogSkeleton_Base
                ,public UnoDialogSkeleton_PBase
                ,public UdlgClient
    {
    public:
        UnoDialogSkeleton( const Reference< XComponentContext >& _rxContext );

        // XTypeProvider
        virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

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
        ~UnoDialogSkeleton();

    protected:
        virtual Dialog* createDialog( Window* _pParent );
        virtual void destroyDialog();

    private:
        ::comphelper::ComponentContext  m_aContext;
    };

    //====================================================================
    //= UnoDialogSkeleton
    //====================================================================
    //--------------------------------------------------------------------
    UnoDialogSkeleton::UnoDialogSkeleton( const Reference< XComponentContext >& _rxContext )
        :UnoDialogSkeleton_Base( _rxContext )
        ,m_aContext( _rxContext )
    {
    }

    //--------------------------------------------------------------------
    UnoDialogSkeleton::~UnoDialogSkeleton()
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
    Reference< XInterface > SAL_CALL UnoDialogSkeleton::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *(new UnoDialogSkeleton( _rxContext ) );
    }

    //--------------------------------------------------------------------
    Dialog* UnoDialogSkeleton::createDialog( Window* _pParent )
    {
        return new RoadmapSkeletonDialog( m_aContext, _pParent );
    }

    //--------------------------------------------------------------------
    void UnoDialogSkeleton::destroyDialog()
    {
        UnoDialogSkeleton_Base::destroyDialog();
    }

    //--------------------------------------------------------------------
    Sequence< sal_Int8 > SAL_CALL UnoDialogSkeleton::getImplementationId() throw(RuntimeException)
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
    ::rtl::OUString SAL_CALL UnoDialogSkeleton::getImplementationName_static() throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svtools.workben.UnoDialogSkeleton" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoDialogSkeleton::getSupportedServiceNames_static() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices(1);
        aServices[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.example.UnoDialogSample" ) );
        return aServices;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoDialogSkeleton::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoDialogSkeleton::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL UnoDialogSkeleton::getPropertySetInfo() throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL UnoDialogSkeleton::getInfoHelper()
    {
        return *const_cast< UnoDialogSkeleton* >( this )->getArrayHelper();
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* UnoDialogSkeleton::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //--------------------------------------------------------------------
    void createRegistryInfo_UnoDialogSkeleton()
    {
        static OAutoRegistration< UnoDialogSkeleton > aAutoRegistration;
    }

//........................................................................
} // namespace udlg
//........................................................................
