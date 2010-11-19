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

#ifndef TOOLPANELCOLLECTION_HXX
#define TOOLPANELCOLLECTION_HXX

#include "svtools/toolpaneldeck.hxx"

#include <memory>

//........................................................................
namespace svt
{
//........................................................................

    struct ToolPanelCollection_Data;

    //====================================================================
    //= ToolPanelCollection
    //====================================================================
    class ToolPanelCollection : public IToolPanelDeck
    {
    public:
        ToolPanelCollection();
        ~ToolPanelCollection();

        // IToolPanelDeck
        virtual size_t      GetPanelCount() const;
        virtual PToolPanel  GetPanel( const size_t i_nPos ) const;
        virtual ::boost::optional< size_t >
                            GetActivePanel() const;
        virtual void        ActivatePanel( const ::boost::optional< size_t >& i_rPanel );
        virtual size_t      InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual PToolPanel  RemovePanel( const size_t i_nPosition );
        virtual void        AddListener( IToolPanelDeckListener& i_rListener );
        virtual void        RemoveListener( IToolPanelDeckListener& i_rListener );

    private:
        ::std::auto_ptr< ToolPanelCollection_Data > m_pData;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // TOOLPANELCOLLECTION_HXX
