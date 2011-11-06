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



#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#define _COMPHELPER_NAMECONTAINER_HXX_

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include "comphelper/comphelperdllapi.h"

namespace comphelper
{
    COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
        NameContainer_createInstance( ::com::sun::star::uno::Type aType );
}

#endif // _COMPHELPER_NAMECONTAINER_HXX_

