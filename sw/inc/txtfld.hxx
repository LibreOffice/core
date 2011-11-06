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


#ifndef _TXTFLD_HXX
#define _TXTFLD_HXX

#include <txatbase.hxx>
#include <tools/string.hxx>
#include <pam.hxx>

class SwTxtNode;

// ATT_FLD ***********************************

class SwTxtFld : public SwTxtAttr
{
    mutable   String m_aExpand;
    SwTxtNode * m_pTxtNode;

public:
    SwTxtFld(SwFmtFld & rAttr, xub_StrLen const nStart);
    virtual ~SwTxtFld();

    void CopyFld( SwTxtFld *pDest ) const;
    void Expand() const;
    inline void ExpandAlways();

    // get and set TxtNode pointer
    SwTxtNode* GetpTxtNode() const { return m_pTxtNode; }
    inline SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }
    // enable notification that field content has changed and needs reformatting
    void NotifyContentChange(SwFmtFld& rFmtFld);

    // #111840#
    /**
       Returns position of this field.

       @return position of this field. Has to be deleted explicitly.
    */
//    SwPosition * GetPosition() const;
};

inline SwTxtNode& SwTxtFld::GetTxtNode() const
{
    ASSERT( m_pTxtNode, "SwTxtFld:: where is my TxtNode?" );
    return *m_pTxtNode;
}

inline void SwTxtFld::ExpandAlways()
{
    m_aExpand += ' ';
    Expand();
}

#endif

