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



class ResMgr;

#define AVMEDIA_RESID( nId ) ResId( nId, * ::avmedia::GetResMgr() )

#ifdef WNT

#define AVMEDIA_MANAGER_SERVICE_NAME                    "com.sun.star.comp.avmedia.Manager_DirectX"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED            sal_False

#define AVMEDIA_MANAGER_SERVICE_NAME_FALLBACK1          ""
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED_FALLBACK1  sal_False

#else
#ifdef QUARTZ

#define AVMEDIA_MANAGER_SERVICE_NAME                    "com.sun.star.comp.avmedia.Manager_QuickTime"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED            sal_False

#define AVMEDIA_MANAGER_SERVICE_NAME_FALLBACK1           "com.sun.star.comp.avmedia.Manager_MacAVF"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED_FALLBACK1  sal_False

#else

#define AVMEDIA_MANAGER_SERVICE_NAME                    "com.sun.star.comp.avmedia.Manager_GStreamer"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED            sal_False

#define AVMEDIA_MANAGER_SERVICE_NAME_FALLBACK1          "com.sun.star.comp.avmedia.Manager_Java"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED_FALLBACK1  sal_True

#endif
#endif

namespace avmedia
{
    ResMgr* GetResMgr();
}
