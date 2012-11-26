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



#ifndef SC_UIITEMS_HXX
#define SC_UIITEMS_HXX

#include "scdllapi.h"
#include "conditio.hxx"
#include "sortparam.hxx"
#include "queryparam.hxx"
#include "paramisc.hxx"
#include <svl/poolitem.hxx>

class ScEditEngineDefaulter;
class EditTextObject;
class ScViewData;
class ScDPSaveData;

// ---------------------------------------------------------------------------

//  Items

class ScInputStatusItem : public SfxPoolItem
{
    ScAddress           aCursorPos;
    ScAddress           aStartPos;
    ScAddress           aEndPos;
    String              aString;
    EditTextObject*     pEditData;

public:
//UNUSED2008-05                                SCTAB nTab,
//UNUSED2008-05                                SCCOL nCol, SCROW nRow,
//UNUSED2008-05                                SCCOL nStartCol, SCROW nStartRow,
//UNUSED2008-05                                SCCOL nEndCol,   SCROW nSEndRow,
//UNUSED2008-05                                const String& rString,
//UNUSED2008-05                                const EditTextObject* pData );

                            ScInputStatusItem( sal_uInt16 nWhich,
                                               const ScAddress& rCurPos,
                                               const ScAddress& rStartPos,
                                               const ScAddress& rEndPos,
                                               const String& rString,
                                               const EditTextObject* pData );
                            ScInputStatusItem( const ScInputStatusItem& rItem );
                            ~ScInputStatusItem();

    virtual String          GetValueText() const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScAddress&        GetPos() const      { return aCursorPos; }
    const ScAddress&        GetStartPos() const { return aStartPos; }
    const ScAddress&        GetEndPos() const   { return aEndPos; }
    SCTAB                   GetTab() const      { return aCursorPos.Tab(); }
    SCCOL                   GetCol() const      { return aCursorPos.Col(); }
    SCROW                   GetRow() const      { return aCursorPos.Row(); }
    SCCOL                   GetStartCol() const { return aStartPos.Col(); }
    SCROW                   GetStartRow() const { return aStartPos.Row(); }
    SCCOL                   GetEndCol() const   { return aEndPos.Col(); }
    SCROW                   GetEndRow() const   { return aEndPos.Row(); }

    const String&           GetString() const   { return aString; }
    const EditTextObject*   GetEditData() const { return pEditData; }
};


#define SC_TAB_INSERTED     1
#define SC_TAB_DELETED      2
#define SC_TAB_MOVED        3
#define SC_TAB_COPIED       4
#define SC_TAB_HIDDEN       5

class ScTablesHint : public SfxHint
{
    sal_uInt16 nId;
    SCTAB nTab1;
    SCTAB nTab2;

public:
                    ScTablesHint(sal_uInt16 nNewId, SCTAB nTable1, SCTAB nTable2=0);
                    ~ScTablesHint();

    sal_uInt16          GetId() const           { return nId; }
    SCTAB           GetTab1() const         { return nTab1; }
    SCTAB           GetTab2() const         { return nTab2; }
};

class ScEditViewHint : public SfxHint
{
    ScEditEngineDefaulter*  pEditEngine;
    ScAddress                   aCursorPos;

public:
                    ScEditViewHint( ScEditEngineDefaulter* pEngine, const ScAddress& rCurPos );
                    ~ScEditViewHint();

    SCCOL           GetCol() const      { return aCursorPos.Col(); }
    SCROW           GetRow() const      { return aCursorPos.Row(); }
    SCTAB           GetTab() const      { return aCursorPos.Tab(); }
    ScEditEngineDefaulter*  GetEngine() const   { return pEditEngine; }

private:
    ScEditViewHint(); // disabled
};

class ScIndexHint : public SfxHint
{
    sal_uInt16 nId;
    sal_uInt16 nIndex;

public:
                    ScIndexHint(sal_uInt16 nNewId, sal_uInt16 nIdx);
                    ~ScIndexHint();

    sal_uInt16          GetId() const           { return nId; }
    sal_uInt16          GetIndex() const        { return nIndex; }
};

//----------------------------------------------------------------------------
// Parameter-Item fuer den Sortierdialog:

class SC_DLLPUBLIC ScSortItem : public SfxPoolItem
{
public:
                            ScSortItem( sal_uInt16              nWhich,
                                        ScViewData*         ptrViewData,
                                        const ScSortParam*  pSortData );
                            ScSortItem( sal_uInt16              nWhich,
                                        const ScSortParam*  pSortData );
                            ScSortItem( const ScSortItem& rItem );
                            ~ScSortItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberUd ) const;

    ScViewData*         GetViewData () const { return pViewData; }
    const ScSortParam&  GetSortData () const { return theSortData; }

private:
    ScViewData*     pViewData;
    ScSortParam     theSortData;
};

//----------------------------------------------------------------------------
// Parameter-Item fuer den Filterdialog:

class SC_DLLPUBLIC ScQueryItem : public SfxPoolItem
{
public:
                            ScQueryItem( sal_uInt16                 nWhich,
                                         ScViewData*            ptrViewData,
                                         const ScQueryParam*    pQueryData );
                            ScQueryItem( sal_uInt16                 nWhich,
                                         const ScQueryParam*    pQueryData );
                            ScQueryItem( const ScQueryItem& rItem );
                            ~ScQueryItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    ScViewData*         GetViewData () const { return pViewData; }
    const ScQueryParam& GetQueryData() const { return theQueryData; }

    sal_Bool        GetAdvancedQuerySource(ScRange& rSource) const;
    void        SetAdvancedQuerySource(const ScRange* pSource);

private:
    ScViewData*     pViewData;
    ScQueryParam    theQueryData;
    sal_Bool            bIsAdvanced;
    ScRange         aAdvSource;
};

//----------------------------------------------------------------------------
// Parameter-Item fuer den Zwischenergebnisdialog:

class SC_DLLPUBLIC ScSubTotalItem : public SfxPoolItem
{
public:
                ScSubTotalItem( sal_uInt16                  nWhich,
                                ScViewData*             ptrViewData,
                                const ScSubTotalParam*  pSubTotalData );
                ScSubTotalItem( sal_uInt16                  nWhich,
                                const ScSubTotalParam*  pSubTotalData );
                ScSubTotalItem( const ScSubTotalItem&   rItem );
                ~ScSubTotalItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberUd ) const;

    ScViewData*             GetViewData () const { return pViewData; }
    const ScSubTotalParam&  GetSubTotalData() const { return theSubTotalData; }

private:
    ScViewData*     pViewData;
    ScSubTotalParam theSubTotalData;
};

//----------------------------------------------------------------------------
// Parameter-Item fuer die Benutzerlisten-TabPage:

class SC_DLLPUBLIC ScUserListItem : public SfxPoolItem
{
public:
                ScUserListItem( sal_uInt16 nWhich );
                ScUserListItem( const ScUserListItem& rItem );
                ~ScUserListItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    void        SetUserList ( const ScUserList& rUserList );
    ScUserList* GetUserList () const { return pUserList; }

private:
    ScUserList* pUserList;
};

//----------------------------------------------------------------------------
// Parameter-Item fuer die Konsolidieren-Dialog

class ScConsolidateItem : public SfxPoolItem
{
public:
                ScConsolidateItem( sal_uInt16                    nWhich,
                                   const ScConsolidateParam* pParam );
                ScConsolidateItem( const ScConsolidateItem& rItem );
                ~ScConsolidateItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScConsolidateParam& GetData() const { return theConsData; }

private:
    ScConsolidateParam  theConsData;
};

//----------------------------------------------------------------------------
// Parameter-Item fuer den Pivot-Dialog

class ScPivotItem : public SfxPoolItem
{
public:
                ScPivotItem( sal_uInt16 nWhich, const ScDPSaveData* pData,
                             const ScRange* pRange, sal_Bool bNew );
                ScPivotItem( const ScPivotItem& rItem );
                ~ScPivotItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScDPSaveData& GetData() const         { return *pSaveData; }
    const ScRange&      GetDestRange() const    { return aDestRange; }
    sal_Bool                IsNewSheet() const      { return bNewSheet; }

private:
    ScDPSaveData*   pSaveData;
    ScRange         aDestRange;
    sal_Bool            bNewSheet;
};

//----------------------------------------------------------------------------
// Parameter-Item fuer den Solver-Dialog

class ScSolveItem : public SfxPoolItem
{
public:
                ScSolveItem( sal_uInt16              nWhich,
                             const ScSolveParam* pParam );
                ScSolveItem( const ScSolveItem& rItem );
                ~ScSolveItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScSolveParam& GetData() const { return theSolveData; }

private:
    ScSolveParam    theSolveData;
};

//----------------------------------------------------------------------------
// Parameter-Item fuer den Mehrfachoperationen-Dialog

class ScTabOpItem : public SfxPoolItem
{
public:
                ScTabOpItem( sal_uInt16              nWhich,
                             const ScTabOpParam* pParam );
                ScTabOpItem( const ScTabOpItem& rItem );
                ~ScTabOpItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScTabOpParam& GetData() const { return theTabOpData; }

private:
    ScTabOpParam    theTabOpData;
};

//----------------------------------------------------------------------------
// Parameter-Item fuer den Dialog bedingte Formatierung

class ScCondFrmtItem : public SfxPoolItem
{
public:
                ScCondFrmtItem( sal_uInt16 nWhich,
//!                             const ScConditionalFormat* pCondFrmt );
                                const ScConditionalFormat& rCondFrmt );
                ScCondFrmtItem( const ScCondFrmtItem& rItem );
                ~ScCondFrmtItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScConditionalFormat&  GetData() const { return theCondFrmtData; }

private:
    ScConditionalFormat theCondFrmtData;
};



#endif

