/*************************************************************************
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

#ifndef TOOLPANELCONTAINER_HXX
#define TOOLPANELCONTAINER_HXX

#include "svtools/toolpanel/toolpanel.hxx"

#include <rtl/ref.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= IToolPanelContainerListener
    //====================================================================
    class SAL_NO_VTABLE IToolPanelContainerListener
    {
    public:
        virtual void PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition ) = 0;
    };

    //====================================================================
    //= IToolPanelContainer
    //====================================================================
    class IToolPanelContainer : public ::rtl::IReference
    {
    public:
        /** returns the number of panels in the container
        */
        virtual size_t      GetPanelCount() const = 0;

        /** retrieves the panel with the given index. Invalid indexes will be reported via an assertion in the
            non-product version, and silently ignored in the product version, with a NULL panel being returned.
        */
        virtual PToolPanel  GetPanel( const size_t i_nPos ) const = 0;

        /** inserts a new panel into the container. NULL panels are not allowed, as are positions greater than the
            current panel count. Violations of this will be reported via an assertion in the non-product version, and
            silently ignored in the product version.
        */
        virtual size_t      InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition ) = 0;

        /** adds a new listener to be notified when the container content changes. The caller is responsible
            for life time control, i.e. removing the listener before it actually dies.
        */
        virtual void        AddListener( IToolPanelContainerListener& i_rListener ) = 0;

        /** removes a container listener previously added via addListener.
        */
        virtual void        RemoveListener( IToolPanelContainerListener& i_rListener ) = 0;
    };

    typedef ::rtl::Reference< IToolPanelContainer > PToolPanelContainer;

//........................................................................
} // namespace svt
//........................................................................

#endif // TOOLPANELCONTAINER_HXX
