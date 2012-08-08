/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#define INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX

#include "svtools/svtdllapi.h"

#include <vector>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <vcl/keycod.hxx>
#include <vcl/evntpost.hxx>
#include <osl/mutex.h>


namespace css = ::com::sun::star;

namespace svt
{


struct TMutexInit
{
    ::osl::Mutex m_aLock;
};

//===============================================
/**
    @descr  implements a helper, which can be used to
            convert vcl key codes into awt key codes ...
            and reverse.

            Further such key code can be triggered.
            Doing so different accelerator
            configurations are merged together; a suitable
            command registered for the given key code is searched
            and will be dispatched.

    @attention

            Because exceution of an accelerator command can be dangerous
            (in case it force an office shutdown for key "ALT+F4"!)
            all internal dispatches are done asynchronous.
            Menas that the trigger call doesnt wait till the dispatch
            is finished. You can call very often. All requests will be
            queued internal and dispatched ASAP.

            Of course this queue will be stopped if the environment
            will be destructed ...
 */
class SVT_DLLPUBLIC AcceleratorExecute : private TMutexInit
{
    //-------------------------------------------
    // const, types
    private:

        /** @deprecated
                replaced by internal class AsyncAccelExec ...
                remove this resource here if we go forwards to next major */
        typedef ::std::vector< ::std::pair< css::util::URL, css::uno::Reference< css::frame::XDispatch > > > TCommandQueue;

    //-------------------------------------------
    // member
    private:

        /** TODO document me */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** TODO document me */
        css::uno::Reference< css::util::XURLTransformer > m_xURLParser;

        /** TODO document me */
        css::uno::Reference< css::frame::XDispatchProvider > m_xDispatcher;

        /** TODO document me */
        css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xGlobalCfg;
        css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xModuleCfg;
        css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xDocCfg;

        /** @deprecated
                replaced by internal class AsyncAccelExec ...
                remove this resource here if we go forwards to next major */
        TCommandQueue m_lCommandQueue;

        /** @deprecated
                replaced by internal class AsyncAccelExec ...
                remove this resource here if we go forwards to next major */
        ::vcl::EventPoster m_aAsyncCallback;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  factory method to create new accelerator
                    helper instance.

            @descr  Such helper instance must be initialized at first.
                    So it can know its environment (global/module or
                    document specific).

                    Afterwards it can be used to execute incoming
                    accelerator requests.

                    The "end of life" of such helper can be reached as follow:

                    - delete the object
                      => If it stands currently in its execute method, they will
                         be finished. All further queued requests will be removed
                         and further not executed!

                    - "let it stay alone"
                      => All currently queued events will be finished. The
                         helper kills itself afterwards. A shutdown of the
                         environment will be recognized ... The helper stop its
                         work immediatly then!
         */
        static AcceleratorExecute* createAcceleratorHelper();

        //---------------------------------------
        /** @short  fight against inlining ... */
        virtual ~AcceleratorExecute();

        //---------------------------------------
        /** @short  init this instance.

            @descr  It must be called as first method after creation.
                    And further it can be called more then once ...
                    but at least its should be used one times only.
                    Otherwhise nobody can say, which asynchronous
                    executions will be used inside the old and which one
                    will be used inside the new environment.

            @param  xSMGR
                    reference to an uno service manager.

            @param  xEnv
                    if it points to a valid frame it will be used
                    to execute the dispatch there. Further the frame
                    is used to locate the right module configuration
                    and use it merged together with the document and
                    the global configuration.

                    If this parameter is set to NULL, the global configuration
                    is used only. Further the global Desktop instance is
                    used for dispatch.
         */
        virtual void init(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR,
                          const css::uno::Reference< css::frame::XFrame >&              xEnv );

        //---------------------------------------
        /** @short  trigger this accelerator.

            @descr  The internal configuartions are used to find
                    as suitable command for this key code.
                    This command will be queued and executed later
                    asynchronous.

            @param  aKey
                    specify the accelerator for execute.

            @return [sal_Bool]
                    sal_True if this key is configured and match to a command.
                    Attention: This state does not mean the success state
                    of the corresponding execute. Because its done asynchronous!
         */
        virtual sal_Bool execute(const KeyCode&            aKey);
        virtual sal_Bool execute(const css::awt::KeyEvent& aKey);

        /** search the command for the given key event.
        *
        * @param aKey The key event
        * @return The command or an empty string if the key event could not be found.
        */
        ::rtl::OUString  findCommand(const ::com::sun::star::awt::KeyEvent& aKey);
        //---------------------------------------
        /** TODO document me */
        static css::awt::KeyEvent st_VCLKey2AWTKey(const KeyCode&            aKey);
        static KeyCode            st_AWTKey2VCLKey(const css::awt::KeyEvent& aKey);

        //---------------------------------------
        /** TODO document me */
        static css::uno::Reference< css::ui::XAcceleratorConfiguration > st_openGlobalConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------
        /** TODO document me */
        static css::uno::Reference< css::ui::XAcceleratorConfiguration > st_openModuleConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                                                                              const css::uno::Reference< css::frame::XFrame >&              xFrame);

        //---------------------------------------
        /** TODO document me */
        static css::uno::Reference< css::ui::XAcceleratorConfiguration > st_openDocConfig(const css::uno::Reference< css::frame::XModel >& xModel);

    //-------------------------------------------
    // internal
    private:

        //---------------------------------------
        /** @short  allow creation of instances of this class
                    by using our factory only!
         */
        SVT_DLLPRIVATE AcceleratorExecute();

        AcceleratorExecute(const AcceleratorExecute& rCopy);
        void operator=(const AcceleratorExecute&) {};
        //---------------------------------------
        /** TODO document me */
        SVT_DLLPRIVATE ::rtl::OUString impl_ts_findCommand(const css::awt::KeyEvent& aKey);

        //---------------------------------------
        /** TODO document me */
        SVT_DLLPRIVATE css::uno::Reference< css::util::XURLTransformer > impl_ts_getURLParser();

        //---------------------------------------
        /** @deprecated
                replaced by internal class AsyncAccelExec ...
                remove this resource here if we go forwards to next major */
        DECL_DLLPRIVATE_LINK(impl_ts_asyncCallback, void*);
};

} // namespace svt

#endif // INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
