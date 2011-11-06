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



#ifndef SC_RTFPARSE_HXX
#define SC_RTFPARSE_HXX

#include "eeparser.hxx"

#ifdef SC_RTFPARSE_CXX

struct ScRTFCellDefault
{
    SfxItemSet          aItemSet;
    SCCOL               nCol;
    sal_uInt16              nTwips;         // rechter Rand der Zelle
    SCCOL               nColOverlap;    // MergeCell wenn >1, merged cells wenn 0

                        ScRTFCellDefault( SfxItemPool* pPool ) :
                            aItemSet( *pPool ), nColOverlap(1) {}
};

DECLARE_LIST( ScRTFDefaultList, ScRTFCellDefault* )
// Remove: (const unsigned short &) not sufficiently different from (unsigned short)
// deswegen sal_uLong, typedef bringt's auch nicht :-(
SV_DECL_VARARR_SORT( ScRTFColTwips, sal_uLong, 16, 4)

#else       // SC_RTFPARSE_CXX

struct ScRTFCellDefault;
class ScRTFDefaultList;
class ScRTFColTwips;

#endif      // SC_RTFPARSE_CXX


class EditEngine;

class ScRTFParser : public ScEEParser
{
private:
    ScRTFDefaultList*   pDefaultList;
    ScRTFColTwips*      pColTwips;
    ScRTFCellDefault*   pInsDefault;
    ScRTFCellDefault*   pActDefault;
    ScRTFCellDefault*   pDefMerge;
    sal_uLong               nStartAdjust;
    sal_uInt16              nLastWidth;
    sal_Bool                bNewDef;

    DECL_LINK( RTFImportHdl, ImportInfo* );
    inline void         NextRow();
    void                EntryEnd( ScEEParseEntry*, const ESelection& );
    void                ProcToken( ImportInfo* );
    void                ColAdjust();
    sal_Bool                SeekTwips( sal_uInt16 nTwips, SCCOL* pCol );
    void                NewCellRow( ImportInfo* );

public:
                        ScRTFParser( EditEngine* );
    virtual             ~ScRTFParser();
    virtual sal_uLong       Read( SvStream&, const String& rBaseURL );
};


#endif
