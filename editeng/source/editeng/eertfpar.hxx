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


#ifndef _EERTFPAR_HXX
#define _EERTFPAR_HXX

#include <editeng/svxrtf.hxx>

#include <editdoc.hxx>
#include <impedit.hxx>

#ifndef SVX_LIGHT

class EditNodeIdx : public SvxNodeIdx
{
private:
    ContentNode*        pNode;
    ImpEditEngine*      pImpEditEngine;

public:
                        EditNodeIdx( ImpEditEngine* pIEE, ContentNode* pNd = 0)
                            { pImpEditEngine = pIEE; pNode = pNd; }
    virtual sal_uLong       GetIdx() const;
    virtual SvxNodeIdx* Clone() const;
    ContentNode*        GetNode() { return pNode; }
};

class EditPosition : public SvxPosition
{
private:
    EditSelection*  pCurSel;
    ImpEditEngine*  pImpEditEngine;

public:
                    EditPosition( ImpEditEngine* pIEE, EditSelection* pSel )
                            { pImpEditEngine = pIEE; pCurSel = pSel; }

    virtual sal_uLong   GetNodeIdx() const;
    virtual sal_uInt16  GetCntIdx() const;

    // erzeuge von sich selbst eine Kopie
    virtual SvxPosition* Clone() const;

    // erzeuge vom NodeIndex eine Kopie
    virtual SvxNodeIdx* MakeNodeIdx() const;
};

#define ACTION_INSERTTEXT       1
#define ACTION_INSERTPARABRK    2

class EditRTFParser : public SvxRTFParser
{
private:
    EditSelection       aCurSel;
    ImpEditEngine*      pImpEditEngine;
    CharSet             eDestCharSet;
    MapMode             aRTFMapMode;
    MapMode             aEditMapMode;

    sal_uInt16              nDefFont;
    sal_uInt16              nDefTab;
    sal_uInt16              nDefFontHeight;
    sal_uInt8               nLastAction;

protected:
    virtual void        InsertPara();
    virtual void        InsertText();
    virtual void        MovePos( int bForward = sal_True );
    virtual void        SetEndPrevPara( SvxNodeIdx*& rpNodePos,
                                            sal_uInt16& rCntPos );

    virtual void        UnknownAttrToken( int nToken, SfxItemSet* pSet );
    virtual void        NextToken( int nToken );
    virtual void        SetAttrInDoc( SvxRTFItemStackType &rSet );
    virtual int         IsEndPara( SvxNodeIdx* pNd, sal_uInt16 nCnt ) const;
    virtual void        CalcValue();
    void                CreateStyleSheets();
    SfxStyleSheet*      CreateStyleSheet( SvxRTFStyleType* pRTFStyle );
    SvxRTFStyleType*    FindStyleSheet( const String& rName );
    void                AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd );
    void                ReadField();
    void                SkipGroup();

public:
                EditRTFParser( SvStream& rIn, EditSelection aCurSel, SfxItemPool& rAttrPool, ImpEditEngine* pImpEditEngine );
                ~EditRTFParser();

    virtual SvParserState   CallParser();


    void        SetDestCharSet( CharSet eCharSet )  { eDestCharSet = eCharSet; }
    CharSet     GetDestCharSet() const              { return eDestCharSet; }

    sal_uInt16      GetDefTab() const                   { return nDefTab; }
    Font        GetDefFont()                        { return GetFont( nDefFont ); }

    EditPaM     GetCurPaM() const                   { return aCurSel.Max(); }
};

SV_DECL_REF( EditRTFParser )
SV_IMPL_REF( EditRTFParser );


#endif  // !SVX_LIGH
#endif  //_EERTFPAR_HXX
