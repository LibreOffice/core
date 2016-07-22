/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_UIITEMS_HXX
#define INCLUDED_SC_SOURCE_UI_INC_UIITEMS_HXX

#include "scdllapi.h"
#include "conditio.hxx"
#include "sortparam.hxx"
#include "subtotalparam.hxx"
#include "paramisc.hxx"
#include <svl/poolitem.hxx>

#include <memory>
#include <vector>

namespace editeng {
    struct MisspellRanges;
}

class ScEditEngineDefaulter;
class EditTextObject;
class ScViewData;
class ScDPSaveData;
struct ScQueryParam;

//  Items

class ScInputStatusItem : public SfxPoolItem
{
    ScAddress           aCursorPos;
    ScAddress           aStartPos;
    ScAddress           aEndPos;
    OUString            aString;
    EditTextObject*     pEditData;
    const std::vector<editeng::MisspellRanges>* mpMisspellRanges;

public:

                            ScInputStatusItem( sal_uInt16 nWhich,
                                               const ScAddress& rCurPos,
                                               const ScAddress& rStartPos,
                                               const ScAddress& rEndPos,
                                               const OUString& rString,
                                               const EditTextObject* pData );
                            ScInputStatusItem( const ScInputStatusItem& rItem );
                            virtual ~ScInputStatusItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const ScAddress&        GetPos() const      { return aCursorPos; }

    const OUString&         GetString() const   { return aString; }
    const EditTextObject*   GetEditData() const { return pEditData; }

    void SetMisspellRanges( const std::vector<editeng::MisspellRanges>* pRanges );
    const std::vector<editeng::MisspellRanges>* GetMisspellRanges() const { return mpMisspellRanges;}
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
                    ScTablesHint(sal_uInt16 nNewId, SCTAB nTable1, SCTAB nTable2=0);
                    virtual ~ScTablesHint();

    sal_uInt16      GetId() const           { return nId; }
    SCTAB           GetTab1() const         { return nTab1; }
    SCTAB           GetTab2() const         { return nTab2; }
};

class ScEditViewHint : public SfxHint
{
    ScEditEngineDefaulter*  pEditEngine;
    ScAddress                   aCursorPos;

public:
                    ScEditViewHint() = delete;
                    ScEditViewHint( ScEditEngineDefaulter* pEngine, const ScAddress& rCurPos );
                    virtual ~ScEditViewHint();

    SCCOL           GetCol() const      { return aCursorPos.Col(); }
    SCROW           GetRow() const      { return aCursorPos.Row(); }
    SCTAB           GetTab() const      { return aCursorPos.Tab(); }
    ScEditEngineDefaulter*  GetEngine() const   { return pEditEngine; }
};

class ScIndexHint : public SfxHint
{
    sal_uInt16 nId;
    sal_uInt16 nIndex;

public:
                    ScIndexHint(sal_uInt16 nNewId, sal_uInt16 nIdx);
                    virtual ~ScIndexHint();

    sal_uInt16      GetId() const           { return nId; }
    sal_uInt16      GetIndex() const        { return nIndex; }
};

// Parameter item for the sort dialog:

class SC_DLLPUBLIC ScSortItem : public SfxPoolItem
{
public:
                            ScSortItem( sal_uInt16              nWhich,
                                        ScViewData*         ptrViewData,
                                        const ScSortParam*  pSortData );
                            ScSortItem( sal_uInt16              nWhich,
                                        const ScSortParam*  pSortData );
                            ScSortItem( const ScSortItem& rItem );
                            virtual ~ScSortItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;

    ScViewData*         GetViewData () const { return pViewData; }
    const ScSortParam&  GetSortData () const { return theSortData; }

private:
    ScViewData*     pViewData;
    ScSortParam     theSortData;
};

class SC_DLLPUBLIC ScQueryItem : public SfxPoolItem
{
public:
                            ScQueryItem( sal_uInt16                 nWhich,
                                         ScViewData*            ptrViewData,
                                         const ScQueryParam*    pQueryData );
                            ScQueryItem( sal_uInt16                 nWhich,
                                         const ScQueryParam*    pQueryData );
                            ScQueryItem( const ScQueryItem& rItem );
                            virtual ~ScQueryItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    ScViewData*         GetViewData () const { return pViewData; }
    const ScQueryParam& GetQueryData() const;

    bool        GetAdvancedQuerySource(ScRange& rSource) const;
    void        SetAdvancedQuerySource(const ScRange* pSource);

private:
    std::unique_ptr<ScQueryParam> mpQueryData;
    ScViewData*     pViewData;
    ScRange         aAdvSource;
    bool            bIsAdvanced;
};

class SC_DLLPUBLIC ScSubTotalItem : public SfxPoolItem
{
public:
                ScSubTotalItem( sal_uInt16                  nWhich,
                                ScViewData*             ptrViewData,
                                const ScSubTotalParam*  pSubTotalData );
                ScSubTotalItem( sal_uInt16                  nWhich,
                                const ScSubTotalParam*  pSubTotalData );
                ScSubTotalItem( const ScSubTotalItem&   rItem );
                virtual ~ScSubTotalItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;

    ScViewData*             GetViewData () const { return pViewData; }
    const ScSubTotalParam&  GetSubTotalData() const { return theSubTotalData; }

private:
    ScViewData*     pViewData;
    ScSubTotalParam theSubTotalData;
};

class SC_DLLPUBLIC ScUserListItem : public SfxPoolItem
{
public:
                ScUserListItem( sal_uInt16 nWhich );
                ScUserListItem( const ScUserListItem& rItem );
                virtual ~ScUserListItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    void        SetUserList ( const ScUserList& rUserList );
    ScUserList* GetUserList () const { return pUserList; }

private:
    ScUserList* pUserList;
};

class ScConsolidateItem : public SfxPoolItem
{
public:
                ScConsolidateItem( sal_uInt16                    nWhich,
                                   const ScConsolidateParam* pParam );
                ScConsolidateItem( const ScConsolidateItem& rItem );
                virtual ~ScConsolidateItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const ScConsolidateParam& GetData() const { return theConsData; }

private:
    ScConsolidateParam  theConsData;
};

class ScPivotItem : public SfxPoolItem
{
public:
                ScPivotItem( sal_uInt16 nWhich, const ScDPSaveData* pData,
                             const ScRange* pRange, bool bNew );
                ScPivotItem( const ScPivotItem& rItem );
                virtual ~ScPivotItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const ScDPSaveData& GetData() const         { return *pSaveData; }
    const ScRange&      GetDestRange() const    { return aDestRange; }
    bool                IsNewSheet() const      { return bNewSheet; }

private:
    ScDPSaveData*   pSaveData;
    ScRange         aDestRange;
    bool            bNewSheet;
};

class ScSolveItem : public SfxPoolItem
{
public:
                ScSolveItem( sal_uInt16              nWhich,
                             const ScSolveParam* pParam );
                ScSolveItem( const ScSolveItem& rItem );
                virtual ~ScSolveItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const ScSolveParam& GetData() const { return theSolveData; }

private:
    ScSolveParam    theSolveData;
};

class ScTabOpItem : public SfxPoolItem
{
public:
                ScTabOpItem( sal_uInt16              nWhich,
                             const ScTabOpParam* pParam );
                ScTabOpItem( const ScTabOpItem& rItem );
                virtual ~ScTabOpItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const ScTabOpParam& GetData() const { return theTabOpData; }

private:
    ScTabOpParam    theTabOpData;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
