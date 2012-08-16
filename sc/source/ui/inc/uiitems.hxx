/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_UIITEMS_HXX
#define SC_UIITEMS_HXX

#include "scdllapi.h"
#include "conditio.hxx"
#include "sortparam.hxx"
#include "subtotalparam.hxx"
#include "paramisc.hxx"
#include <svl/poolitem.hxx>

#include <boost/scoped_ptr.hpp>

class ScEditEngineDefaulter;
class EditTextObject;
class ScViewData;
class ScDPSaveData;
struct ScQueryParam;

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
                            TYPEINFO();

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
#define SC_TABS_INSERTED    6
#define SC_TABS_DELETED     7

class ScTablesHint : public SfxHint
{
    sal_uInt16 nId;
    SCTAB nTab1;
    SCTAB nTab2;

public:
                    TYPEINFO();
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
                    TYPEINFO();
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
                    TYPEINFO();
                    ScIndexHint(sal_uInt16 nNewId, sal_uInt16 nIdx);
                    ~ScIndexHint();

    sal_uInt16          GetId() const           { return nId; }
    sal_uInt16          GetIndex() const        { return nIndex; }
};

//----------------------------------------------------------------------------
// Parameter item for the sort dialog:

class SC_DLLPUBLIC ScSortItem : public SfxPoolItem
{
public:
                            TYPEINFO();
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
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberUd ) const;

    ScViewData*         GetViewData () const { return pViewData; }
    const ScSortParam&  GetSortData () const { return theSortData; }

private:
    ScViewData*     pViewData;
    ScSortParam     theSortData;
};

//----------------------------------------------------------------------------

class SC_DLLPUBLIC ScQueryItem : public SfxPoolItem
{
public:
                            TYPEINFO();
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
    const ScQueryParam& GetQueryData() const;

    bool        GetAdvancedQuerySource(ScRange& rSource) const;
    void        SetAdvancedQuerySource(const ScRange* pSource);

private:
    boost::scoped_ptr<ScQueryParam> mpQueryData;
    ScViewData*     pViewData;
    ScRange         aAdvSource;
    bool            bIsAdvanced;
};

//----------------------------------------------------------------------------

class SC_DLLPUBLIC ScSubTotalItem : public SfxPoolItem
{
public:
                TYPEINFO();
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
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberUd ) const;

    ScViewData*             GetViewData () const { return pViewData; }
    const ScSubTotalParam&  GetSubTotalData() const { return theSubTotalData; }

private:
    ScViewData*     pViewData;
    ScSubTotalParam theSubTotalData;
};

//----------------------------------------------------------------------------

class SC_DLLPUBLIC ScUserListItem : public SfxPoolItem
{
public:
                TYPEINFO();
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

class ScConsolidateItem : public SfxPoolItem
{
public:
                TYPEINFO();
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

class ScPivotItem : public SfxPoolItem
{
public:
                TYPEINFO();
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

class ScSolveItem : public SfxPoolItem
{
public:
                TYPEINFO();
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

class ScTabOpItem : public SfxPoolItem
{
public:
                TYPEINFO();
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
