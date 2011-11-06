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


#ifndef CHART2_RANGESELECTIONLISTENER_HXX
#define CHART2_RANGESELECTIONLISTENER_HXX

#include "ControllerLockGuard.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XRangeSelectionListener.hpp>

namespace chart
{

class RangeSelectionListenerParent
{
public:
    virtual void listeningFinished( const ::rtl::OUString & rNewRange ) = 0;
    virtual void disposingRangeSelection() = 0;
};

// ----------------------------------------

class RangeSelectionListener : public
    ::cppu::WeakImplHelper1<
        ::com::sun::star::sheet::XRangeSelectionListener >
{
public:
    explicit RangeSelectionListener(
        RangeSelectionListenerParent & rParent,
        const ::rtl::OUString & rInitialRange,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >& xModelToLockController );
    virtual ~RangeSelectionListener();

protected:
    // ____ XRangeSelectionListener ____
    virtual void SAL_CALL done( const ::com::sun::star::sheet::RangeSelectionEvent& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL aborted( const ::com::sun::star::sheet::RangeSelectionEvent& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

private:
    RangeSelectionListenerParent & m_rParent;
    ::rtl::OUString m_aRange;
    ControllerLockGuard m_aControllerLockGuard;
};

} //  namespace chart

// CHART2_RANGESELECTIONLISTENER_HXX
#endif
