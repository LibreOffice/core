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
#ifndef CHART2_DRAGMETHOD_BASE_HXX
#define CHART2_DRAGMETHOD_BASE_HXX

#include "DrawViewWrapper.hxx"
#include <svx/ActionDescriptionProvider.hxx>
// header for class SdrDragMethod
#include <svx/svddrgmt.hxx>

// header for class WeakReference
#include <cppuhelper/weakref.hxx>

namespace chart
{

class DragMethod_Base : public SdrDragMethod
{
public:
    DragMethod_Base( DrawViewWrapper& rDrawViewWrapper, const rtl::OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
        , ActionDescriptionProvider::ActionType eActionType = ActionDescriptionProvider::MOVE );
    virtual ~DragMethod_Base();

    virtual rtl::OUString getUndoDescription() const;

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual Pointer GetSdrDragPointer() const;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getChartModel() const;

protected:
    DrawViewWrapper&    m_rDrawViewWrapper;
    rtl::OUString       m_aObjectCID;
    ActionDescriptionProvider::ActionType m_eActionType;

private:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel > m_xChartModel;
};

} //  namespace chart

// CHART2_DRAGMETHOD_BASE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
