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


#ifndef _CPPUHELPER_IMPLBASE_EX_PRE_HXX_
#define _CPPUHELPER_IMPLBASE_EX_PRE_HXX_

/** @internal */
#define __IFC_EX_TYPE_INIT_NAME( class_cast, ifc_name ) \
{ { ifc_name::static_type }, ((sal_IntPtr)(ifc_name *) class_cast 16) - 16 }
/** @internal */
#define __IFC_EX_TYPE_INIT( class_cast, N ) __IFC_EX_TYPE_INIT_NAME( class_cast, Ifc##N )

#endif
