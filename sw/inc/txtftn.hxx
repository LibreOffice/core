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


#ifndef _TXTFTN_HXX
#define _TXTFTN_HXX

#include <txatbase.hxx>

class SwNodeIndex;
class SwTxtNode;
class SwNodes;
class SwDoc;
class SwFrm;

// ATT_FTN **********************************************************

class SW_DLLPUBLIC SwTxtFtn : public SwTxtAttr
{
    SwNodeIndex * m_pStartNode;
    SwTxtNode * m_pTxtNode;
    sal_uInt16 m_nSeqNo;

public:
    SwTxtFtn( SwFmtFtn& rAttr, xub_StrLen nStart );
    virtual ~SwTxtFtn();

    inline SwNodeIndex *GetStartNode() const { return m_pStartNode; }
    void SetStartNode( const SwNodeIndex *pNode, sal_Bool bDelNodes = sal_True );
    void SetNumber( const sal_uInt16 nNumber, const String* = 0 );
    void CopyFtn(SwTxtFtn & rDest, SwTxtNode & rDestNode) const;

    // get and set TxtNode pointer
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

        // lege eine neue leere TextSection fuer diese Fussnote an
    void MakeNewTextSection( SwNodes& rNodes );

        // loesche die FtnFrame aus der Seite
    void DelFrms( const SwFrm* );
        // bedingten Absatzvorlagen checken
    void CheckCondColl();

        // fuer die Querverweise auf Fussnoten
    sal_uInt16 SetSeqRefNo();
    void SetSeqNo( sal_uInt16 n )       { m_nSeqNo = n; }   // for Readers
    sal_uInt16 GetSeqRefNo() const      { return m_nSeqNo; }

    static void SetUniqueSeqRefNo( SwDoc& rDoc );
};

inline const SwTxtNode& SwTxtFtn::GetTxtNode() const
{
    ASSERT( m_pTxtNode, "SwTxtFtn: where is my TxtNode?" );
    return *m_pTxtNode;
}

#endif

