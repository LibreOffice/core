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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTSTATISTICSMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTSTATISTICSMANAGER_HXX

#include <IDocumentStatistics.hxx>
#include <vcl/idle.hxx>

class SwDoc;
struct SwDocStat;

namespace sw {

class DocumentStatisticsManager : public IDocumentStatistics
{

public:

    DocumentStatisticsManager( SwDoc& i_rSwdoc );

    void DocInfoChgd(bool isEnableSetModified) override;
    const SwDocStat &GetDocStat() const override;
    void SetDocStatModified(bool bSet);
    const SwDocStat &GetUpdatedDocStat(bool bCompleteAsync, bool bFields) override;
    void SetDocStat(const SwDocStat& rStat) override;
    void UpdateDocStat(bool bCompleteAsync, bool bFields) override;
    virtual ~DocumentStatisticsManager() override;

private:

    DocumentStatisticsManager(DocumentStatisticsManager const&) = delete;
    DocumentStatisticsManager& operator=(DocumentStatisticsManager const&) = delete;

    SwDoc& m_rDoc;

    /** continue computing a chunk of document statistics
      * \param nChars  number of characters to count before exiting
      * \param bFields if stat. fields should be updated
      *
      * returns false when there is no more to calculate
      */
    bool IncrementalDocStatCalculate(long nChars, bool bFields = true);

    // Our own 'StatsUpdateTimer' calls the following method
    DECL_LINK_TYPED( DoIdleStatsUpdate, Timer *, void );


    SwDocStat       *mpDocStat;          //< Statistics information
    bool             mbInitialized;      //< allow first time update
    AutoTimer        maStatsUpdateTimer; //< Timer for asynchronous stats calculation
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
