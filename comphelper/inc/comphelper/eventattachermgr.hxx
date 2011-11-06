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



#ifndef _COMPHELPER_EVENTATTACHERMGR_HXX_
#define _COMPHELPER_EVENTATTACHERMGR_HXX_

#include <com/sun/star/uno/Reference.hxx>
#include "comphelper/comphelperdllapi.h"

namespace com { namespace sun { namespace star {
namespace uno {
    class Exception;
}
namespace lang {
    class XMultiServiceFactory;
}
namespace script {
    class XEventAttacherManager;
}
namespace beans {
    class XIntrospection;
}
} } }


namespace comphelper
{

COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >
createEventAttacherManager(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospection > & rIntrospection,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
    throw( ::com::sun::star::uno::Exception );


COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >
createEventAttacherManager(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
    throw( ::com::sun::star::uno::Exception );

}

#endif // _COMPHELPER_EVENTATTACHERMGR_HXX_

