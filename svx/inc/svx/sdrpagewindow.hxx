/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrpagewindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:11:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SDRPAGEWINDOW_HXX
#define _SDRPAGEWINDOW_HXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODECHANGELISTENER_HPP_
#include <com/sun/star/util/XModeChangeListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#ifndef _SVDTYPES_HXX
#include <svx/svdtypes.hxx> // fuer SdrLayerID
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _CONTNR_HXX
#include <tools/contnr.hxx>
#endif

#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predeclarations

class Region;
class SdrPaintInfoRec;
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

class SdrUnoControlList;
class SdrPaintWindow;
class Rectangle;
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

    // support method for creating PageInfoRecs for painting
    SdrPaintInfoRec* ImpCreateNewPageInfoRec(const Rectangle& rDirtyRect,
        sal_uInt16 nPaintMode, const SdrLayerID* pId) const;

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
    void PrepareRedraw(const Region& rReg);
    void RedrawAll(sal_uInt16 nPaintMode, ::sdr::contact::ViewObjectContactRedirector* pRedirector) const;
    void RedrawLayer(sal_uInt16 nPaintMode, const SdrLayerID* pId, ::sdr::contact::ViewObjectContactRedirector* pRedirector) const;

    // Invalidate call, used from ObjectContact(OfPageView) in InvalidatePartOfView(...)
    void Invalidate(const Rectangle& rRectangle);

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
