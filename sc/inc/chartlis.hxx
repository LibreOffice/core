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

#ifndef SC_CHARTLIS_HXX
#define SC_CHARTLIS_HXX


#include <vcl/timer.hxx>
#include <svl/listener.hxx>
#include "rangelst.hxx"
#include "token.hxx"
#include "externalrefmgr.hxx"

#include <memory>
#include <vector>
#include <list>

#include <boost/unordered_set.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_map.hpp>

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
        virtual void notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType);
        void addFileId(sal_uInt16 nFileId);
        void removeFileId(sal_uInt16 nFileId);
        ::boost::unordered_set<sal_uInt16>& getAllFileIds();

    private:
        ExternalRefListener();
        ExternalRefListener(const ExternalRefListener& r);

        ScChartListener& mrParent;
        ::boost::unordered_set<sal_uInt16> maFileIds;
        ScDocument*                 mpDoc;
    };

private:

    boost::scoped_ptr<ExternalRefListener> mpExtRefListener;
    boost::scoped_ptr<std::vector<ScTokenRef> > mpTokens;

    rtl::OUString maName;
    ScChartUnoData* pUnoData;
    ScDocument*     pDoc;
    bool            bUsed:1;  // for ScChartListenerCollection::FreeUnused
    bool            bDirty:1;
    bool            bSeriesRangesScheduled:1;

                    // not implemented
    ScChartListener& operator=( const ScChartListener& );

public:
    ScChartListener( const rtl::OUString& rName, ScDocument* pDoc,
                     const ScRangeListRef& rRangeListRef );
    ScChartListener( const rtl::OUString& rName, ScDocument* pDoc,
                     ::std::vector<ScTokenRef>* pTokens );
    ScChartListener( const ScChartListener& );
    ~ScChartListener();

    const rtl::OUString& GetName() const;
    void SetName(const rtl::OUString& rName);

    void            SetUno( const com::sun::star::uno::Reference< com::sun::star::chart::XChartDataChangeEventListener >& rListener,
                            const com::sun::star::uno::Reference< com::sun::star::chart::XChartData >& rSource );
    com::sun::star::uno::Reference< com::sun::star::chart::XChartDataChangeEventListener >  GetUnoListener() const;
    com::sun::star::uno::Reference< com::sun::star::chart::XChartData >                     GetUnoSource() const;

    bool            IsUno() const   { return (pUnoData != NULL); }

    virtual void    Notify( SvtBroadcaster& rBC, const SfxHint& rHint );
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

// ============================================================================

class ScChartHiddenRangeListener
{
public:
    ScChartHiddenRangeListener();
    virtual ~ScChartHiddenRangeListener();
    virtual void notify() = 0;
};

// ============================================================================

class ScChartListenerCollection
{
public:
    struct RangeListenerItem
    {
        ScRange                     maRange;
        ScChartHiddenRangeListener* mpListener;
        explicit RangeListenerItem(const ScRange& rRange, ScChartHiddenRangeListener* p);
    };

    typedef boost::ptr_map<rtl::OUString, ScChartListener> ListenersType;
    typedef boost::unordered_set<rtl::OUString, rtl::OUStringHash> StringSetType;
private:
    ListenersType maListeners;
    ::std::list<RangeListenerItem> maHiddenListeners;
    StringSetType maNonOleObjectNames;

    Timer           aTimer;
    ScDocument*     pDoc;

                    DECL_LINK(TimerHdl, void *);

                    // not implemented
    ScChartListenerCollection& operator=( const ScChartListenerCollection& );

public:
    ScChartListenerCollection( ScDocument* pDoc );
    ScChartListenerCollection( const ScChartListenerCollection& );
    ~ScChartListenerCollection();

                    // only needed after copy-ctor, if newly added to doc
    void            StartAllListeners();

    SC_DLLPUBLIC void insert(ScChartListener* pListener);
    ScChartListener* findByName(const rtl::OUString& rName);
    const ScChartListener* findByName(const rtl::OUString& rName) const;
    bool hasListeners() const;

    void removeByName(const rtl::OUString& rName);

    const ListenersType& getListeners() const;
    ListenersType& getListeners();
    StringSetType& getNonOleObjectNames();

    /**
     * Create a unique name that's not taken by any existing chart listener
     * objects.  The name consists of a prefix given followed by a number.
     */
    rtl::OUString getUniqueName(const rtl::OUString& rPrefix) const;

    void            ChangeListening( const String& rName,
                                    const ScRangeListRef& rRangeListRef,
                                    bool bDirty = false );
    // use FreeUnused only the way it's used in ScDocument::UpdateChartListenerCollection
    void            FreeUnused();
    void            FreeUno( const com::sun::star::uno::Reference< com::sun::star::chart::XChartDataChangeEventListener >& rListener,
                             const com::sun::star::uno::Reference< com::sun::star::chart::XChartData >& rSource );
    void            StartTimer();
    void            UpdateDirtyCharts();
    SC_DLLPUBLIC void SetDirty();
    void            SetDiffDirty( const ScChartListenerCollection&,
                        bool bSetChartRangeLists = false );

    void            SetRangeDirty( const ScRange& rRange );     // for example rows/columns

    void            UpdateScheduledSeriesRanges();
    void            UpdateChartsContainingTab( SCTAB nTab );

    bool operator==( const ScChartListenerCollection& r ) const;
    bool operator!=( const ScChartListenerCollection& r ) const;

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
