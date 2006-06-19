/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statusindicatorinterfacewrapper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:05:50 $
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

#ifndef __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include <vector>

namespace framework
{

class StatusIndicatorInterfaceWrapper :   public ::com::sun::star::lang::XTypeProvider       ,
                                          public ::com::sun::star::task::XStatusIndicator    ,
                                          public ::cppu::OWeakObject
{
    public:
        StatusIndicatorInterfaceWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rStatusIndicatorImpl );
        virtual ~StatusIndicatorInterfaceWrapper();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider
        //---------------------------------------------------------------------------------------------------------
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //---------------------------------------------------------------------------------------------------------
        //  XStatusIndicator
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL start   ( const ::rtl::OUString& sText  ,
                                              sal_Int32        nRange ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL end     (                               ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL reset   (                               ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL setText ( const ::rtl::OUString& sText  ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL setValue(       sal_Int32        nValue ) throw( ::com::sun::star::uno::RuntimeException );

    private:
        StatusIndicatorInterfaceWrapper();

        ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XComponent > m_xStatusIndicatorImpl;
};

}

#endif // __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_
