/*************************************************************************
 *
 *  $RCSfile: uielementwrapperbase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 16:49:57 $
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

#ifndef __FRAMEWORK_HELPER_UIELEMENTWRAPPERBASE_HXX_
#define __FRAMEWORK_HELPER_UIELEMENTWRAPPERBASE_HXX_

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

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUIELEMENT_HPP_
#include <drafts/com/sun/star/ui/XUIElement.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XUPDATABLE_HPP_
#include <com/sun/star/util/XUpdatable.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

namespace framework
{

class UIElementWrapperBase : public ::com::sun::star::lang::XTypeProvider       ,
                             public ::drafts::com::sun::star::ui::XUIElement    ,
                             public ::com::sun::star::lang::XInitialization     ,
                             public ::com::sun::star::lang::XComponent          ,
                             public ::com::sun::star::util::XUpdatable          ,
                             protected ThreadHelpBase                           ,
                             public ::cppu::OBroadcastHelper                    ,
                             public ::cppu::OPropertySetHelper                  ,
                             public ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
        UIElementWrapperBase( sal_Int16 nType );
        virtual ~UIElementWrapperBase();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider
        //---------------------------------------------------------------------------------------------------------
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XUpdatable
        virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException);

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException) = 0;

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:

        //  OPropertySetHelper
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        ( com::sun::star::uno::Any&        aConvertedValue ,
                                                                                                               com::sun::star::uno::Any&        aOldValue       ,
                                                                                                               sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException );
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::uno::Exception                 );
        virtual void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper();
        virtual ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException);

        static const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

        ::cppu::OMultiTypeInterfaceContainerHelper                          m_aListenerContainer;   /// container for ALL Listener
        rtl::OUString                                                       m_aResourceURL;
        com::sun::star::uno::WeakReference< com::sun::star::frame::XFrame > m_xWeakFrame;
        sal_Int16                                                           m_nType;
        sal_Bool                                                            m_bInitialized : 1,
                                                                            m_bDisposed;
};

} // namespace framework

#endif // __FRAMEWORK_HELPER_UIELEMENTWRAPPERBASE_HXX_
