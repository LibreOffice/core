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




#ifndef APPLICAT_HXX
#define APPLICAT_HXX

class SvxErrorHandler;

#include <sfx2/app.hxx>

/**************************************************************************/
/*
**
**  MACRO DEFINITION
**
**/

#define SMDLL   1

#define APPLICATIONNAME      "smath3"

/**************************************************************************/
/*
**
**  CLASS DEFINITION
**
**/

#ifdef PM2
#define RELEASE     "PM304"
#endif

#ifdef WNT
#define RELEASE     "WNT304"
#endif

#ifdef UNX
#define RELEASE     "UNX304"
#endif

#ifndef SMDLL
class SmResId : public ResId
{
public:
    SmResId(sal_uInt16 nId) :
        ResId(nId)
    {
    }

};

#endif

#endif

