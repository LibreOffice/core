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



#ifndef SW_VBAEVENTS_HXX
#define SW_VBAEVENTS_HXX

#include <vbahelper/vbaeventshelperbase.hxx>

// ============================================================================

class SwVbaEventsHelper : public VbaEventsHelperBase
{
public:
    SwVbaEventsHelper(
        const css::uno::Sequence< css::uno::Any >& rArgs,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~SwVbaEventsHelper();

protected:
    virtual bool implPrepareEvent( EventQueue& rEventQueue, const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::uno::Any > implBuildArgumentList( const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException);
    virtual void implPostProcessEvent( EventQueue& rEventQueue, const EventHandlerInfo& rInfo, bool bCancel ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString implGetDocumentModuleName( const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) const throw (css::lang::IllegalArgumentException);
};

// ============================================================================

#endif

