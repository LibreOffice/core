/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: taskcreatorsrv.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:10:36 $
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

#ifndef __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_
#define __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif

//_______________________________________________
// definition

namespace framework
{

//_______________________________________________
/**
 *  TODO document me
 */
class TaskCreatorService : public  css::lang::XTypeProvider
                         , public  css::lang::XServiceInfo
                         , public  css::lang::XSingleServiceFactory
                           // attention! Must be the first base class to guarentee right initialize lock ...
                         , private ThreadHelpBase
                         , public  ::cppu::OWeakObject
{
    //___________________________________________
    // types

    public:

        /// [XFrame] if it's set, it will be used as parent frame for the new created frame.
        static const ::rtl::OUString ARGUMENT_PARENTFRAME;

        /** [OUString] if it's not a special name (beginning with "_" ... which are not allowed here!)
                       it will be set as the API name of the new created frame.
         */
        static const ::rtl::OUString ARGUMENT_FRAMENAME;

        /// [sal_Bool] If its set to TRUE we will make the new created frame visible.
        static const ::rtl::OUString ARGUMENT_MAKEVISIBLE;

        /** [sal_Bool] If not "ContainerWindow" property is set it force creation of a
                       top level window as new container window.
         */
        static const ::rtl::OUString ARGUMENT_CREATETOPWINDOW;

        /// [Rectangle] Place the new created frame on this place and resize the container window.
        static const ::rtl::OUString ARGUMENT_POSSIZE;

        /// [XWindow] an outside created window, used as container window of the new created frame.
        static const ::rtl::OUString ARGUMENT_CONTAINERWINDOW;

        /** [sal_Bool] enable/disable special mode, where this frame will be part of
                       the persistent window state feature suitable for any office module window
         */
        static const ::rtl::OUString ARGUMENT_SUPPORTPERSISTENTWINDOWSTATE;

        /** [sal_Bool] enable/disable special mode, where the title bar of our
                       the new created frame will be updated automaticly.
                       Default = ON !
         */
        static const ::rtl::OUString ARGUMENT_ENABLE_TITLEBARUPDATE;
    //___________________________________________
    // member

    private:

        //---------------------------------------
        /** @short  the global uno service manager.
            @descr  Must be used to create own needed services.
         */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

    //___________________________________________
    // interface

    public:

                 TaskCreatorService(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~TaskCreatorService(                                                                   );

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XSingleServiceFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance()
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);
    //___________________________________________
    // helper

    private:

        css::uno::Reference< css::awt::XWindow > implts_createContainerWindow( const css::uno::Reference< css::awt::XWindow >& xParentWindow ,
                                                                               const css::awt::Rectangle&                      aPosSize      ,
                                                                                     sal_Bool                                  bTopWindow    );

        void implts_applyDocStyleToWindow(const css::uno::Reference< css::awt::XWindow >& xWindow) const;

        css::uno::Reference< css::frame::XFrame > implts_createFrame( const css::uno::Reference< css::frame::XFrame >& xParentFrame     ,
                                                                      const css::uno::Reference< css::awt::XWindow >&  xContainerWindow ,
                                                                      const ::rtl::OUString&                           sName            );

        void implts_establishWindowStateListener( const css::uno::Reference< css::frame::XFrame >& xFrame );
        void implts_establishTitleBarUpdate( const css::uno::Reference< css::frame::XFrame >& xFrame );

        void implts_establishDocModifyListener( const css::uno::Reference< css::frame::XFrame >& xFrame );

        ::rtl::OUString impl_filterNames( const ::rtl::OUString& sName );
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_
