/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdUnoDrawView.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2006-04-28 14:58:51 $
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

#ifndef SD_DRAW_SUB_CONTROLLER_HXX
#include "DrawSubController.hxx"
#endif

class SdXImpressDocument;
class SdPage;

namespace com { namespace sun { namespace star { namespace drawing {
class XLayer;
} } } }

namespace sd {

class DrawController;
class DrawViewShell;

/** This class implements the DrawViewShell specific part of the controller.
*/
class SdUnoDrawView
    : public DrawSubController
{
public:
    SdUnoDrawView (
        DrawController& rController,
        DrawViewShell& rViewShell,
        View& rView) throw();
    virtual ~SdUnoDrawView (void) throw();

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException);

    // XDrawView
    virtual void SAL_CALL setCurrentPage( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getCurrentPage(  ) throw(::com::sun::star::uno::RuntimeException);

    /**
     * Converted the value rValue and return the result in rConvertedValue and the
     * old value in rOldValue. A IllegalArgumentException is thrown.
     * The method is not implemented in this class. After this call the vetoable
     * listeners are notified.
     *
     * @param rConvertedValue the converted value. Only set if return is true.
     * @param rOldValue the old value. Only set if return is true.
     * @param nHandle the handle of the proberty.
     * @return true if the value converted.
     */
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);
    /**
     * The same as setFastProperyValue, but no exception is thrown and nHandle
     * is always valid. You must not broadcast the changes in this method.<BR>
     * <B>You type is correct you need't test it.</B>
     */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);
    /**
     * The same as getFastProperyValue, but return the value through
     * rValue and nHandle is always valid.
     */
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const;

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
