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

#ifndef INCLUDED_SVX_SDRPAGEWINDOW_HXX
#define INCLUDED_SVX_SDRPAGEWINDOW_HXX

#include <basegfx/range/b2irectangle.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svxdllapi.h>
#include <memory>

namespace com::sun::star::awt { class XControlContainer; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }
namespace rtl { template <class reference_type> class Reference; }
namespace sdr::overlay { class OverlayManager; }
namespace vcl { class Region; }

namespace sdr::contact
{
    class ObjectContact;
    class ViewObjectContactRedirector;
}


namespace basegfx { class B2DRange; }

class SdrPaintWindow;
class SdrPageView;

class SVXCORE_DLLPUBLIC SdrPageWindow
{
    struct Impl;

    std::unique_ptr<Impl> mpImpl;

    SdrPageWindow( const SdrPageWindow& ) = delete;
    SdrPageWindow& operator= ( const SdrPageWindow& ) = delete;

public:
    SdrPageWindow(SdrPageView& rNewPageView, SdrPaintWindow& rPaintWindow);
    ~SdrPageWindow();

    // data read accesses
    SdrPageView& GetPageView() const;
    SdrPaintWindow& GetPaintWindow() const;
    const SdrPaintWindow* GetOriginalPaintWindow() const;
    css::uno::Reference<css::awt::XControlContainer> const & GetControlContainer( bool _bCreateIfNecessary = true ) const;

    // OVERLAYMANAGER
    rtl::Reference< sdr::overlay::OverlayManager > const & GetOverlayManager() const;

    // #i72752# allow patcing SdrPaintWindow from SdrPageView::DrawLayer if needed
    [[nodiscard]] SdrPaintWindow* patchPaintWindow(SdrPaintWindow& rPaintWindow);
    void unpatchPaintWindow(SdrPaintWindow* pPreviousPaintWindow);

    // the repaint method. For migration from pPaintProc, use one more parameter
    void PrePaint();
    void PrepareRedraw(const vcl::Region& rReg);
    void RedrawAll( sdr::contact::ViewObjectContactRedirector* pRedirector );
    void RedrawLayer( const SdrLayerID* pId, sdr::contact::ViewObjectContactRedirector* pRedirector, basegfx::B2IRectangle const*);

    // Invalidate call, used from ObjectContact(OfPageView) in InvalidatePartOfView(...)
    void InvalidatePageWindow(const basegfx::B2DRange& rRange);

    // #110094# ObjectContact section
    const sdr::contact::ObjectContact& GetObjectContact() const;
    sdr::contact::ObjectContact& GetObjectContact();
    /// determines whether there already exists an ObjectContact
    bool                         HasObjectContact() const;

    // #i26631#
    void ResetObjectContact();

    /** sets all elements in the view which support a design and an alive mode into the given mode
    */
    void    SetDesignMode( bool _bDesignMode ) const;
};

#endif // INCLUDED_SVX_SDRPAGEWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
