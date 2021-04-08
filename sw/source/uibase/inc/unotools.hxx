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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UNOTOOLS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UNOTOOLS_HXX

#include <vcl/idle.hxx>
#include <vcl/weld.hxx>
#include <vcl/customweld.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <swdllapi.h>

#define EX_SHOW_ONLINE_LAYOUT   0x01
// hard zoom value
#define EX_SHOW_BUSINESS_CARDS  0x02
//don't modify page size
#define EX_SHOW_DEFAULT_PAGE    0x04
//replace sample toc strings in the template to localized versions
#define EX_LOCALIZE_TOC_STRINGS 0x08

class SwView;

class SW_DLLPUBLIC SwOneExampleFrame : public weld::CustomWidgetController
{
    ScopedVclPtr<VirtualDevice> m_xVirDev;
    css::uno::Reference< css::frame::XModel >         m_xModel;
    css::uno::Reference< css::frame::XController >    m_xController;
    css::uno::Reference< css::text::XTextCursor >     m_xCursor;

    Idle            m_aLoadedIdle;
    Link<SwOneExampleFrame&,void> m_aInitializedLink;

    OUString        m_sArgumentURL;

    SwView*         m_pModuleView;

    sal_uInt32      m_nStyleFlags;

    bool            m_bIsInitialized;

    DECL_DLLPRIVATE_LINK( TimeoutHdl, Timer*, void );
    void PopupHdl(const OString& rId);

    SAL_DLLPRIVATE void  CreateControl();
    SAL_DLLPRIVATE void  DisposeControl();

public:
    SwOneExampleFrame(sal_uInt32 nStyleFlags,
                    const Link<SwOneExampleFrame&,void>* pInitializedLink,
                    const OUString* pURL = nullptr);
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual bool Command(const CommandEvent& rCEvt) override;
    virtual ~SwOneExampleFrame() override;

    css::uno::Reference< css::frame::XModel > &       GetModel()      {return m_xModel;}
    css::uno::Reference< css::text::XTextCursor > &   GetTextCursor() {return m_xCursor;}

    void ClearDocument();

    bool IsInitialized() const {return m_bIsInitialized;}

    bool CreatePopup(const Point& rPt);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
