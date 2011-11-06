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



#ifndef SFX2_SIGNATURESTATE_HXX
#define SFX2_SIGNATURESTATE_HXX

// xmlsec05, check with SFX team
#define SIGNATURESTATE_UNKNOWN                  (sal_Int16)(-1)
#define SIGNATURESTATE_NOSIGNATURES             (sal_Int16)0
#define SIGNATURESTATE_SIGNATURES_OK            (sal_Int16)1
#define SIGNATURESTATE_SIGNATURES_BROKEN        (sal_Int16)2
// State was SIGNATURES_OK, but doc is modified now
#define SIGNATURESTATE_SIGNATURES_INVALID       (sal_Int16)3
// signature is OK, but certificate could not be validated
#define SIGNATURESTATE_SIGNATURES_NOTVALIDATED  (sal_Int16)4
//signatur and certificate are ok,but not al files are signed, as was the case in
//OOo 2.x - OOo 3.1.1. This state is only used  together with document signatures.
#define SIGNATURESTATE_SIGNATURES_PARTIAL_OK    (sal_Int16)5

#endif // SFX2_SIGNATURESTATE_HXX
