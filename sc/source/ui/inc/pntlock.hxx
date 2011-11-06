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



#ifndef SC_PNTLOCK_HXX
#define SC_PNTLOCK_HXX

#include "rangelst.hxx"

class ScPaintLockData
{
private:
    ScRangeListRef  xRangeList;
    sal_uInt16          nMode;
    sal_uInt16          nLevel;
    sal_uInt16          nDocLevel;
    sal_uInt16          nParts;
    sal_Bool            bModified;

public:
                    ScPaintLockData(sal_uInt16 nNewMode);
                    ~ScPaintLockData();

    void            AddRange( const ScRange& rRange, sal_uInt16 nP );

    void            SetModified()   { bModified = sal_True; }
    void            IncLevel(sal_Bool bDoc)
                        { if (bDoc) ++nDocLevel; else ++nLevel; }
    void            DecLevel(sal_Bool bDoc)
                        { if (bDoc) --nDocLevel; else --nLevel; }

    const ScRangeListRef&   GetRangeList() const            { return xRangeList; }
    sal_uInt16                  GetParts() const                { return nParts; }
    sal_uInt16                  GetLevel(sal_Bool bDoc) const
                                { return bDoc ? nDocLevel : nLevel; }
    sal_Bool                    GetModified() const             { return bModified; }

                    // fuer Wiederherstellen nach Reset
    void            SetLevel(sal_uInt16 nNew, sal_Bool bDoc)
                        { if (bDoc) nDocLevel = nNew; else nLevel = nNew; }
};

#endif

