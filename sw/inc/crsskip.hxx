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



#ifndef _CRSSKIP_HXX
#define _CRSSKIP_HXX

#include <tools/solar.h>


// define for cursor travelling normally in western text cells and chars do
// the same, but in complex text cell skip over legatures and char skip
// into it.
// These defines exist only to cut off the dependicies to I18N project.
const sal_uInt16 CRSR_SKIP_CHARS  = 0;
const sal_uInt16 CRSR_SKIP_CELLS  = 1;
const sal_uInt16 CRSR_SKIP_HIDDEN = 2;

#endif
