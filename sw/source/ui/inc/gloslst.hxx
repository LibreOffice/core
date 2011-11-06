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



#ifndef _GLOSLST_HXX
#define _GLOSLST_HXX


#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <vcl/timer.hxx>
#include <svl/svarray.hxx>

class SwGlossaries;
class SvStringsISortDtor;

struct AutoTextGroup
{
    sal_uInt16      nCount;
    String      sName;
    String      sTitle;
    String      sLongNames;   // durch 0x0A getrennte Langnamen
    String      sShortNames;  // durch 0x0A getrennte Kurznamen
    DateTime    aDateModified;
};


typedef AutoTextGroup* AutoTextGroupPtr;
SV_DECL_PTRARR(AutoTextGroups, AutoTextGroupPtr, 4, 4)

class SwGlossaryList : public AutoTimer
{
    AutoTextGroups  aGroupArr;
    String          sPath;
    sal_Bool            bFilled;

    AutoTextGroup*  FindGroup(const String& rGroupName);
    void            FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGloss);

public:
        SwGlossaryList();
        ~SwGlossaryList();

    sal_Bool            HasLongName(const String& rBegin, SvStringsISortDtor* pLongNames );
    sal_Bool            GetShortName(const String& rLongName,
                                        String& rShortName, String& rGroupName );

    sal_uInt16          GetGroupCount();
    String          GetGroupName(sal_uInt16 nPos, sal_Bool bNoPath = sal_True, String* pTitle = 0);
    sal_uInt16          GetBlockCount(sal_uInt16 nGroup);
    String          GetBlockName(sal_uInt16 nGroup, sal_uInt16 nBlock, String& rShortName);

    void            Update();

    virtual void    Timeout();

    void            ClearGroups();
};

#endif






