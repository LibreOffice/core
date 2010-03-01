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

#ifndef TABLAYOUTER_HXX
#define TABLAYOUTER_HXX

#include "svtools/toolpanel/decklayouter.hxx"
#include "svtools/toolpanel/toolpanelcontainer.hxx"
#include "refbase.hxx"

#include <memory>

//........................................................................
namespace svt
{
//........................................................................

    class ToolPanelDeck;

    enum TabAlignment
    {
        TABS_RIGHT,
        TABS_LEFT
    };

    struct TabDeckLayouter_Data;

    //====================================================================
    //= TabDeckLayouter
    //====================================================================
    class TabDeckLayouter   :public RefBase
                            ,public IDeckLayouter
    {
    public:
        /** creates a new layouter
            @param i_eAlignment
                specifies the alignment of the panel selector
            @param i_rPanelDeck
                the panel deck which the layouter is responsible for. Provides access to the panels
                container, and can and should be used as parent for any other windows which the layouter
                needs to create.
        */
        TabDeckLayouter( const TabAlignment i_eAlignment, ToolPanelDeck& i_rPanelDeck );
        ~TabDeckLayouter();

        // IDeckLayouter
        virtual Rectangle   Layout( const Rectangle& i_rDeckPlayground );
        virtual void        Destroy();

        // IReference
        DECLARE_IREFERENCE()

    private:
        ::std::auto_ptr< TabDeckLayouter_Data > m_pData;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // TABLAYOUTER_HXX
