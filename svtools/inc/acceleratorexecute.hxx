/*************************************************************************
 *
 *  $RCSfile: acceleratorexecute.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:27:36 $
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

#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#define INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX

//===============================================
// includes

#include <vector>

#ifndef __COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef __COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef __COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef __DRAFTS_COM_SUN_STAR_UI_XACCELERATORCONFIGURATION_HPP_
#include <drafts/com/sun/star/ui/XAcceleratorConfiguration.hpp>
#endif

#ifndef __COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef __COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif

#ifndef _SV_KEYCODE_HXX
#include <vcl/keycod.hxx>
#endif

#ifndef _VCL_EVNTPOST_HXX
#include <vcl/evntpost.hxx>
#endif

#ifndef _OSL_MUTEX_H_
#include <osl/mutex.h>
#endif

//===============================================
// namespace

namespace svt
{

//===============================================
// definitions

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
class AcceleratorExecute : private TMutexInit
{
    //-------------------------------------------
    // const, types
    private:

        /** TODO document me */
        typedef ::std::vector< ::std::pair< css::util::URL, css::uno::Reference< css::frame::XDispatch > > > TCommandQueue;

    //-------------------------------------------
    // member
    private:

        /** TODO document me */
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xSMGR;

        /** TODO document me */
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLParser;

        /** TODO document me */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xDispatcher;

        /** TODO document me */
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XAcceleratorConfiguration > m_xGlobalCfg;
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XAcceleratorConfiguration > m_xModuleCfg;
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XAcceleratorConfiguration > m_xDocCfg;

        /** TODO document me */
        TCommandQueue m_lCommandQueue;

        /** TODO document me */
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
        virtual void init(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMGR,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&              xEnv );

        //---------------------------------------
        /** @short  trigger this accelerator.

            @descr  The internal configuartions are used to find
                    as suitable command for this key code.
                    This command will be queued and executed later
                    asynchronous.

            @param  aKey
                    specify the accelerator for execute.

            @return [sal_Bool]
                    TRUE if this key is configured and match to a command.
                    Attention: This state does not mean the success state
                    of the corresponding execute. Because its done asynchronous!
         */
        virtual sal_Bool execute(const KeyCode&            aKey);
        virtual sal_Bool execute(const ::com::sun::star::awt::KeyEvent& aKey);

        //---------------------------------------
        /** TODO document me */
        static ::com::sun::star::awt::KeyEvent st_VCLKey2AWTKey(const KeyCode&            aKey);
        static KeyCode            st_AWTKey2VCLKey(const ::com::sun::star::awt::KeyEvent& aKey);

        //---------------------------------------
        /** TODO document me */
        static ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XAcceleratorConfiguration > st_openGlobalConfig(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------
        /** TODO document me */
        static ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XAcceleratorConfiguration > st_openModuleConfig(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMGR ,
                                                                                              const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&              xFrame);

        //---------------------------------------
        /** TODO document me */
        static ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XAcceleratorConfiguration > st_openDocConfig(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel);

    //-------------------------------------------
    // internal
    private:

        //---------------------------------------
        /** @short  allow creation of instances of this class
                    by using our factory only!
         */
        AcceleratorExecute();
        AcceleratorExecute(const AcceleratorExecute& rCopy);
        void operator=(const AcceleratorExecute& rCopy) {};

        //---------------------------------------
        /** TODO document me */
        ::rtl::OUString impl_ts_findCommand(const ::com::sun::star::awt::KeyEvent& aKey);

        //---------------------------------------
        /** TODO document me */
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > impl_ts_getURLParser();

        //---------------------------------------
        /** TODO document me */
        DECL_LINK(impl_ts_asyncCallback, void*);
};

} // namespace svt

#endif // INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
