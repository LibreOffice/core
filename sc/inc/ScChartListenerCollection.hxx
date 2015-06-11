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

#ifndef INCLUDED_SC_INC_CHARTLISTENERCOLLECTION_HXX
#define INCLUDED_SC_INC_CHARTLISTENERCOLLECTION_HXX

#include "address.hxx"
#include "rangelst.hxx"

#include "com/sun/star/chart/XChartData.hpp"
#include "com/sun/star/chart/XChartDataChangeEventListener.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ustring.hxx"
#include "vcl/idle.hxx"

#include <boost/ptr_container/ptr_map.hpp>
#include <unordered_map>
#include <unordered_set>

class ScChartHiddenRangeListener;
class ScChartListener;
class ScDocument;

class ScChartListenerCollection
{
public:
    struct RangeListenerItem
    {
        ScRange                     maRange;
        ScChartHiddenRangeListener* mpListener;
        RangeListenerItem() : mpListener(0) {}
        explicit RangeListenerItem(const ScRange& rRange, ScChartHiddenRangeListener* p);
    };

    typedef boost::ptr_map<OUString, ScChartListener> ListenersType;
    typedef std::unordered_set<OUString, OUStringHash> StringSetType;
private:
    ListenersType maListeners;
    enum UpdateStatus
    {
        SC_CLCUPDATE_NONE,
        SC_CLCUPDATE_RUNNING,
        SC_CLCUPDATE_MODIFIED
    } meModifiedDuringUpdate;

    std::unordered_map<ScChartHiddenRangeListener*, RangeListenerItem> maHiddenListeners;
    StringSetType maNonOleObjectNames;

    Idle            aIdle;
    ScDocument*     pDoc;

                    DECL_LINK_TYPED(TimerHdl, Idle *, void);

    ScChartListenerCollection& operator=( const ScChartListenerCollection& ) SAL_DELETED_FUNCTION;

public:
    ScChartListenerCollection( ScDocument* pDoc );
    ScChartListenerCollection( const ScChartListenerCollection& );
    ~ScChartListenerCollection();

                    // only needed after copy-ctor, if newly added to doc
    void            StartAllListeners();

    SC_DLLPUBLIC void insert(ScChartListener* pListener);
    ScChartListener* findByName(const OUString& rName);
    const ScChartListener* findByName(const OUString& rName) const;
    bool hasListeners() const;

    void removeByName(const OUString& rName);

    const ListenersType& getListeners() const { return maListeners;}
    ListenersType& getListeners() { return maListeners;}
    StringSetType& getNonOleObjectNames() { return maNonOleObjectNames;}

    /**
     * Create a unique name that's not taken by any existing chart listener
     * objects.  The name consists of a prefix given followed by a number.
     */
    OUString getUniqueName(const OUString& rPrefix) const;

    void            ChangeListening( const OUString& rName,
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
