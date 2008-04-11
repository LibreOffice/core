/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: loaddispatcher.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_

//_______________________________________________
// my own includes

#include <loadenv/loadenv.hxx>

//_______________________________________________
// interface includes
#include <com/sun/star/frame/XNotifyingDispatch.hpp>

//_______________________________________________
// other includes
#include <cppuhelper/implbase1.hxx>

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
