/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertystatecontainer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:36:52 $
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

#ifndef COMPHELPER_PROPERTYSTATECONTAINER_HXX
#define COMPHELPER_PROPERTYSTATECONTAINER_HXX

#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

#include <map>

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= OPropertyStateContainer
    //=====================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::beans::XPropertyState
                                >   OPropertyStateContainer_TBase;

    /** helper implementation for components which have properties with a default

        <p>This class is not intended for direct use, you need to derive from it.</p>

        @see com.sun.star.beans.XPropertyState
    */
    class COMPHELPER_DLLPUBLIC OPropertyStateContainer
                :public  OPropertyContainer
                ,public  OPropertyStateContainer_TBase
    {
    protected:
        /** ctor
            @param _rBHelper
                help to be used for broadcasting events
        */
        OPropertyStateContainer( ::cppu::OBroadcastHelper&  _rBHelper );

        // ................................................................
        // XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ................................................................
        // own overridables
        // these are the impl-methods for the XPropertyState members - they are implemented already by this class,
        // but you may want to override them for whatever reasons (for instance, if your derived class
        // supports the AMBIGUOUS state for properties)

        /** get the PropertyState of the property denoted by the given handle

            <p>Already implemented by this base class, no need to override</p>
            @precond <arg>_nHandle</arg> is a valid property handle
        */
        virtual ::com::sun::star::beans::PropertyState  getPropertyStateByHandle( sal_Int32 _nHandle );

        /** set the property denoted by the given handle to it's default value

            <p>Already implemented by this base class, no need to override</p>
            @precond <arg>_nHandle</arg> is a valid property handle
        */
        virtual void                                    setPropertyToDefaultByHandle( sal_Int32 _nHandle );

        /** get the default value for the property denoted by the given handle

            <p>Already implemented by this base class, no need to override</p>
            @precond <arg>_nHandle</arg> is a valid property handle
        */
        virtual ::com::sun::star::uno::Any              getPropertyDefaultByHandle( sal_Int32 _nHandle ) const = 0;

    protected:
        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

    protected:
        sal_Int32   getHandleForName( const ::rtl::OUString& _rPropertyName ) SAL_THROW( ( ::com::sun::star::beans::UnknownPropertyException ) );
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_PROPERTYSTATECONTAINER_HXX
