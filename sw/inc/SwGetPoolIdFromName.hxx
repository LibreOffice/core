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


#ifndef _GETPOOLIDFROMNAMEENUM_HXX
#define _GETPOOLIDFROMNAMEENUM_HXX

/* When using the NameMapper to translate pool ids to UI or programmatic
 * names, this enum is used to define which family is required */

typedef sal_uInt16 SwGetPoolIdFromName;

namespace nsSwGetPoolIdFromName
{
    const SwGetPoolIdFromName GET_POOLID_TXTCOLL  = 0x01;
    const SwGetPoolIdFromName GET_POOLID_CHRFMT   = 0x02;
    const SwGetPoolIdFromName GET_POOLID_FRMFMT   = 0x04;
    const SwGetPoolIdFromName GET_POOLID_PAGEDESC = 0x08;
    const SwGetPoolIdFromName GET_POOLID_NUMRULE  = 0x10;
}

#endif
