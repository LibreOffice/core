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

#ifndef INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_DUMMYPANEL_HXX
#define INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_DUMMYPANEL_HXX

#include "svtools/toolpanel/toolpanel.hxx"
#include "svtools/toolpanel/refbase.hxx"


namespace svt
{



    //= DummyPanel

    /// is a dummy implementation of the IToolPanel interface
    class DummyPanel    :public RefBase
                        ,public IToolPanel
    {
    public:
        DummyPanel();
        ~DummyPanel();

        // IToolPanel
        virtual OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual OString GetHelpID() const;
        virtual void Activate( Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                    CreatePanelAccessible(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                    );

        DECLARE_IREFERENCE()
    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_DUMMYPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
