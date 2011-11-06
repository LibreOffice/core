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



#ifndef INCLUDED_SD_BULMAPER_HXX
#define INCLUDED_SD_BULMAPER_HXX

#include "tools/solar.h"

class SfxItemSet;
class SvxBulletItem;
class SvxNumberFormat;
class SvxNumRule;

class SdBulletMapper
{
public:
/* #i35937#
    static void PreMapNumBulletForDialog( SfxItemSet& rSet );
    static void PostMapNumBulletForDialog( SfxItemSet& rSet );
*/
    static void MapFontsInNumRule( SvxNumRule& aNumRule, const SfxItemSet& rSet );

};

#endif /* INCLUDED_SD_BULMAPER_HXX */
