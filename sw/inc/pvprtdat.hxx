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



#ifndef _PVPRTDAT_HXX
#define _PVPRTDAT_HXX


#include <tools/solar.h>

class SwPagePreViewPrtData
{
    sal_uLong nLeftSpace, nRightSpace, nTopSpace, nBottomSpace,
            nHorzSpace, nVertSpace;
    sal_uInt8 nRow, nCol;
    sal_Bool bLandscape : 1;
    sal_Bool bStretch : 1;
public:
    SwPagePreViewPrtData()
        : nLeftSpace(0), nRightSpace(0), nTopSpace(0), nBottomSpace(0),
            nHorzSpace(0), nVertSpace(0), nRow(1), nCol(1),
            bLandscape(0),bStretch(0)
    {}

    sal_uLong GetLeftSpace() const          { return nLeftSpace; }
    void SetLeftSpace( sal_uLong n )        { nLeftSpace = n; }

    sal_uLong GetRightSpace() const         { return nRightSpace; }
    void SetRightSpace( sal_uLong n )       { nRightSpace = n; }

    sal_uLong GetTopSpace() const           { return nTopSpace; }
    void SetTopSpace( sal_uLong n )         { nTopSpace = n; }

    sal_uLong GetBottomSpace() const        { return nBottomSpace; }
    void SetBottomSpace( sal_uLong n )      { nBottomSpace = n; }

    sal_uLong GetHorzSpace() const          { return nHorzSpace; }
    void SetHorzSpace( sal_uLong n )        { nHorzSpace = n; }

    sal_uLong GetVertSpace() const          { return nVertSpace; }
    void SetVertSpace( sal_uLong n )        { nVertSpace = n; }

    sal_uInt8 GetRow() const                { return nRow; }
    void SetRow(sal_uInt8 n )               { nRow = n; }

    sal_uInt8 GetCol() const                { return nCol; }
    void SetCol( sal_uInt8 n )              { nCol = n; }

    sal_Bool GetLandscape() const           { return bLandscape; }
    void SetLandscape( sal_Bool b )         { bLandscape = b; }
};


#endif


