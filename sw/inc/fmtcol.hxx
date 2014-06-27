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


#ifndef _FMTCOL_HXX
#define _FMTCOL_HXX

#include <svl/svarray.hxx>
#include "swdllapi.h"
#include <format.hxx>
#include <swtypes.hxx>      // fuer MAXLEVEL

class SwDoc;        // fuer friend

class SwFmtColl : public SwFmt
{
protected:
    SwFmtColl( SwAttrPool& rPool, const sal_Char* pFmtName,
                const sal_uInt16* pWhichRanges, SwFmtColl* pDerFrom,
                sal_uInt16 nFmtWhich )
          : SwFmt( rPool, pFmtName, pWhichRanges, pDerFrom, nFmtWhich )
    { SetAuto( sal_False ); }

    SwFmtColl( SwAttrPool& rPool, const String &rFmtName,
                const sal_uInt16* pWhichRanges, SwFmtColl* pDerFrom,
                sal_uInt16 nFmtWhich )
          : SwFmt( rPool, rFmtName, pWhichRanges, pDerFrom, nFmtWhich )
    { SetAuto( sal_False ); }


private:
    // erstmal wird nicht kopiert und nicht zugewiesen
    SwFmtColl(const SwFmtColl & );
    const SwFmtColl &operator=(const SwFmtColl &);
};


class SW_DLLPUBLIC SwTxtFmtColl: public SwFmtColl
{
    friend class SwDoc;

    SwTxtFmtColl(const SwTxtFmtColl & rRef);

    // --> OD 2007-01-24 #i73790#
    bool mbStayAssignedToListLevelOfOutlineStyle;
    // <--

protected:
    //sal_uInt8 nOutlineLevel;       //<-#outline level, removed by zhaojianwei
    bool mbAssignedToOutlineStyle;//<-#outline level added by zhaojianwei

    SwTxtFmtColl *pNextTxtFmtColl;

    SwTxtFmtColl(
        SwAttrPool& rPool,
        const sal_Char* pFmtCollName,
        SwTxtFmtColl* pDerFrom = 0,
        sal_uInt16 nFmtWh = RES_TXTFMTCOLL )
        : SwFmtColl( rPool, pFmtCollName, aTxtFmtCollSetRange
        , pDerFrom, nFmtWh )
        , mbStayAssignedToListLevelOfOutlineStyle( false )
        , mbAssignedToOutlineStyle( false )
    {
        pNextTxtFmtColl = this;
    }

    SwTxtFmtColl(
        SwAttrPool& rPool,
        const String &rFmtCollName,
        SwTxtFmtColl* pDerFrom = 0,
        sal_uInt16 nFmtWh = RES_TXTFMTCOLL )
        : SwFmtColl( rPool, rFmtCollName, aTxtFmtCollSetRange
        , pDerFrom, nFmtWh )
        , mbStayAssignedToListLevelOfOutlineStyle( false )
        , mbAssignedToOutlineStyle( false )
    {
        pNextTxtFmtColl = this;
    }

    // zum "abfischen" von UL-/LR-/FontHeight Aenderungen
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:

    TYPEINFO();     //Bereits in Basisklasse Client drin.

    inline void SetNextTxtFmtColl(SwTxtFmtColl& rNext);
    SwTxtFmtColl& GetNextTxtFmtColl() const { return *pNextTxtFmtColl; }

    sal_Bool IsAtDocNodeSet() const;

    void SetAttrOutlineLevel( int );
    int GetAttrOutlineLevel() const;

    // Return the list level of the Outline Style - the List Style for the outline numbering -
    // to which the Paragraph Style is assigned.
    int GetAssignedOutlineStyleLevel() const;

    inline bool IsAssignedToListLevelOfOutlineStyle() const
    {
        return mbAssignedToOutlineStyle;
    }

    // If a Paragraph Style is assigned to list level N of the Outline Style,
    // then its outline level - AttrOutlineLevel - is set to N+1
    void AssignToListLevelOfOutlineStyle(const int nAssignedListLevel);

    void DeleteAssignmentToListLevelOfOutlineStyle( const bool bResetOutlineLevel = true );

    // override to recognize changes on the <SwNumRuleItem> and register/unregister
    // the paragragh style at the corresponding <SwNumRule> instance
    virtual sal_Bool SetFmtAttr( const SfxPoolItem& rAttr );
    virtual sal_Bool SetFmtAttr( const SfxItemSet& rSet );
    virtual sal_Bool ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );

    // override <ResetAllFmtAttr()> to stay assigned to list level of outline style
    virtual sal_uInt16 ResetAllFmtAttr();

    inline bool StayAssignedToListLevelOfOutlineStyle() const
    {
        return mbStayAssignedToListLevelOfOutlineStyle;
    }
    // <--

    // --> OD 2008-02-13 #newlistleveattrs#
    bool AreListLevelIndentsApplicable() const;
    // <--

/*----------------- JP 09.08.94 17:36 -------------------
 wird die Funktionalitaet von Zeichenvorlagen an Absatzvorlagen
 ueberhaupt benoetigt ??

 Wenn, ja dann muessen im TextNode und hier in der TxtCollection ein 2.
 Attset fuer die Char-Attribute angelegt werden; damit die Vererbung
 und der Zugriff auf die gesetzen Attribute richtig funktioniert!!

    virtual sal_Bool SetDerivedFrom( SwFmtColl* pDerFrom = 0 );

    inline SwCharFmt* GetCharFmt() const;
    inline sal_Bool IsCharFmtSet() const;
    void SetCharFmt(SwCharFmt *);
    void ResetCharFmt();
inline sal_Bool SwTxtFmtColl::IsCharFmtSet() const
{
    return aCharDepend.GetRegisteredIn() ? sal_True : sal_False;
}
inline SwCharFmt* SwTxtFmtColl::GetCharFmt() const
{
    return (SwCharFmt*)aCharDepend.GetRegisteredIn();
}
--------------------------------------------------*/
};

typedef SwTxtFmtColl* SwTxtFmtCollPtr;
SV_DECL_PTRARR(SwTxtFmtColls,SwTxtFmtCollPtr,2,4)


class SwGrfFmtColl: public SwFmtColl
{
    friend class SwDoc;
protected:
    SwGrfFmtColl( SwAttrPool& rPool, const sal_Char* pFmtCollName,
                    SwGrfFmtColl* pDerFrom = 0 )
        : SwFmtColl( rPool, pFmtCollName, aGrfFmtCollSetRange,
                    pDerFrom, RES_GRFFMTCOLL )
    {}

    SwGrfFmtColl( SwAttrPool& rPool, const String &rFmtCollName,
                    SwGrfFmtColl* pDerFrom = 0 )
        : SwFmtColl( rPool, rFmtCollName, aGrfFmtCollSetRange,
                    pDerFrom, RES_GRFFMTCOLL )
    {}

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.
};

typedef SwGrfFmtColl* SwGrfFmtCollPtr;
SV_DECL_PTRARR(SwGrfFmtColls,SwGrfFmtCollPtr,2,4)



//FEATURE::CONDCOLL
// --------- Bedingte Vorlagen -------------------------------

enum Master_CollConditions
{
    PARA_IN_LIST        = 0x0001,
    PARA_IN_OUTLINE     = 0x0002,
    PARA_IN_FRAME       = 0x0004,
    PARA_IN_TABLEHEAD   = 0x0008,
    PARA_IN_TABLEBODY   = 0x0010,
    PARA_IN_SECTION     = 0x0020,
    PARA_IN_FOOTENOTE   = 0x0040,
    PARA_IN_FOOTER      = 0x0080,
    PARA_IN_HEADER      = 0x0100,
    PARA_IN_ENDNOTE     = 0x0200,
    // ...
    USRFLD_EXPRESSION   = (int)0x8000
};


class SW_DLLPUBLIC SwCollCondition : public SwClient
{
    sal_uLong nCondition;
    union
    {
        sal_uLong nSubCondition;
        String* pFldExpression;
    } aSubCondition;

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.


    SwCollCondition( SwTxtFmtColl* pColl, sal_uLong nMasterCond,
                    sal_uLong nSubCond = 0 );
    SwCollCondition( SwTxtFmtColl* pColl, sal_uLong nMasterCond,
                    const String& rSubExp );
    virtual ~SwCollCondition();

    // @@@ public copy ctor, but no copy assignment?
    SwCollCondition( const SwCollCondition& rCpy );
private:
    // @@@ public copy ctor, but no copy assignment?
    SwCollCondition & operator= (const SwCollCondition &);
public:

    int operator==( const SwCollCondition& rCmp ) const;
    int operator!=( const SwCollCondition& rCmp ) const
                            { return ! (*this == rCmp); }

    sal_uLong GetCondition() const      { return nCondition; }
    sal_uLong GetSubCondition() const   { return aSubCondition.nSubCondition; }
    const String* GetFldExpression() const
                                    { return aSubCondition.pFldExpression; }

    void SetCondition( sal_uLong nCond, sal_uLong nSubCond );
    SwTxtFmtColl* GetTxtFmtColl() const     { return (SwTxtFmtColl*)GetRegisteredIn(); }
    void RegisterToFormat( SwFmt& );
};


typedef SwCollCondition* SwCollConditionPtr;
SV_DECL_PTRARR_DEL( SwFmtCollConditions, SwCollConditionPtr, 0, 5 )

class SW_DLLPUBLIC SwConditionTxtFmtColl : public SwTxtFmtColl
{
    friend class SwDoc;
protected:
    SwFmtCollConditions aCondColls;

    SwConditionTxtFmtColl( SwAttrPool& rPool, const sal_Char* pFmtCollName,
                            SwTxtFmtColl* pDerFrom = 0 )
        : SwTxtFmtColl( rPool, pFmtCollName, pDerFrom, RES_CONDTXTFMTCOLL )
    {}
    SwConditionTxtFmtColl( SwAttrPool& rPool, const String &rFmtCollName,
                            SwTxtFmtColl* pDerFrom = 0 )
        : SwTxtFmtColl( rPool, rFmtCollName, pDerFrom, RES_CONDTXTFMTCOLL )
    {}

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.

    virtual ~SwConditionTxtFmtColl();

    const SwCollCondition* HasCondition( const SwCollCondition& rCond ) const;
    const SwFmtCollConditions& GetCondColls() const     { return aCondColls; }
    void InsertCondition( const SwCollCondition& rCond );
    sal_Bool RemoveCondition( const SwCollCondition& rCond );

    void SetConditions( const SwFmtCollConditions& );
};

//FEATURE::CONDCOLL

// ------------- Inline Implementierungen --------------------

inline void SwTxtFmtColl::SetNextTxtFmtColl( SwTxtFmtColl& rNext )
{
    pNextTxtFmtColl = &rNext;
}
#endif

