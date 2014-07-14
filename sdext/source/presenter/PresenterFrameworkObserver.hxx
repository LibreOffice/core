/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SDEXT_PRESENTER_PRESENTER_FRAMEWORK_OBSERVER_HXX
#define SDEXT_PRESENTER_PRESENTER_FRAMEWORK_OBSERVER_HXX

#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {


typedef ::cppu::WeakComponentImplHelper1 <
    ::com::sun::star::drawing::framework::XConfigurationChangeListener
    > PresenterFrameworkObserverInterfaceBase;

/** Watch the drawing framework for changes and run callbacks when a certain
    change takes place.
*/
class PresenterFrameworkObserver
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterFrameworkObserverInterfaceBase
{
public:
    typedef ::boost::function<bool(void)> Predicate;
    typedef ::boost::function<void(bool)> Action;

    /** Register an action callback to be run when the specified resource is
        activated.  The action may be called synchronously when the resource
        is already active or asynchronously when the resource is activated in
        the future.
        @param rxController
            The controller gives access to the drawing framework.
        @param rxResourceId
            The id of the resource to watch for activation.
        @param rAction
            The callback that is called when the resource is activated.
    */
    static void RunOnResourceActivation (
        const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        const Action& rAction);
    static void RunOnUpdateEnd (
        const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
        const Action& rAction);

    virtual void SAL_CALL disposing (void);
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);

private:
    ::rtl::OUString msEventType;
    ::css::uno::Reference<css::drawing::framework::XConfigurationController> mxConfigurationController;
    Predicate maPredicate;
    Action maAction;

    /** Create a new PresenterFrameworkObserver object.
        @param rsEventName
            An event name other than ConfigurationUpdateEnd.  When the
            observer shall only listen for ConfigurationUpdateEnd then pass
            an empty name.
        @param rPredicate
            This functor tests whether the action is to be executed or not.
        @param rAction
            The functor to execute when the predicate returns true,
            e.g. when some resource has been created.
    */
    PresenterFrameworkObserver (
        const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
        const ::rtl::OUString& rsEventName,
        const Predicate& rPredicate,
        const Action& rAction);
    virtual ~PresenterFrameworkObserver (void);

    void Shutdown (void);

    /** Predicate that returns true when the specified resource is active
        with respect to the given configuration controller.
    */
    static bool HasResource (
        const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId);

    /** Predicate that always returns true.
    */
    static bool True (void);

    /** Predicate that always returns false.
    */
    static bool False (void);
};


} }  // end of namespace ::sdext::presenter

#endif
