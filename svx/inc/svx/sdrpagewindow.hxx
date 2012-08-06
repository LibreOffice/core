/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef _SDRPAGEWINDOW_HXX
#define _SDRPAGEWINDOW_HXX

#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/util/XModeChangeListener.hpp>
#include <cppuhelper/implbase4.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdtypes.hxx> // for SdrLayerID
#include <tools/contnr.hxx>
#include <svx/sdrpagewindow.hxx>
#include "svx/svxdllapi.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////

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
    void*                                               mpDummy;
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDRPAGEWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
