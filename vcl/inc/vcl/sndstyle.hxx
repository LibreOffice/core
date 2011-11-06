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



#ifndef _SV_SNDSTYLE_HXX
#define _SV_SNDSTYLE_HXX

#include <vcl/sv.h>

// ---------------
// - Sound-Types -
// ---------------

typedef sal_uInt16 SoundType;
#define SOUND_DEFAULT                   ((SoundType)0)
#define SOUND_INFO                      ((SoundType)1)
#define SOUND_WARNING                   ((SoundType)2)
#define SOUND_ERROR                     ((SoundType)3)
#define SOUND_QUERY                     ((SoundType)4)

#define SOUND_DISABLE                   ((SoundType)5)

#endif // _SV_SNDSTYLE_HXX
