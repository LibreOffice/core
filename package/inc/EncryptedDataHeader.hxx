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


#ifndef _ENCRYPTED_DATA_HEADER_HXX_
#define _ENCRYPTED_DATA_HEADER_HXX_

#include <sal/types.h>

/* The structure of this header is as follows:

   Header signature  4 bytes
   Version number    2 bytes
   Iteraction count  4 bytes
   Size              4 bytes
   EncAlgorithm      4 bytes
   DigestAlgorithm   4 bytes
   DerivedKeySize    4 bytes
   StartKeyAlgorithm 4 bytes
   Salt length       2 bytes
   IV length         2 bytes
   Digest length     2 bytes
   MediaType length  2 bytes
   Salt content      X bytes
   IV content        X bytes
   digest content    X bytes
   MediaType         X bytes

*/
const sal_uInt32 n_ConstHeader = 0x05024d4dL; // "MM\002\005"
const sal_Int32 n_ConstHeaderSize = 38; // + salt length + iv length + digest length + mediatype length
const sal_Int16 n_ConstCurrentVersion = 1;
#endif
