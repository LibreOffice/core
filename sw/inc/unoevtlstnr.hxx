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


#ifndef _UNOEVTLSTNR_HXX
#define _UNOEVTLSTNR_HXX

#include <svl/svarray.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com{namespace sun{namespace star{
    namespace lang
        {
            class XEventListener;
        }
}}}
/* -----------------22.04.99 11:18-------------------
 *  Verwaltung der EventListener
 * --------------------------------------------------*/
typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > *  XEventListenerPtr;
SV_DECL_PTRARR(SwEvtLstnrArray, XEventListenerPtr, 4, 4)

class SwEventListenerContainer
{
    protected:
        SwEvtLstnrArray*                            pListenerArr;
        ::com::sun::star::uno::XInterface*          pxParent;
    public:
        SwEventListenerContainer( ::com::sun::star::uno::XInterface* pxParent);
        ~SwEventListenerContainer();

        void    AddListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & rxListener);
        sal_Bool    RemoveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & rxListener);
        void    Disposing();
};
#endif
