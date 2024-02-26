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

#pragma once

#include <types.hxx>
#include <scdllapi.h>

#include <sfx2/viewsh.hxx>
#include <sfx2/zoomitem.hxx>
#include <svtools/scrolladaptor.hxx>
#include <vcl/syswin.hxx>

#include <shellids.hxx>

class ScDocument;
class ScDocShell;
class ScPreview;
struct ScHeaderFieldData;
class ScPreviewLocationData;
class CommandEvent;
class SfxViewFactory;

class SAL_DLLPUBLIC_RTTI ScPreviewShell final : public SfxViewShell
{
    ScDocShell*     pDocShell;

    VclPtr<SystemWindow>   mpFrameWindow;
    VclPtr<ScPreview>      pPreview;               // Output window
    VclPtr<ScrollAdaptor>  pHorScroll;
    VclPtr<ScrollAdaptor>  pVerScroll;

    TriState        nSourceDesignMode;      // form design mode from TabView
    SvxZoomType     eZoom;
    tools::Long     nMaxVertPos;
    tools::Long     nPrevHThumbPos;
    tools::Long     nPrevVThumbPos;

    std::unique_ptr<SfxBroadcaster> pAccessibilityBroadcaster;
    bool            GetPageSize( Size& aPageSize );
private:
    void            Construct( vcl::Window* pParent );
    DECL_LINK( HorzScrollHandler, weld::Scrollbar&, void );
    DECL_LINK( VertScrollHandler, weld::Scrollbar&, void );
    void ScrollHandler(ScrollAdaptor* pScrollBar);
    DECL_LINK( CloseHdl, SystemWindow&, void);
    void            DoScroll( sal_uInt16 nMode );
    void            ExitPreview();

    virtual void    Activate(bool bMDI) override;
    void            AdjustPosSizePixel( const Point &rPos, const Size &rSize );

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize, bool inplaceEditModeChange ) override;
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize ) override;

    virtual void    WriteUserData(OUString &, bool bBrowse = false) override;
    virtual void    ReadUserData(const OUString &, bool bBrowse = false) override;

    virtual void    WriteUserDataSequence (css::uno::Sequence < css::beans::PropertyValue >& ) override;
    virtual void    ReadUserDataSequence (const css::uno::Sequence < css::beans::PropertyValue >& ) override;

public:
                    SFX_DECL_INTERFACE(SCID_PREVIEW_SHELL)
                    SFX_DECL_VIEWFACTORY(ScPreviewShell);

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
                    ScPreviewShell(SfxViewFrame& rViewFrame, SfxViewShell* pOldSh);

    virtual         ~ScPreviewShell() override;

    void            InitStartTable(SCTAB nTab);

    void            UpdateScrollBars();
    void            UpdateNeededScrollBars(bool bFromZoom);
    bool            ScrollCommand( const CommandEvent& rCEvt );

    void            Execute( SfxRequest& rReq );
    void            GetState( SfxItemSet& rSet );

    void            FillFieldData( ScHeaderFieldData& rData );

    TriState        GetSourceDesignMode() const { return nSourceDesignMode; }

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual SfxPrinter*     GetPrinter( bool bCreate = false ) override;
    virtual sal_uInt16      SetPrinter( SfxPrinter* pNewPrinter, SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL ) override;
    virtual bool            HasPrintOptionsPage() const override;
    virtual std::unique_ptr<SfxTabPage> CreatePrintOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rOptions) override;

    void            AddAccessibilityObject( SfxListener& rObject );
    void            RemoveAccessibilityObject( SfxListener& rObject );
    void            BroadcastAccessibility( const SfxHint &rHint );
    bool            HasAccessibilityObjects() const;

    const ScPreviewLocationData& GetLocationData();
    ScDocument&     GetDocument();
    ScPreview*      GetPreview() { return pPreview; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
