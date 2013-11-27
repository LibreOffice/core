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


#ifndef _DOCFLD_HXX
#define _DOCFLD_HXX

#include <calc.hxx>         // fuer SwHash

class SwTxtFld;
class SwIndex;
class SwNodeIndex;
class SwCntntNode;
class SwCntntFrm;
class SwSectionNode;
class SwSection;
class SwTxtTOXMark;
class SwTableBox;
class SwTxtINetFmt;
class SwFlyFrmFmt;
class SwDoc;
class SwNode;
struct SwPosition;

// Update an den Expression Feldern
class _SetGetExpFld
{
    sal_uLong nNode;
    xub_StrLen nCntnt;
    union {
        const SwTxtFld* pTxtFld;
        const SwSection* pSection;
        const SwPosition* pPos;
        const SwTxtTOXMark* pTxtTOX;
        const SwTableBox* pTBox;
        const SwTxtINetFmt* pTxtINet;
        const SwFlyFrmFmt* pFlyFmt;
    } CNTNT;
    enum _SetGetExpFldType
        {
            TEXTFIELD, TEXTTOXMARK, SECTIONNODE, CRSRPOS, TABLEBOX,
            TEXTINET, FLYFRAME
        } eSetGetExpFldType;

public:
    _SetGetExpFld( const SwNodeIndex& rNdIdx, const SwTxtFld* pFld = 0,
                    const SwIndex* pIdx = 0 );

    _SetGetExpFld( const SwNodeIndex& rNdIdx, const SwTxtINetFmt& rINet,
                    const SwIndex* pIdx = 0 );

    _SetGetExpFld( const SwSectionNode& rSectNode,
                    const SwPosition* pPos = 0  );

    _SetGetExpFld( const SwTableBox& rTableBox,
                    const SwPosition* pPos = 0  );

    _SetGetExpFld( const SwNodeIndex& rNdIdx, const SwTxtTOXMark& rTOX,
                    const SwIndex* pIdx );

    _SetGetExpFld( const SwPosition& rPos );

    _SetGetExpFld( const SwFlyFrmFmt& rFlyFmt, const SwPosition* pPos = 0 );

    sal_Bool operator==( const _SetGetExpFld& rFld ) const;
    sal_Bool operator<( const _SetGetExpFld& rFld ) const;

    const SwTxtFld* GetTxtFld() const
        { return TEXTFIELD == eSetGetExpFldType ? CNTNT.pTxtFld : 0; }
    const SwTxtTOXMark* GetTOX() const
        { return TEXTTOXMARK == eSetGetExpFldType ? CNTNT.pTxtTOX : 0; }
    const SwSection* GetSection() const
        { return SECTIONNODE == eSetGetExpFldType ? CNTNT.pSection : 0; }
    const SwTableBox* GetTableBox() const
        { return TABLEBOX == eSetGetExpFldType ? CNTNT.pTBox : 0; }
    const SwTxtINetFmt* GetINetFmt() const
        { return TEXTINET == eSetGetExpFldType ? CNTNT.pTxtINet : 0; }
    const SwFlyFrmFmt* GetFlyFmt() const
        { return FLYFRAME == eSetGetExpFldType ? CNTNT.pFlyFmt : 0; }

    sal_uLong GetNode() const { return nNode; }
    xub_StrLen GetCntnt() const { return nCntnt; }
    const void* GetPointer() const { return CNTNT.pTxtFld; }

    void GetPos( SwPosition& rPos ) const;
    void GetPosOfContent( SwPosition& rPos ) const;

    const SwNode* GetNodeFromCntnt() const;
    xub_StrLen GetCntPosFromCntnt() const;

    void SetBodyPos( const SwCntntFrm& rFrm );
};

typedef _SetGetExpFld* _SetGetExpFldPtr;
SV_DECL_PTRARR_SORT_DEL( _SetGetExpFlds, _SetGetExpFldPtr, 0, 10 )


// Struktur zum Speichern der Strings aus SetExp-String-Feldern
struct _HashStr : public SwHash
{
    String aSetStr;
    _HashStr( const String& rName, const String& rText, _HashStr* = 0 );
};

struct SwCalcFldType : public SwHash
{
    const SwFieldType* pFldType;

    SwCalcFldType( const String& rStr, const SwFieldType* pFldTyp )
        : SwHash( rStr ), pFldType( pFldTyp )
    {}
};

// Suche nach dem String, der unter dem Namen in der HashTabelle abgelegt
// wurde
void LookString( SwHash** ppTbl, sal_uInt16 nSize, const String& rName,
                    String& rRet, sal_uInt16* pPos = 0 );


// --------

const int GETFLD_ALL        = 3;        // veroderte Flags !!
const int GETFLD_CALC       = 1;
const int GETFLD_EXPAND     = 2;

class SwDocUpdtFld
{
    _SetGetExpFlds* pFldSortLst;    // akt. Field-Liste zum Calculieren
    SwCalcFldType*  aFldTypeTable[ TBLSZ ];

// noch eine weitere Optimierung - wird z.Z. nicht angesprochen!
    long nFldUpdtPos;               // ab dieser Position mit Update starten
    SwCntntNode* pCNode;            // der TxtNode zur UpdatePos.

    sal_uLong nNodes;                   // sollte die NodesAnzahl unterschiedlich sein
    sal_uInt8 nFldLstGetMode;

    sal_Bool bInUpdateFlds : 1;         // zur Zeit laeuft ein UpdateFlds,
    sal_Bool bFldsDirty : 1;            // irgendwelche Felder sind ungueltig

    void _MakeFldList( SwDoc& pDoc, int eGetMode );
    void GetBodyNode( const SwTxtFld& , sal_uInt16 nFldWhich );
    void GetBodyNode( const SwSectionNode&);
public:
    SwDocUpdtFld();
    ~SwDocUpdtFld();

    const _SetGetExpFlds* GetSortLst() const { return pFldSortLst; }

    void MakeFldList( SwDoc& rDoc, int bAll, int eGetMode );

    void InsDelFldInFldLst( sal_Bool bIns, const SwTxtFld& rFld );

    void InsertFldType( const SwFieldType& rType );
    void RemoveFldType( const SwFieldType& rType );

    sal_Bool IsInUpdateFlds() const         { return bInUpdateFlds; }
    void SetInUpdateFlds( sal_Bool b )      { bInUpdateFlds = b; }

    sal_Bool IsFieldsDirty() const          { return bFldsDirty; }
    void SetFieldsDirty( sal_Bool b )       { bFldsDirty = b; }

    SwHash**    GetFldTypeTable() const { return (SwHash**)aFldTypeTable; }
};


#endif  // _DOCFLD_HXX

