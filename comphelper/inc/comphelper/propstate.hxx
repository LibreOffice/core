/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propstate.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:13:28 $
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

#ifndef _COMPHELPER_PROPERTY_STATE_HXX_
#define _COMPHELPER_PROPERTY_STATE_HXX_

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif


#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _CPPUHELPER_PROPTYPEHLP_HXX
#include <cppuhelper/proptypehlp.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

//=========================================================================
//= property helper classes
//=========================================================================

//.........................................................................
namespace comphelper
{
//.........................................................................

    //==================================================================
    //= OPropertyStateHelper
    //==================================================================
    /// helper class for implementing property states
    class COMPHELPER_DLLPUBLIC OPropertyStateHelper :public ::cppu::OPropertySetHelper
                                                    ,public ::com::sun::star::beans::XPropertyState
    {
    public:
        OPropertyStateHelper(::cppu::OBroadcastHelper& rBHlp):OPropertySetHelper(rBHlp) { }
        OPropertyStateHelper(::cppu::OBroadcastHelper& rBHlp,
                             ::cppu::IEventNotificationHook *i_pFireEvents);

        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type& aType) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL
            getPropertyState(const ::rtl::OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> SAL_CALL
            getPropertyStates(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL
            setPropertyToDefault(const ::rtl::OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL
            getPropertyDefault(const ::rtl::OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // access via handle
        virtual ::com::sun::star::beans::PropertyState  getPropertyStateByHandle(sal_Int32 nHandle);
        virtual void                                    setPropertyToDefaultByHandle(sal_Int32 nHandle);
        virtual ::com::sun::star::uno::Any              getPropertyDefaultByHandle(sal_Int32 nHandle) const;

    protected:
        virtual ~OPropertyStateHelper();

        void firePropertyChange(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aNewValue, const ::com::sun::star::uno::Any& aOldValue);

    protected:
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    };

    //==================================================================
    //= OPropertyStateHelper
    //==================================================================
    class COMPHELPER_DLLPUBLIC OStatefulPropertySet  :public ::cppu::OWeakObject
                                ,public ::com::sun::star::lang::XTypeProvider
                                ,public OMutexAndBroadcastHelper    // order matters: before OPropertyStateHelper/OPropertySetHelper
                                ,public OPropertyStateHelper
    {
    protected:
        OStatefulPropertySet();
        virtual ~OStatefulPropertySet();

    protected:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_PROPERTY_STATE_HXX_

