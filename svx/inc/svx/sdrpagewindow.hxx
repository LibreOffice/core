/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SDRPAGEWINDOW_HXX
#define _SDRPAGEWINDOW_HXX

#include <tools/debug.hxx>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/util/XModeChangeListener.hpp>
#include <cppuhelper/implbase4.hxx>
#include <svx/svdtypes.hxx> // fuer SdrLayerID
#include <svl/svarray.hxx>
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

class SdrUnoControlList;
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

    // #110094# ObjectContact section
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
    ::sdr::overlay::OverlayManager* GetOverlayManager() const;

    // #i72752# allow patcing SdrPaintWindow from SdrPageView::DrawLayer if needed
    void patchPaintWindow(SdrPaintWindow& rPaintWindow);
    void unpatchPaintWindow();

    // the repaint method. For migration from pPaintProc, use one more parameter
    void PrePaint();
    void PrepareRedraw(const Region& rReg);
    void RedrawAll(sdr::contact::ViewObjectContactRedirector* pRedirector) const;
    void RedrawLayer(const SdrLayerID* pId, sdr::contact::ViewObjectContactRedirector* pRedirector) const;
    void PostPaint();

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
    void SetDesignMode( bool _bDesignMode );
};

// typedefs for a list of SdrPageWindow
typedef ::std::vector< SdrPageWindow* > SdrPageWindowVector;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDRPAGEWINDOW_HXX
