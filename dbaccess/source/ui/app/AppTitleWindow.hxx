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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_APP_APPTITLEWINDOW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_APP_APPTITLEWINDOW_HXX

#include <vcl/fixed.hxx>

namespace dbaui
{
    class OTitleWindow : public vcl::Window
    {
        VclPtr<FixedText>   m_aSpace1;
        VclPtr<FixedText>   m_aSpace2;
        VclPtr<FixedText>   m_aTitle;
        VclPtr<vcl::Window> m_pChild;
        bool                m_bShift;
        void ImplInitSettings();
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    public:
        OTitleWindow(vcl::Window* _pParent, const char* pTitleId, WinBits _nBits, bool _bShift = true);
        virtual ~OTitleWindow() override;
        virtual void dispose() override;

        // Window overrides
        virtual void Resize() override;
        virtual void GetFocus() override;

        virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

        /** sets the child window which should be displayed below the title. It will be destroyed at the end.
            @param  _pChild
                The child window.
        */
        void setChildWindow(vcl::Window* _pChild);

        /** gets the child window.

            @return
                The child window.
        */
        vcl::Window* getChildWindow() const { return m_pChild; }

        /** sets the title text out of the resource
            @param  pTitleId
                The resource id of the title text.
        */
        void setTitle(const char* pTitleId);

        /** Gets the min Width in Pixel which is needed to display the whole

            @return
                the min width
        */
        long GetWidthPixel() const;
    };
} // namespace dbaui
#endif // INCLUDED_DBACCESS_SOURCE_UI_APP_APPTITLEWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
