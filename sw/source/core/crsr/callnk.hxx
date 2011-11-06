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


#ifndef _CALLNK_HXX
#define _CALLNK_HXX

#include <tools/solar.h>

class SwCrsrShell;
class SwTxtNode;
class SwRootFrm;

class SwCallLink
{
public:
    SwCrsrShell & rShell;
    sal_uLong nNode;
    xub_StrLen nCntnt;
    sal_uInt8 nNdTyp;
    long nLeftFrmPos;
    bool bHasSelection;

    SwCallLink( SwCrsrShell & rSh );
    SwCallLink( SwCrsrShell & rSh, sal_uLong nAktNode, xub_StrLen nAktCntnt,
                                    sal_uInt8 nAktNdTyp, long nLRPos,
                                    bool bAktSelection );
    ~SwCallLink();

    static long getLayoutFrm( const SwRootFrm*, SwTxtNode& rNd, xub_StrLen nCntPos, sal_Bool bCalcFrm );
};



#endif  // _CALLNK_HXX
