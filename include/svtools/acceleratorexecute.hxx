/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#define INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX

#include <svtools/svtdllapi.h>

#include <memory>

#include <com/sun/star/awt/KeyEvent.hpp>
#include <vcl/keycod.hxx>

namespace com :: sun :: star :: frame { class XDispatchProvider; }
namespace com :: sun :: star :: frame { class XFrame; }
namespace com :: sun :: star :: frame { class XModel; }
namespace com :: sun :: star :: ui { class XAcceleratorConfiguration; }
namespace com :: sun :: star :: uno { class XComponentContext; }
namespace com :: sun :: star :: util { class XURLTransformer; }


namespace svt
{


struct TMutexInit
{
    ::osl::Mutex m_aLock;
};


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

            Because execution of an accelerator command can be dangerous
            (in case it force an office shutdown for key "ALT+F4"!)
            all internal dispatches are done asynchronous.
            Means that the trigger call doesn't wait till the dispatch
            is finished. You can call very often. All requests will be
            queued internal and dispatched ASAP.

            Of course this queue will be stopped if the environment
            will be destructed...
 */
class SVT_DLLPUBLIC AcceleratorExecute : private TMutexInit
{
    // member
    private:

        /** TODO document me */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** TODO document me */
        css::uno::Reference< css::util::XURLTransformer > m_xURLParser;

        /** TODO document me */
        css::uno::Reference< css::frame::XDispatchProvider > m_xDispatcher;

        /** TODO document me */
        css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xGlobalCfg;
        css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xModuleCfg;
        css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xDocCfg;
    // interface
    public:


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
                         work immediately then!
         */
        static std::unique_ptr<AcceleratorExecute> createAcceleratorHelper();


        /** @short  fight against inlining ... */
        virtual ~AcceleratorExecute();


        /** @short  init this instance.

            @descr  It must be called as first method after creation.
                    And further it can be called more than once ...
                    but at least its should be used one times only.
                    Otherwise nobody can say, which asynchronous
                    executions will be used inside the old and which one
                    will be used inside the new environment.

            @param  rxContext
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
        void init(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                          const css::uno::Reference< css::frame::XFrame >&              xEnv );


        /** @short  trigger this accelerator.

            @descr  The internal configurations are used to find
                    as suitable command for this key code.
                    This command will be queued and executed later
                    asynchronous.

            @param  aKey
                    specify the accelerator for execute.

            @return [bool]
                    true if this key is configured and match to a command.
                    Attention: This state does not mean the success state
                    of the corresponding execute. Because its done asynchronous!
         */
        bool execute(const vcl::KeyCode&       aKey);
        bool execute(const css::awt::KeyEvent& aKey);

        /** search the command for the given key event.
        *
        * @param aKey The key event
        * @return The command or an empty string if the key event could not be found.
        */
        OUString  findCommand(const css::awt::KeyEvent& aKey);

        /** TODO document me */
        static css::awt::KeyEvent st_VCLKey2AWTKey(const vcl::KeyCode&       aKey);
        static vcl::KeyCode       st_AWTKey2VCLKey(const css::awt::KeyEvent& aKey);


        /** TODO document me */
        static css::uno::Reference< css::ui::XAcceleratorConfiguration > st_openModuleConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext ,
                                                                                              const css::uno::Reference< css::frame::XFrame >&              xFrame);


        /** TODO document me */
        static css::uno::Reference< css::ui::XAcceleratorConfiguration > st_openDocConfig(const css::uno::Reference< css::frame::XModel >& xModel);


    // internal
    private:


        /** @short  allow creation of instances of this class
                    by using our factory only!
         */
        SVT_DLLPRIVATE AcceleratorExecute();

        AcceleratorExecute(const AcceleratorExecute& rCopy) = delete;
        void operator=(const AcceleratorExecute&)  = delete;

        /** TODO document me */
        SVT_DLLPRIVATE OUString impl_ts_findCommand(const css::awt::KeyEvent& aKey);


        /** TODO document me */
        SVT_DLLPRIVATE css::uno::Reference< css::util::XURLTransformer > impl_ts_getURLParser();
};

} // namespace svt

#endif // INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
