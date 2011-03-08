/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SVT_TABLAYOUTER_HXX
#define SVT_TABLAYOUTER_HXX

#include "svtools/svtdllapi.h"
#include "svtools/toolpanel/decklayouter.hxx"
#include "svtools/toolpanel/tabalignment.hxx"
#include "svtools/toolpanel/tabitemcontent.hxx"
#include "svtools/toolpanel/refbase.hxx"

#include <memory>

#include <boost/noncopyable.hpp>

class Window;

//........................................................................
namespace svt
{
//........................................................................

    class IToolPanelDeck;

    struct TabDeckLayouter_Data;

    //====================================================================
    //= TabDeckLayouter
    //====================================================================
    class SVT_DLLPUBLIC TabDeckLayouter :public RefBase
                                        ,public IDeckLayouter
                                        ,public ::boost::noncopyable
    {
    public:
        /** creates a new layouter
            @param i_rParent
                is the parent window for any VCL windows the layouter needs to create.
            @param i_rPanels
                is the panel deck which the layouter is responsible for.
            @param i_eAlignment
                specifies the alignment of the panel selector
            @param TabItemContent
                specifies the content to show on the tab items
        */
        TabDeckLayouter(
            Window& i_rParent,
            IToolPanelDeck& i_rPanels,
            const TabAlignment i_eAlignment,
            const TabItemContent i_eItemContent
        );
        ~TabDeckLayouter();

        // attribute access
        TabItemContent  GetTabItemContent() const;
        void            SetTabItemContent( const TabItemContent& i_eItemContent );
        TabAlignment    GetTabAlignment() const;

        // helpers for the A11Y implementation
        ::boost::optional< size_t >
                        GetFocusedPanelItem() const;
        void            FocusPanelItem( const size_t i_nItemPos );
        bool            IsPanelSelectorEnabled() const;
        bool            IsPanelSelectorVisible() const;
        Rectangle       GetItemScreenRect( const size_t i_nItemPos ) const;

        // IDeckLayouter
        virtual Rectangle   Layout( const Rectangle& i_rDeckPlayground );
        virtual void        Destroy();
        virtual void        SetFocusToPanelSelector();
        virtual size_t      GetAccessibleChildCount() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                            GetAccessibleChild(
                                const size_t i_nChildIndex,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                            );

        // IReference
        DECLARE_IREFERENCE()

    private:
        ::std::auto_ptr< TabDeckLayouter_Data > m_pData;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_TABLAYOUTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
