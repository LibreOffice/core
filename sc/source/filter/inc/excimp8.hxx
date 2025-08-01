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

#pragma once

#include "imp_op.hxx"
#include "root.hxx"
#include "excscen.hxx"
#include <queryparam.hxx>

class ScDBData;
class XclImpStream;

class ImportExcel8 final : public ImportExcel
{
public:
                            ImportExcel8( XclImpRootData& rImpData, SvStream& rStrm );
    virtual                 ~ImportExcel8() override;

    virtual ErrCode         Read() override;

    void                    Calccount();              // 0x0C
    void                    Precision();              // 0x0E
    void                    Delta();                  // 0x10
    void                    Iteration();              // 0x11
    void                    Boundsheet();             // 0x85
    void                    FilterMode();             // 0x9B
    void                    AutoFilterInfo();         // 0x9D
    void                    AutoFilter();             // 0x9E
    void                    Scenman();                // 0xAE
    void                    Scenario();               // 0xAF
    void                    ReadBasic();              // 0xD3
    void                    Labelsst();               // 0xFD

    void                    FeatHdr();                // 0x0867
    void                    Feat();                   // 0x0868

    virtual void            EndSheet() override;
    virtual void            PostDocLoad() override;

private:
    ExcScenarioList maScenList;
};


class XclImpAutoFilterData : private ExcRoot
{
private:
    ScDBData*                   pCurrDBData;
    ScQueryParam                aParam;
    ScRange                     aCriteriaRange;
    bool                        bActive:1;
    bool                        bCriteria:1;
    bool                        bAutoOrAdvanced:1;

    void                        SetCellAttribs();
    void                        InsertQueryParam();

protected:
public:
                                XclImpAutoFilterData(
                                    RootData* pRoot,
                                    const ScRange& rRange);

    bool                 IsActive() const    { return bActive; }
    bool                 IsFiltered() const  { return bAutoOrAdvanced; }
    SCTAB                Tab() const         { return aParam.nTab; }
    SCCOL                StartCol() const    { return aParam.nCol1; }
    SCROW                StartRow() const    { return aParam.nRow1; }
    SCCOL                EndCol() const      { return aParam.nCol2; }
    SCROW                EndRow() const      { return aParam.nRow2; }

    void ReadAutoFilter( XclImpStream& rStrm, svl::SharedStringPool& rPool );

    void                 Activate()          { bActive = true; }
    void                        SetAdvancedRange( const ScRange* pRange );
    void                        SetExtractPos( const ScAddress& rAddr );
    void                 SetAutoOrAdvanced()  { bAutoOrAdvanced = true; }
    void                        Apply();
    void                        EnableRemoveFilter();
};

class XclImpAutoFilterBuffer
{
public:

    void                        Insert( RootData* pRoot, const ScRange& rRange);
    void                        AddAdvancedRange( const ScRange& rRange );
    void                        AddExtractPos( const ScRange& rRange );
    void                        Apply();

    XclImpAutoFilterData*       GetByTab( SCTAB nTab );

private:
    typedef std::shared_ptr<XclImpAutoFilterData> XclImpAutoFilterSharePtr;
    std::vector<XclImpAutoFilterSharePtr> maFilters;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
