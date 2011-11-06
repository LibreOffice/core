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



#ifndef _EVENTNOTIFICATION_HXX_
#define _EVENTNOTIFICATION_HXX_

#ifndef _COM_SUN_STAR_UNO_XINTEFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#include <com/sun/star/uno/Reference.hxx>

//-----------------------------------
// encapsulate a filepicker event
// notification, because there are
// two types of filepicker notifications
// with and without parameter
// this is an application of the
// "command" pattern see GoF
//-----------------------------------

class CEventNotification
{
public:
    virtual ~CEventNotification() { };

    virtual void SAL_CALL notifyEventListener( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xListener ) = 0;
};

#endif
