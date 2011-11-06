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



#ifndef CHART2_MODIFYLISTENERCALLBACK_HXX
#define CHART2_MODIFYLISTENERCALLBACK_HXX

// header for class Link
#include <tools/link.hxx>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include "charttoolsdllapi.hxx"

namespace chart {

/** Use this class as a member if you want to listen on a XModifyBroadcaster
without becoming a XModifyListener yourself
 */

class ModifyListenerCallBack_impl;

class OOO_DLLPUBLIC_CHARTTOOLS ModifyListenerCallBack
{
public:
    explicit ModifyListenerCallBack( const Link& rCallBack );

    virtual ~ModifyListenerCallBack();

    void startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >& xBroadcaster );
    SAL_DLLPRIVATE void stopListening();

private: //methods
    SAL_DLLPRIVATE ModifyListenerCallBack();
    SAL_DLLPRIVATE ModifyListenerCallBack( const ModifyListenerCallBack& );

private: //member
    ModifyListenerCallBack_impl* pModifyListener_impl;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener >   m_xModifyListener;
};

} // namespace chart

#endif

