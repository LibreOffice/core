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

#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <svl/listener.hxx>
#include "rangelst.hxx"
#include "token.hxx"
#include "externalrefmgr.hxx"

#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
        virtual ~ExternalRefListener() override;
        virtual void notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType) override;
        void addFileId(sal_uInt16 nFileId);
        void removeFileId(sal_uInt16 nFileId);
        std::unordered_set<sal_uInt16>& getAllFileIds() { return maFileIds;}

    private:
        ExternalRefListener(const ExternalRefListener& r) = delete;

        ScChartListener& mrParent;
        std::unordered_set<sal_uInt16> maFileIds;
        ScDocument*                 mpDoc;
    };

private:

    std::unique_ptr<ExternalRefListener> mpExtRefListener;
    std::unique_ptr<std::vector<ScTokenRef> > mpTokens;

    OUString maName;
    ScChartUnoData* pUnoData;
    ScDocument*     mpDoc;
    bool            bUsed:1;  // for ScChartListenerCollection::FreeUnused
    bool            bDirty:1;
    bool            bSeriesRangesScheduled:1;

    ScChartListener& operator=( const ScChartListener& ) = delete;

public:
    ScChartListener( const OUString& rName, ScDocument* pDoc,
                     const ScRangeListRef& rRangeListRef );
    ScChartListener( const OUString& rName, ScDocument* pDoc,
                     ::std::vector<ScTokenRef>* pTokens );
    ScChartListener( const ScChartListener& );
    virtual ~ScChartListener() override;

    const OUString& GetName() const { return maName;}

    void            SetUno( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& rListener,
                            const css::uno::Reference< css::chart::XChartData >& rSource );
    css::uno::Reference< css::chart::XChartDataChangeEventListener >  GetUnoListener() const;
    css::uno::Reference< css::chart::XChartData >                     GetUnoSource() const;

    bool            IsUno() const   { return (pUnoData != nullptr); }

    virtual void Notify( const SfxHint& rHint ) override;
    void            StartListeningTo();
    void            EndListeningTo();
    void            ChangeListening( const ScRangeListRef& rRangeListRef,
                                    bool bDirty );
    void            Update();
    ScRangeListRef  GetRangeList() const;
    void            SetRangeList( const ScRangeListRef& rNew );
    bool            IsUsed() const { return bUsed; }
    void            SetUsed( bool bFlg ) { bUsed = bFlg; }
    bool            IsDirty() const { return bDirty; }
    void            SetDirty( bool bFlg ) { bDirty = bFlg; }

    void            UpdateChartIntersecting( const ScRange& rRange );

    // if chart series ranges are to be updated later on (e.g. DeleteTab, InsertTab)
    void            UpdateScheduledSeriesRanges();
    void            UpdateSeriesRanges();

    ExternalRefListener* GetExtRefListener();
    void            SetUpdateQueue();

    bool operator==( const ScChartListener& ) const;
    bool operator!=( const ScChartListener& r ) const;
};

class SC_DLLPUBLIC ScChartHiddenRangeListener
{
public:
    ScChartHiddenRangeListener();
    virtual ~ScChartHiddenRangeListener();
    virtual void notify() = 0;
};

class SC_DLLPUBLIC ScChartListenerCollection
{
public:
    typedef std::map<OUString, std::unique_ptr<ScChartListener>> ListenersType;
    typedef std::unordered_set<OUString, OUStringHash> StringSetType;
private:
    ListenersType m_Listeners;
    enum UpdateStatus
    {
        SC_CLCUPDATE_NONE,
        SC_CLCUPDATE_RUNNING,
        SC_CLCUPDATE_MODIFIED
    } meModifiedDuringUpdate;

    std::unordered_multimap<ScChartHiddenRangeListener*, ScRange> maHiddenListeners;

    StringSetType maNonOleObjectNames;

    Idle            aIdle;
    ScDocument*     pDoc;

                    DECL_LINK_TYPED(TimerHdl, Idle *, void);

    ScChartListenerCollection& operator=( const ScChartListenerCollection& ) = delete;

protected:
    void Init();

public:
    ScChartListenerCollection( ScDocument* pDoc );
    ScChartListenerCollection( const ScChartListenerCollection& );
    ~ScChartListenerCollection();

                    // only needed after copy-ctor, if newly added to doc
    void            StartAllListeners();

    void insert(ScChartListener* pListener);
    ScChartListener* findByName(const OUString& rName);
    const ScChartListener* findByName(const OUString& rName) const;
    bool hasListeners() const;

    void removeByName(const OUString& rName);

    const ListenersType& getListeners() const { return m_Listeners; }
    ListenersType& getListeners() { return m_Listeners; }
    StringSetType& getNonOleObjectNames() { return maNonOleObjectNames;}

    /**
     * Create a unique name that's not taken by any existing chart listener
     * objects.  The name consists of a prefix given followed by a number.
     */
    OUString getUniqueName(const OUString& rPrefix) const;

    void            ChangeListening( const OUString& rName,
                                    const ScRangeListRef& rRangeListRef );
    // use FreeUnused only the way it's used in ScDocument::UpdateChartListenerCollection
    void            FreeUnused();
    void            FreeUno( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& rListener,
                             const css::uno::Reference< css::chart::XChartData >& rSource );
    void            StartTimer();
    void            UpdateDirtyCharts();
    void            SetDirty();
    void            SetDiffDirty( const ScChartListenerCollection&,
                        bool bSetChartRangeLists );

    void            SetRangeDirty( const ScRange& rRange );     // for example rows/columns

    void            UpdateScheduledSeriesRanges();
    void            UpdateChartsContainingTab( SCTAB nTab );

    bool operator==( const ScChartListenerCollection& r ) const;

    /**
     * Start listening on hide/show change within specified cell range.  A
     * single listener may listen on multiple ranges when the caller passes
     * the same pointer multiple times with different ranges.
     *
     * Note that the caller is responsible for managing the life-cycle of the
     * listener instance.
     */
    void            StartListeningHiddenRange( const ScRange& rRange,
                                               ScChartHiddenRangeListener* pListener );

    /**
     * Remove all ranges associated with passed listener instance from the
     * list of hidden range listeners.  This does not delete the passed
     * listener instance.
     */
    void            EndListeningHiddenRange( ScChartHiddenRangeListener* pListener );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
