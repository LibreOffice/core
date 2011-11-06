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



#ifndef _SFXQUERYSTATUS_HXX
#define _SFXQUERYSTATUS_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <svl/poolitem.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/conditn.hxx>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

#include <sfx2/sfxuno.hxx>

class SfxQueryStatus_Impl;
class SFX2_DLLPUBLIC SfxQueryStatus
{
    public:
        SfxQueryStatus( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const rtl::OUString& aCommand );
        ~SfxQueryStatus();

        // Query method
        SfxItemState QueryState( SfxPoolItem*& pPoolItem );

    private:
        SfxQueryStatus( const SfxQueryStatus& );
        SfxQueryStatus();
        SfxQueryStatus& operator=( const SfxQueryStatus& );

        com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >   m_xStatusListener;
        SfxQueryStatus_Impl*                                                       m_pSfxQueryStatusImpl;
};

#endif // _SFXQUERYSTATUS_HXX
