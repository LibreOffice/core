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




#ifndef SC_DECL_H
#define SC_DECL_H

#include <tools/solar.h>
#include <tools/string.hxx>

enum WKTYP { eWK_UNKNOWN = -2, eWK_1 = 0, eWK_2, eWK3, eWK4, eWK_Error, eWK123 };
typedef void ( BEARBFKT )( void );
typedef sal_Char STRING16[ 16 ];
typedef sal_Char STRING14[ 14 ];
typedef sal_Char STRING8[ 8 ];
typedef sal_Char STRING6[ 6 ];
typedef sal_uInt16 USHORT4[ 4 ];
//typedef unsigned short USHORT4[ 4 ];
#endif

