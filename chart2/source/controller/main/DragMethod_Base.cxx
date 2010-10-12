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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "DragMethod_Base.hxx"

#include "Strings.hrc"
#include "ResId.hxx"
#include "macros.hxx"
#include "ObjectNameProvider.hxx"
#include "ObjectIdentifier.hxx"
#include <rtl/math.hxx>
//header for class SdrPageView
#include <svx/svdpagv.hxx>
#include <svx/ActionDescriptionProvider.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::WeakReference;

DragMethod_Base::DragMethod_Base( DrawViewWrapper& rDrawViewWrapper
                                             , const rtl::OUString& rObjectCID
                                             , const Reference< frame::XModel >& xChartModel
                                             , ActionDescriptionProvider::ActionType eActionType )
    : SdrDragMethod( rDrawViewWrapper )
    , m_rDrawViewWrapper(rDrawViewWrapper)
    , m_aObjectCID(rObjectCID)
    , m_eActionType( eActionType )
    , m_xChartModel( WeakReference< frame::XModel >(xChartModel) )
{
    setMoveOnly(true);
}
DragMethod_Base::~DragMethod_Base()
{
}

Reference< frame::XModel > DragMethod_Base::getChartModel() const
{
    return Reference< frame::XModel >( m_xChartModel );;
}

rtl::OUString DragMethod_Base::getUndoDescription() const
{
    return ActionDescriptionProvider::createDescription(
                m_eActionType,
                ObjectNameProvider::getName( ObjectIdentifier::getObjectType( m_aObjectCID )));
}
void DragMethod_Base::TakeSdrDragComment(String& rStr) const
{
    rStr = String( getUndoDescription() );
}
Pointer DragMethod_Base::GetSdrDragPointer() const
{
    if( IsDraggingPoints() || IsDraggingGluePoints() )
        return Pointer(POINTER_MOVEPOINT);
    else
        return Pointer(POINTER_MOVE);
}
//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
