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

#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/util/XModeChangeListener.hpp>
#include <cppuhelper/implbase4.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdtypes.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/svxdllapi.h>

#include <vector>


// predeclarations

class Region;
class SdrUnoObj;
class SdrPageView;

// #110094#
namespace sdr
{
    namespace contact
    {
        class ObjectContact;
        class ViewObjectContactRedirector;
    } // end of namespace contact

    namespace overlay
    {
        class OverlayManager;
    } // end of namespace overlay
} // end of namespace sdr

namespace basegfx
{
    class B2DRange;
} // end of namespace basegfx

class SdrPaintWindow;
class Link;



class SVX_DLLPUBLIC SdrPageWindow
{
    // #110094# ObjectContact section
    sdr::contact::ObjectContact*                        mpObjectContact;

    // the SdrPageView this window belongs to
    SdrPageView&                                        mrPageView;

    // the PaintWindow to paint on. Here is access to OutDev etc.
    // #i72752# change to pointer to allow patcing it in DrawLayer() if necessary
    SdrPaintWindow*                                     mpPaintWindow;
    SdrPaintWindow*                                     mpOriginalPaintWindow;

    // UNO stuff for xControls
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > mxControlContainer;

    sdr::contact::ObjectContact* CreateViewSpecificObjectContact();

public:
    SdrPageWindow(SdrPageView& rNewPageView, SdrPaintWindow& rPaintWindow);
    ~SdrPageWindow();

    // data read accesses
    SdrPageView& GetPageView() const { return mrPageView; }
    SdrPaintWindow& GetPaintWindow() const { return *mpPaintWindow; }
    const SdrPaintWindow* GetOriginalPaintWindow() const { return mpOriginalPaintWindow; }
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > GetControlContainer( bool _bCreateIfNecessary = true ) const;

    // OVERLAYMANAGER
    rtl::Reference< ::sdr::overlay::OverlayManager > GetOverlayManager() const;

    // #i72752# allow patcing SdrPaintWindow from SdrPageView::DrawLayer if needed
    void patchPaintWindow(SdrPaintWindow& rPaintWindow);
    void unpatchPaintWindow();

    // the repaint method. For migration from pPaintProc, use one more parameter
    void PrePaint();
    void PrepareRedraw(const Region& rReg);
    void RedrawAll(sdr::contact::ViewObjectContactRedirector* pRedirector) const;
    void RedrawLayer(const SdrLayerID* pId, sdr::contact::ViewObjectContactRedirector* pRedirector) const;

    // Invalidate call, used from ObjectContact(OfPageView) in InvalidatePartOfView(...)
    void InvalidatePageWindow(const basegfx::B2DRange& rRange);

    // #110094# ObjectContact section
    sdr::contact::ObjectContact& GetObjectContact() const;
    /// determines whether there already exists an ObjectContact
    bool                         HasObjectContact() const;

    // #i26631#
    void ResetObjectContact();

    /** sets all elements in the view which support a design and a alive mode into the given mode
    */
    void    SetDesignMode( bool _bDesignMode ) const;
};

// typedefs for a list of SdrPageWindow
typedef ::std::vector< SdrPageWindow* > SdrPageWindowVector;



#endif // INCLUDED_SVX_SDRPAGEWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
