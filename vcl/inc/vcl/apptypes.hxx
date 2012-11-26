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



#ifndef _VCL_APPTYPES_HXX
#define _VCL_APPTYPES_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>

// ---------------------
// - Application-Types -
// ---------------------

#define EXC_RSCNOTLOADED            ((sal_uInt16)0x0100)
#define EXC_SYSOBJNOTCREATED        ((sal_uInt16)0x0200)
#define EXC_SYSTEM                  ((sal_uInt16)0x0300)
#define EXC_DISPLAY                 ((sal_uInt16)0x0400)
#define EXC_REMOTE                  ((sal_uInt16)0x0500)
#define EXC_USER                    ((sal_uInt16)0x1000)
#define EXC_MAJORTYPE               ((sal_uInt16)0xFF00)
#define EXC_MINORTYPE               ((sal_uInt16)0x00FF)

#define UNIQUEID_SV_BEGIN           64000

class VCL_DLLPUBLIC ApplicationProperty
{
public:
    virtual ~ApplicationProperty();
};

#define INPUT_MOUSE                 0x0001
#define INPUT_KEYBOARD              0x0002
#define INPUT_PAINT                 0x0004
#define INPUT_TIMER                 0x0008
#define INPUT_OTHER                 0x0010
#define INPUT_APPEVENT              0x0020
#define INPUT_MOUSEANDKEYBOARD      (INPUT_MOUSE | INPUT_KEYBOARD)
#define INPUT_ANY                   (INPUT_MOUSEANDKEYBOARD | INPUT_PAINT | INPUT_TIMER | INPUT_OTHER | INPUT_APPEVENT)

#define DISPATCH_OPEN               0x0001
#define DISPATCH_PRINT              0x0002
#define DISPATCH_SERVER             0x0004

// --------------
// - UserActive -
// --------------

#define USERACTIVE_MOUSEDRAG            ((sal_uInt16)0x0001)
#define USERACTIVE_INPUT                ((sal_uInt16)0x0002)
#define USERACTIVE_MODALDIALOG          ((sal_uInt16)0x0004)
#define USERACTIVE_ALL                  ((sal_uInt16)0xFFFF)

#endif // _VCL_APPTYPES_HXX
