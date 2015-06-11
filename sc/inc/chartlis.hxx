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

#ifndef INCLUDED_SC_INC_CHARTLIS_HXX
#define INCLUDED_SC_INC_CHARTLIS_HXX

#include <svl/listener.hxx>
#include "rangelst.hxx"
#include "token.hxx"
#include "externalrefmgr.hxx"

#include <vector>

#include <boost/scoped_ptr.hpp>
#include <unordered_set>

class ScDocument;
class ScChartUnoData;
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart/XChartDataChangeEventListener.hpp>

class SC_DLLPUBLIC ScChartListener : public SvtListener
{
public:
    class ExternalRefListener : public ScExternalRefManager::LinkListener
    {
    public:
        ExternalRefListener(ScChartListener& rParent, ScDocument* pDoc);
        virtual ~ExternalRefListener();
        virtual void notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType) SAL_OVERRIDE;
        void addFileId(sal_uInt16 nFileId);
        void removeFileId(sal_uInt16 nFileId);
        std::unordered_set<sal_uInt16>& getAllFileIds() { return maFileIds;}

    private:
        ExternalRefListener(const ExternalRefListener& r) SAL_DELETED_FUNCTION;

        ScChartListener& mrParent;
        std::unordered_set<sal_uInt16> maFileIds;
        ScDocument*                 mpDoc;
    };

private:

    boost::scoped_ptr<ExternalRefListener> mpExtRefListener;
    boost::scoped_ptr<std::vector<ScTokenRef> > mpTokens;

    OUString maName;
    ScChartUnoData* pUnoData;
    ScDocument*     mpDoc;
    bool            bUsed:1;  // for ScChartListenerCollection::FreeUnused
    bool            bDirty:1;
    bool            bSeriesRangesScheduled:1;

    ScChartListener& operator=( const ScChartListener& ) SAL_DELETED_FUNCTION;

public:
    ScChartListener( const OUString& rName, ScDocument* pDoc,
                     const ScRangeListRef& rRangeListRef );
    ScChartListener( const OUString& rName, ScDocument* pDoc,
                     ::std::vector<ScTokenRef>* pTokens );
    ScChartListener( const ScChartListener& );
    virtual ~ScChartListener();

    const OUString& GetName() const { return maName;}

    void            SetUno( const com::sun::star::uno::Reference< com::sun::star::chart::XChartDataChangeEventListener >& rListener,
                            const com::sun::star::uno::Reference< com::sun::star::chart::XChartData >& rSource );
    com::sun::star::uno::Reference< com::sun::star::chart::XChartDataChangeEventListener >  GetUnoListener() const;
    com::sun::star::uno::Reference< com::sun::star::chart::XChartData >                     GetUnoSource() const;

    bool            IsUno() const   { return (pUnoData != NULL); }

    virtual void Notify( const SfxHint& rHint ) SAL_OVERRIDE;
    void            StartListeningTo();
    void            EndListeningTo();
    void            ChangeListening( const ScRangeListRef& rRangeListRef,
                                    bool bDirty = false );
    void            Update();
    ScRangeListRef  GetRangeList() const;
    void            SetRangeList( const ScRangeListRef& rNew );
    bool            IsUsed() const { return bUsed; }
    void            SetUsed( bool bFlg ) { bUsed = bFlg; }
    bool            IsDirty() const { return bDirty; }
    void            SetDirty( bool bFlg ) { bDirty = bFlg; }

    void            UpdateChartIntersecting( const ScRange& rRange );

    // if chart series ranges are to be updated later on (e.g. DeleteTab, InsertTab)
    void            ScheduleSeriesRanges()      { bSeriesRangesScheduled = true; }
    void            UpdateScheduledSeriesRanges();
    void            UpdateSeriesRanges();

    ExternalRefListener* GetExtRefListener();
    void            SetUpdateQueue();

    bool operator==( const ScChartListener& ) const;
    bool operator!=( const ScChartListener& r ) const;
};

class ScChartHiddenRangeListener
{
public:
    ScChartHiddenRangeListener();
    virtual ~ScChartHiddenRangeListener();
    virtual void notify() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
