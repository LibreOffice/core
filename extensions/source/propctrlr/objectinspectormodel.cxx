/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectinspectormodel.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 10:49:12 $
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
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_PROPCTRLR_MODULEPRC_HXX
#include "modulepcr.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif
#ifndef INSPECTORMODELBASE_HXX
#include "inspectormodelbase.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UCB_ALREADYINITIALIZEDEXCEPTION_HPP_
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
/** === end UNO includes === **/

#include <cppuhelper/implbase3.hxx>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::inspection::XObjectInspectorModel;
    using ::com::sun::star::lang::XInitialization;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::inspection::PropertyCategoryDescriptor;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::makeAny;
    /** === end UNO using === **/

    //====================================================================
    //= ObjectInspectorModel
    //====================================================================
    class ObjectInspectorModel : public ImplInspectorModel
    {
    private:
        Sequence< Any >             m_aFactories;

    public:
        ObjectInspectorModel( const Reference< XComponentContext >& _rxContext );

        // XObjectInspectorModel
        virtual Sequence< Any > SAL_CALL getHandlerFactories() throw (RuntimeException);
        virtual Sequence< PropertyCategoryDescriptor > SAL_CALL describeCategories(  ) throw (RuntimeException);
        virtual ::sal_Int32 SAL_CALL getPropertyOrderIndex( const ::rtl::OUString& PropertyName ) throw (RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(RuntimeException);
        static Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(RuntimeException);
        static Reference< XInterface > SAL_CALL
                        Create(const Reference< XComponentContext >&);

    protected:
        void    createDefault();
        void    createWithHandlerFactories( const Sequence< Any >& _rFactories );
        void    createWithHandlerFactoriesAndHelpSection( const Sequence< Any >& _rFactories, sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );

    private:
        /** checks a given condition to be <TRUE/>, and throws an IllegalArgumentException if not
        */
        void    impl_verifyArgument_throw( bool _bCondition, sal_Int16 _nArgumentPosition );
    };

    //====================================================================
    //= ObjectInspectorModel
    //====================================================================
    ObjectInspectorModel::ObjectInspectorModel( const Reference< XComponentContext >& _rxContext )
        :ImplInspectorModel( _rxContext )
    {
    }

    //--------------------------------------------------------------------
    Sequence< Any > SAL_CALL ObjectInspectorModel::getHandlerFactories() throw (RuntimeException)
    {
        return m_aFactories;
    }

    //--------------------------------------------------------------------
    Sequence< PropertyCategoryDescriptor > SAL_CALL ObjectInspectorModel::describeCategories(  ) throw (RuntimeException)
    {
        // no category info provided by this default implementation
        return Sequence< PropertyCategoryDescriptor >( );
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL ObjectInspectorModel::getPropertyOrderIndex( const ::rtl::OUString& /*PropertyName*/ ) throw (RuntimeException)
    {
        // no ordering provided by this default implementation
        return 0;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ObjectInspectorModel::initialize( const Sequence< Any >& _arguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_aFactories.getLength() )
            throw AlreadyInitializedException();

        StlSyntaxSequence< Any > arguments( _arguments );
        if ( arguments.empty() )
        {   // constructor: "createDefault()"
            createDefault();
            return;
        }

        Sequence< Any > factories;
        impl_verifyArgument_throw( arguments[0] >>= factories, 1 );

        if ( arguments.size() == 1 )
        {   // constructor: "createWithHandlerFactories( any[] )"
            createWithHandlerFactories( factories );
            return;
        }

        sal_Int32 nMinHelpTextLines( 0 ), nMaxHelpTextLines( 0 );
        if ( arguments.size() == 3 )
        {   // constructor: "createWithHandlerFactoriesAndHelpSection( any[], long, long )"
            impl_verifyArgument_throw( arguments[1] >>= nMinHelpTextLines, 2 );
            impl_verifyArgument_throw( arguments[2] >>= nMaxHelpTextLines, 3 );
            createWithHandlerFactoriesAndHelpSection( factories, nMinHelpTextLines, nMaxHelpTextLines );
            return;
        }

        impl_verifyArgument_throw( false, 2 );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ObjectInspectorModel::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ObjectInspectorModel::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString ObjectInspectorModel::getImplementationName_static(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.extensions.ObjectInspectorModel" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > ObjectInspectorModel::getSupportedServiceNames_static(  ) throw(RuntimeException)
    {
        ::rtl::OUString sService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.inspection.ObjectInspectorModel" ) );
        return Sequence< ::rtl::OUString >( &sService, 1 );
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ObjectInspectorModel::Create(const Reference< XComponentContext >& _rxContext )
    {
        return *( new ObjectInspectorModel( _rxContext ) );
    }

    //--------------------------------------------------------------------
    void ObjectInspectorModel::createDefault()
    {
        m_aFactories.realloc( 1 );
        m_aFactories[0] <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.inspection.GenericPropertyHandler" ) );
    }

    //--------------------------------------------------------------------
    void ObjectInspectorModel::createWithHandlerFactories( const Sequence< Any >& _rFactories )
    {
        impl_verifyArgument_throw( _rFactories.getLength() > 0, 1 );
        m_aFactories = _rFactories;
    }

    //--------------------------------------------------------------------
    void ObjectInspectorModel::createWithHandlerFactoriesAndHelpSection( const Sequence< Any >& _rFactories, sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines )
    {
        impl_verifyArgument_throw( _rFactories.getLength() > 0, 1 );
        impl_verifyArgument_throw( _nMinHelpTextLines >= 1, 2 );
        impl_verifyArgument_throw( _nMaxHelpTextLines >= 1, 3 );
        impl_verifyArgument_throw( _nMinHelpTextLines <= _nMaxHelpTextLines, 2 );

        m_aFactories = _rFactories;
        enableHelpSectionProperties( _nMinHelpTextLines, _nMaxHelpTextLines );
    }

    //--------------------------------------------------------------------
    void ObjectInspectorModel::impl_verifyArgument_throw( bool _bCondition, sal_Int16 _nArgumentPosition )
    {
        if ( !_bCondition )
            throw IllegalArgumentException( ::rtl::OUString(), *this, _nArgumentPosition );
    }

//........................................................................
} // namespace pcr
//........................................................................

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ObjectInspectorModel()
{
    ::pcr::OAutoRegistration< ::pcr::ObjectInspectorModel > aObjectInspectorModelRegistration;
}
