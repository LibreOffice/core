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

#ifndef INCLUDED_SVTOOLS_TOOLPANEL_TABLAYOUTER_HXX
#define INCLUDED_SVTOOLS_TOOLPANEL_TABLAYOUTER_HXX

#include <svtools/svtdllapi.h>
#include <svtools/toolpanel/decklayouter.hxx>
#include <svtools/toolpanel/tabalignment.hxx>
#include <svtools/toolpanel/tabitemcontent.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <memory>

#include <boost/noncopyable.hpp>

namespace vcl { class Window; }


namespace svt
{


    class IToolPanelDeck;

    struct TabDeckLayouter_Data;


    //= TabDeckLayouter

    class SVT_DLLPUBLIC TabDeckLayouter :public IDeckLayouter
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
            vcl::Window& i_rParent,
            IToolPanelDeck& i_rPanels,
            const TabAlignment i_eAlignment,
            const TabItemContent i_eItemContent
        );
        virtual ~TabDeckLayouter();

        // attribute access
        TabItemContent  GetTabItemContent() const;
        void            SetTabItemContent( const TabItemContent& i_eItemContent );
        TabAlignment    GetTabAlignment() const;

        // IDeckLayouter
        virtual Rectangle   Layout( const Rectangle& i_rDeckPlayground ) override;
        virtual void        Destroy() override;
        virtual void        SetFocusToPanelSelector() override;
        virtual size_t      GetAccessibleChildCount() const override;
        virtual css::uno::Reference< css::accessibility::XAccessible >
                            GetAccessibleChild(
                                const size_t i_nChildIndex,
                                const css::uno::Reference< css::accessibility::XAccessible >& i_rParentAccessible
                            ) override;

    private:
        ::std::unique_ptr< TabDeckLayouter_Data > m_pData;
    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_TOOLPANEL_TABLAYOUTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
