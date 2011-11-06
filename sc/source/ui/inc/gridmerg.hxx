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



#ifndef SC_GRIDMERG_HXX
#define SC_GRIDMERG_HXX

#include <tools/solar.h>

class OutputDevice;

class ScGridMerger
{
private:
    OutputDevice*   pDev;
    long            nOneX;
    long            nOneY;
    long            nFixStart;
    long            nFixEnd;
    long            nVarStart;
    long            nVarDiff;
    long            nCount;
    sal_Bool            bVertical;
    sal_Bool            bOptimize;

    void        AddLine( long nStart, long nEnd, long nPos );

public:
                ScGridMerger( OutputDevice* pOutDev, long nOnePixelX, long nOnePixelY );
                ~ScGridMerger();

    void        AddHorLine( long nX1, long nX2, long nY );
    void        AddVerLine( long nX, long nY1, long nY2 );
    void        Flush();
};



#endif


