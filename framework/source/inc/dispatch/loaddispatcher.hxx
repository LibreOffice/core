/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loaddispatcher.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:31:41 $
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

#ifndef __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_

//_______________________________________________
// my own includes

#ifndef __FRAMEWORK_LOADENV_LOADENV_HXX_
#include <loadenv/loadenv.hxx>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//_______________________________________________
// namespace

namespace framework{

namespace css = ::com::sun::star;

//_______________________________________________
// exported const

//_______________________________________________
// exported definitions

/** @short      implements a dispatch object which can be used to load
                non-visible components (by using the mechanism of ContentHandler)
                or visible-components (by using the mechanism of FrameLoader).

    @author     as96863
 */
class LoadDispatcher : private ThreadHelpBase
                     , public  ::cppu::WeakImplHelper1< css::frame::XNotifyingDispatch > // => XDispatch => XInterface
{
    //___________________________________________
    // member

    private:

        /** @short  can be used to create own needed services on demand. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  TODO document me */
        css::uno::WeakReference< css::frame::XFrame > m_xOwnerFrame;

        /** @short  TODO document me */
        ::rtl::OUString m_sTarget;

        /** @short  TODO document me */
        sal_Int32 m_nSearchFlags;

        /** @short  TODO document me */
        LoadEnv m_aLoader;

    //___________________________________________
    // native interface

    public:

        /** @short  creates a new instance and initialize it with all neccessary parameters.

            @descr  Every instance of such LoadDispatcher can be used for the specified context only.
                    That means: It can be used to load any further requested content into tzhe here(!)
                    specified target frame.

            @param  xSMGR
                    will be used to create own needed services on demand.

            @param  xOwnerFrame
                    used as startpoit to locate the right target frame.

            @param  sTargetName
                    the name or the target frame for loading or a special qualifier
                    which define such target.

            @param  nSearchFlags
                    used in case sTargetFrame isnt a special one.
         */
        LoadDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR       ,
                       const css::uno::Reference< css::frame::XFrame >&              xOwnerFrame ,
                       const ::rtl::OUString                                         sTargetName ,
                             sal_Int32                                               nSearchFlags);

        //_______________________________________

        /** @short  used to free internal resources.
         */
        virtual ~LoadDispatcher();

    //___________________________________________
    // uno interface

    public:

        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                       const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
            throw(css::uno::RuntimeException);

        // XDispatch
        virtual void SAL_CALL dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                   const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);

}; // class LoadDispatcher

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_
