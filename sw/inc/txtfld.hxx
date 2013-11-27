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
    mutable String m_aExpand; // only used to determine, if field content is changing in <ExpandTxtFld()>
    SwTxtNode * m_pTxtNode;

public:
    SwTxtFld(
        SwFmtFld & rAttr,
        xub_StrLen const nStart );

    virtual ~SwTxtFld();

    void CopyTxtFld( SwTxtFld *pDest ) const;

    void ExpandTxtFld() const;
    inline void ExpandAlways()
    {
        m_aExpand += ' '; // changing current value to assure that <ExpandTxtFld()> changes the value.
        ExpandTxtFld();
    }

    // get and set TxtNode pointer
    inline SwTxtNode* GetpTxtNode() const
    {
        return m_pTxtNode;
    }
    inline SwTxtNode& GetTxtNode() const
    {
        ASSERT( m_pTxtNode, "SwTxtFld:: where is my TxtNode?" );
        return *m_pTxtNode;
    }
    inline void ChgTxtNode( SwTxtNode* pNew )
    {
        m_pTxtNode = pNew;
    }

    bool IsFldInDoc() const;

    // enable notification that field content has changed and needs reformatting
    virtual void NotifyContentChange( SwFmtFld& rFmtFld );

};

class SwTxtInputFld : public SwTxtFld
{
public:
    SwTxtInputFld(
        SwFmtFld & rAttr,
        xub_StrLen const nStart,
        xub_StrLen const nEnd );

    virtual ~SwTxtInputFld();

    virtual xub_StrLen* GetEnd();

    virtual void NotifyContentChange( SwFmtFld& rFmtFld );

    void UpdateTextNodeContent( const String& rNewContent );

    const String GetFieldContent() const;
    void UpdateFieldContent();

private:
    xub_StrLen m_nEnd;
};

#endif

