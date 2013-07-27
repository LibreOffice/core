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

#ifndef SVX_SIDEBAR_PANEL_FACTORY_HXX
#define SVX_SIDEBAR_PANEL_FACTORY_HXX

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/ui/XUIElementFactory.hpp>

#include <boost/noncopyable.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace svx { namespace sidebar {

namespace
{
    typedef ::cppu::WeakComponentImplHelper1 <
        css::ui::XUIElementFactory
        > PanelFactoryInterfaceBase;
}


class PanelFactory
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PanelFactoryInterfaceBase
{
public:
    static ::rtl::OUString SAL_CALL getImplementationName (void);
    static cssu::Reference<cssu::XInterface> SAL_CALL createInstance (
        const cssu::Reference<css::lang::XMultiServiceFactory>& rxFactory);
    static cssu::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames (void);

    PanelFactory (void);
    virtual ~PanelFactory (void);

    // XUIElementFactory
    cssu::Reference<css::ui::XUIElement> SAL_CALL createUIElement (
        const ::rtl::OUString& rsResourceURL,
        const ::cssu::Sequence<css::beans::PropertyValue>& rArguments)
        throw(
            css::container::NoSuchElementException,
            css::lang::IllegalArgumentException,
            cssu::RuntimeException);
};


} } // end of namespace svx::sidebar

#endif
