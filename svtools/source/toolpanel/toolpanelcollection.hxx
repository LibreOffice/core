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

#ifndef INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TOOLPANELCOLLECTION_HXX
#define INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TOOLPANELCOLLECTION_HXX

#include <svtools/toolpanel/toolpaneldeck.hxx>

#include <memory>


namespace svt
{


    struct ToolPanelCollection_Data;


    //= ToolPanelCollection

    class ToolPanelCollection : public IToolPanelDeck
    {
    public:
        ToolPanelCollection();
        virtual ~ToolPanelCollection();

        // IToolPanelDeck
        virtual size_t      GetPanelCount() const override;
        virtual PToolPanel  GetPanel( const size_t i_nPos ) const override;
        virtual ::boost::optional< size_t >
                            GetActivePanel() const override;
        virtual void        ActivatePanel( const ::boost::optional< size_t >& i_rPanel ) override;
        virtual size_t      InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition ) override;
        virtual PToolPanel  RemovePanel( const size_t i_nPosition ) override;
        virtual void        AddListener( IToolPanelDeckListener& i_rListener ) override;
        virtual void        RemoveListener( IToolPanelDeckListener& i_rListener ) override;

    private:
        ::std::unique_ptr< ToolPanelCollection_Data > m_pData;
    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TOOLPANELCOLLECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
