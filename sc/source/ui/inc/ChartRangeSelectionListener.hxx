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


#ifndef SC_CHARTRANGESELECTIONLISTENER_HXX
#define SC_CHARTRANGESELECTIONLISTENER_HXX

#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

class ScTabViewShell;

typedef ::cppu::WeakComponentImplHelper2<
        ::com::sun::star::view::XSelectionChangeListener,
        ::com::sun::star::lang::XServiceInfo >
    ScChartRangeSelectionListener_Base;

class ScChartRangeSelectionListener :
    public comphelper::OBaseMutex,
    public ScChartRangeSelectionListener_Base
{
public:
    explicit ScChartRangeSelectionListener( ScTabViewShell * pViewShell );
    virtual ~ScChartRangeSelectionListener();

protected:
    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XSelectionChangeListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ WeakComponentImplHelperBase ____
    // is called when dispose() is called at this component
    virtual void SAL_CALL disposing();

    // ____ XServiceInfo ____
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(
        const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

private:
    ScTabViewShell * m_pViewShell;
};

// SC_CHARTRANGESELECTIONLISTENER_HXX
#endif
