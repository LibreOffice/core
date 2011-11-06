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



#ifndef __ATK_REGISTRY_HXX__
#define __ATK_REGISTRY_HXX__

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <atk/atk.h>

AtkObject * ooo_wrapper_registry_get(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible);

void ooo_wrapper_registry_add(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible, AtkObject *obj);

void ooo_wrapper_registry_remove(::com::sun::star::accessibility::XAccessible *pAccessible);

#endif // __ATK_REGISTRY_HXX_
