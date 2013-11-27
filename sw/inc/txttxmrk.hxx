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


#ifndef _TXTTXMRK_HXX
#define _TXTTXMRK_HXX

#include <txatbase.hxx>

class SwTxtNode;
class SwDoc;

// SWTXT_TOXMARK *******************************
// Attribut fuer die Verzeichnisse

class SwTxtTOXMark : public SwTxtAttrEnd
{
    SwTxtNode * m_pTxtNode;
    xub_StrLen * m_pEnd;   // 0 if SwTOXMark without AlternativeText

public:
    SwTxtTOXMark( SwTOXMark& rAttr,
            xub_StrLen const nStart, xub_StrLen const*const pEnd = 0);
    virtual ~SwTxtTOXMark();

    virtual xub_StrLen *GetEnd();     // SwTxtAttr

    void CopyTOXMark( SwDoc* pDestDoc );

    // get and set TxtNode pointer
    inline const SwTxtNode* GetpTxtNd() const { return m_pTxtNode; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }
};

inline const SwTxtNode& SwTxtTOXMark::GetTxtNode() const
{
    ASSERT( m_pTxtNode, "SwTxtTOXMark: where is my TxtNode?" );
    return *m_pTxtNode;
}

#endif

