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


#ifndef _TXTRFMRK_HXX
#define _TXTRFMRK_HXX

#include <txatbase.hxx>

class SwTxtNode;

// SWTXT_REFMARK *************************************
// Attribut fuer Inhalts-/Positions-Referenzen im Text

class SwTxtRefMark : public SwTxtAttrEnd
{
    SwTxtNode * m_pTxtNode;
    xub_StrLen * m_pEnd; // end is optional (point reference)

public:
    SwTxtRefMark( SwFmtRefMark& rAttr,
            xub_StrLen const nStart, xub_StrLen const*const pEnd = 0);

    virtual xub_StrLen * GetEnd();       // SwTxtAttr

    // get and set TxtNode pointer
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }
};

inline const SwTxtNode& SwTxtRefMark::GetTxtNode() const
{
    ASSERT( m_pTxtNode, "SwTxtRefMark: where is my TxtNode?" );
    return *m_pTxtNode;
}

#endif

