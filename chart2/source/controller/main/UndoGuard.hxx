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
#if 1

#include "ChartModelClone.hxx"

#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <rtl/ustring.hxx>

#include <boost/shared_ptr.hpp>

namespace chart
{

/** A guard which which does nothing, unless you explicitly call commitAction. In particular, in its destructor, it
    does neither auto-commit nor auto-rollback the model changes.
 */
class UndoGuard
{
public:
    explicit UndoGuard(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager,
        const ModelFacet i_facet = E_MODEL
    );
    ~UndoGuard();

    void    commit();
    void    rollback();

protected:
    bool    isActionPosted() const { return m_bActionPosted; }

private:
    void    discardSnapshot();

private:
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >           m_xChartModel;
    const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager >  m_xUndoManager;

    ::boost::shared_ptr< ChartModelClone >  m_pDocumentSnapshot;
    rtl::OUString                           m_aUndoString;
    bool                                    m_bActionPosted;
};

/** A guard which, in its destructor, restores the model state it found in the constructor. If
    <member>commitAction</member> is called inbetween, the restouration is not performed.
 */
class UndoLiveUpdateGuard : public UndoGuard
{
public:
    explicit UndoLiveUpdateGuard(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    ~UndoLiveUpdateGuard();
};

/** Same as UndoLiveUpdateGuard but with additional storage of the chart's data.
    Only use this if the data has internal data.
 */
class UndoLiveUpdateGuardWithData :
        public UndoGuard
{
public:
    explicit UndoLiveUpdateGuardWithData(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    ~UndoLiveUpdateGuardWithData();
};

class UndoGuardWithSelection : public UndoGuard
{
public:
    explicit UndoGuardWithSelection(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    virtual ~UndoGuardWithSelection();
};

class HiddenUndoContext
{
public:
    HiddenUndoContext(
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    ~HiddenUndoContext();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager >    m_xUndoManager;
};

}
// CHART2_UNDOGUARD_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
