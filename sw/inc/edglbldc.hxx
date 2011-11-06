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


#ifndef _EDGLBLDC_HXX
#define _EDGLBLDC_HXX

#include <svl/svarray.hxx>

class SwSection;
class SwTOXBase;
class SwTOXBaseSection;

enum GlobalDocContentType {
    GLBLDOC_UNKNOWN,
    GLBLDOC_TOXBASE,
    GLBLDOC_SECTION
};

class SwGlblDocContent
{
    GlobalDocContentType eType;
    sal_uLong nDocPos;
    union {
        const SwTOXBase* pTOX;
        const SwSection* pSect;
    } PTR;

public:
    SwGlblDocContent( sal_uLong nPos );
    SwGlblDocContent( const SwTOXBaseSection* pTOX );
    SwGlblDocContent( const SwSection* pSect );

    // Inhalte abfragen
    GlobalDocContentType GetType() const { return eType; }
    const SwSection* GetSection() const
                            { return GLBLDOC_SECTION == eType ? PTR.pSect : 0; }
    const SwTOXBase* GetTOX() const
                            { return GLBLDOC_TOXBASE == eType ? PTR.pTOX : 0; }
    sal_uLong GetDocPos() const { return nDocPos; }

    // fuers Sortieren
    inline int operator==( const SwGlblDocContent& rCmp ) const
        {   return GetDocPos() == rCmp.GetDocPos(); }
    inline int operator<( const SwGlblDocContent& rCmp ) const
        {   return GetDocPos() < rCmp.GetDocPos(); }
};


typedef SwGlblDocContent* SwGlblDocContentPtr;
SV_DECL_PTRARR_SORT_DEL( SwGlblDocContents, SwGlblDocContentPtr, 10, 10 )



#endif
