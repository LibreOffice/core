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


#ifndef _VCOMPAT_HXX
#define _VCOMPAT_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

// -----------
// - Defines -
// -----------

#define COMPAT_FORMAT( char1, char2, char3, char4 ) \
    ((sal_uInt32)((((sal_uInt32)(char)(char1)))|                \
    (((sal_uInt32)(char)(char2))<<8UL)|                 \
    (((sal_uInt32)(char)(char3))<<16UL)|                    \
    ((sal_uInt32)(char)(char4))<<24UL))

// --------------
// - ImplCompat -
// --------------

class SvStream;

class TOOLS_DLLPUBLIC VersionCompat
{
    SvStream*       mpRWStm;
    sal_uInt32          mnCompatPos;
    sal_uInt32          mnTotalSize;
    sal_uInt16          mnStmMode;
    sal_uInt16          mnVersion;

                    VersionCompat() {}
                    VersionCompat( const VersionCompat& ) {}
    VersionCompat&  operator=( const VersionCompat& ) { return *this; }
    sal_Bool            operator==( const VersionCompat& ) { return sal_False; }

public:

                    VersionCompat( SvStream& rStm, sal_uInt16 nStreamMode, sal_uInt16 nVersion = 1 );
                    ~VersionCompat();

    sal_uInt16          GetVersion() const { return mnVersion; }
};

#endif // _VCOMPAT_HXX
