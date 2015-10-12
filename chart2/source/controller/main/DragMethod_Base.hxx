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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_DRAGMETHOD_BASE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_DRAGMETHOD_BASE_HXX

#include "DrawViewWrapper.hxx"
#include <svx/ActionDescriptionProvider.hxx>
#include <svx/svddrgmt.hxx>
#include <cppuhelper/weakref.hxx>

namespace chart
{

class DragMethod_Base : public SdrDragMethod
{
public:
    DragMethod_Base( DrawViewWrapper& rDrawViewWrapper, const OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
        , ActionDescriptionProvider::ActionType eActionType = ActionDescriptionProvider::MOVE );
    virtual ~DragMethod_Base();

    OUString getUndoDescription() const;

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual Pointer GetSdrDragPointer() const override;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getChartModel() const;

protected:
    DrawViewWrapper&    m_rDrawViewWrapper;
    OUString       m_aObjectCID;
    ActionDescriptionProvider::ActionType m_eActionType;

private:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel > m_xChartModel;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_DRAGMETHOD_BASE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
