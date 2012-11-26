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


#ifndef _SC_BRDCST_HXX
#define _SC_BRDCST_HXX

#include "global.hxx"
#include "address.hxx"
#include <svl/hint.hxx>
#include <svl/smplhint.hxx>
class ScBaseCell;

#define SC_HINT_DYING       SFX_HINT_DYING
#define SC_HINT_DATACHANGED SFX_HINT_DATACHANGED
#define SC_HINT_TABLEOPDIRTY    SFX_HINT_USER00
#define SC_HINT_CALCALL         SFX_HINT_USER01

class ScHint : public SfxSimpleHint
{
private:
    ScAddress   aAddress;
    ScBaseCell* pCell;

public:
                        ScHint( sal_uLong n, const ScAddress& a, ScBaseCell* p )
                            : SfxSimpleHint( n ), aAddress( a ), pCell( p ) {}
    ScBaseCell*         GetCell() const { return pCell; }
    void                SetCell( ScBaseCell* p )    { pCell = p; }
    const ScAddress&    GetAddress() const { return aAddress; }
          ScAddress&    GetAddress()       { return aAddress; }
    void                SetAddress( const ScAddress& rAdr ) { aAddress = rAdr; }
};

class ScAreaChangedHint : public SfxHint
{
private:
    ScRange aNewRange;
public:
                    ScAreaChangedHint(const ScRange& rRange) : aNewRange(rRange) {}
    const ScRange&  GetRange() const { return aNewRange; }
};


#endif
