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


#ifndef _UNOHELPER_HXX
#define _UNOHELPER_HXX

#include <sal/types.h>

// forward declarations
namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno {
        class XInterface;
        template<class T> class Reference;
    }
    namespace beans { class XPropertySet; }
} } }


#define OUSTRING(msg) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( msg ) )

namespace xforms
{

/** instantiate a UNO service using the process global service factory */
com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
    createInstance( const rtl::OUString& sServiceName );

/** copy the properties from one PropertySet into the next */
void copy( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& , com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& );

} // namespace

#endif
