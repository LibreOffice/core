/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdUnoDrawView.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:55:46 $
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

#ifndef SD_UNO_DRAW_VIEW_HXX
#define SD_UNO_DRAW_VIEW_HXX

#include "DrawSubController.hxx"
#include "DrawController.hxx"
#include "DrawViewShell.hxx"
#include <cppuhelper/basemutex.hxx>

class SdXImpressDocument;
class SdPage;

namespace css = ::com::sun::star;

namespace com { namespace sun { namespace star { namespace drawing {
class XLayer;
} } } }

namespace sd {

class DrawController;
class DrawViewShell;

/** This class implements the DrawViewShell specific part of the controller.
*/
class SdUnoDrawView
    : private cppu::BaseMutex,
      public DrawSubControllerInterfaceBase
{
public:
    SdUnoDrawView (
        DrawController& rController,
        DrawViewShell& rViewShell,
        View& rView) throw();
    virtual ~SdUnoDrawView (void) throw();

    // XSelectionSupplier

    virtual sal_Bool SAL_CALL select (
        const css::uno::Any& aSelection)
        throw(css::lang::IllegalArgumentException,
            css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getSelection (void)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL addSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL removeSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
        throw(css::uno::RuntimeException);


    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage >& xPage)
        throw(css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage (void)
        throw(css::uno::RuntimeException);


    // XFastPropertySet

    virtual void SAL_CALL setFastPropertyValue (
        sal_Int32 nHandle,
        const css::uno::Any& rValue)
        throw(css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getFastPropertyValue (
        sal_Int32 nHandle)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

protected:
    sal_Bool getMasterPageMode(void) const throw();
    void setMasterPageMode(sal_Bool MasterPageMode_) throw();
    sal_Bool getLayerMode(void) const throw();
    void setLayerMode(sal_Bool LayerMode_) throw();

    /** Return a reference to the active layer object.
        @return
            The returned value may be empty when the internal state of this
            view is not valid (like during destruction.)
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer> getActiveLayer (void) throw ();

    /** Make the specified object the active layer.
        @param rxLayer
            The new layer object.
    */
    void setActiveLayer (const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer>& rxLayer) throw ();

    void SetZoom( sal_Int16 nZoom );
    sal_Int16 GetZoom(void) const;

    void SetViewOffset(const com::sun::star::awt::Point& rWinPos );
    com::sun::star::awt::Point GetViewOffset() const;

    void SetZoomType( sal_Int16 nType );

private:
    DrawController& mrController;
    DrawViewShell& mrDrawViewShell;
    sd::View& mrView;

    SdXImpressDocument* GetModel (void) const throw();
};

} // end of namespace sd

#endif
