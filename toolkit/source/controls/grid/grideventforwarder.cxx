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

#include "grideventforwarder.hxx"
#include "gridcontrol.hxx"


namespace toolkit
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::awt::grid::GridDataEvent;
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::lang::EventObject;


    //= GridEventForwarder


    GridEventForwarder::GridEventForwarder( UnoGridControl& i_parent )
        :m_parent( i_parent )
    {
    }


    GridEventForwarder::~GridEventForwarder()
    {
    }


    void SAL_CALL GridEventForwarder::acquire() noexcept
    {
        m_parent.acquire();
    }


    void SAL_CALL GridEventForwarder::release() noexcept
    {
        m_parent.release();
    }


    void SAL_CALL GridEventForwarder::rowsInserted( const GridDataEvent& i_event )
    {
        Reference< XGridDataListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->rowsInserted( i_event );
    }


    void SAL_CALL GridEventForwarder::rowsRemoved( const GridDataEvent& i_event )
    {
        Reference< XGridDataListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->rowsRemoved( i_event );
    }


    void SAL_CALL GridEventForwarder::dataChanged( const GridDataEvent& i_event )
    {
        Reference< XGridDataListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->dataChanged( i_event );
    }


    void SAL_CALL GridEventForwarder::rowHeadingChanged( const GridDataEvent& i_event )
    {
        Reference< XGridDataListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->rowHeadingChanged( i_event );
    }


    void SAL_CALL GridEventForwarder::elementInserted( const ContainerEvent& i_event )
    {
        Reference< XContainerListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->elementInserted( i_event );
    }


    void SAL_CALL GridEventForwarder::elementRemoved( const ContainerEvent& i_event )
    {
        Reference< XContainerListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->elementRemoved( i_event );
    }


    void SAL_CALL GridEventForwarder::elementReplaced( const ContainerEvent& i_event )
    {
        Reference< XContainerListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->elementReplaced( i_event );
    }


    void SAL_CALL GridEventForwarder::disposing( const EventObject& i_event )
    {
        Reference< XEventListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->disposing( i_event );
    }


} // namespace toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
