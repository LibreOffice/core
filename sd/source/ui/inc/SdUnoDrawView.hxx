#ifndef SD_UNO_DRAW_VIEW_HXX
#define SD_UNO_DRAW_VIEW_HXX

#ifndef SD_DRAW_CONTROLLER_HXX
#include "DrawController.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XLAYER_HPP_
#include <com/sun/star/drawing/XLayer.hpp>
#endif

#ifndef _SFX_SFXBASECONTROLLER_HXX_
#include <sfx2/sfxbasecontroller.hxx>
#endif

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#ifndef _CPPUHELPER_PROPTYPEHLP_HXX
#include <cppuhelper/proptypehlp.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

class SdXImpressDocument;
class SdPage;

namespace sd {

class DrawViewShell;

/**
 * This class implements the view component for a SdDrawViewShell
 */
class SdUnoDrawView
    : public DrawController
{
public:
    enum properties
    {
        PROPERTY__BEGIN = DrawController::PROPERTY__BEGIN,
        PROPERTY_CURRENTPAGE = PROPERTY__BEGIN,
        PROPERTY_MASTERPAGEMODE,
        PROPERTY_LAYERMODE,
        PROPERTY_ACTIVE_LAYER,
        PROPERTY_ZOOMTYPE,
        PROPERTY_ZOOMVALUE,
        PROPERTY_VIEWOFFSET,
        PROPERTY__END
    };

    SdUnoDrawView (
        ViewShellBase& rBase,
        ViewShell& rViewShell,
        View& rView) throw();
    virtual ~SdUnoDrawView (void) throw();

    virtual void FireChangeEditMode (bool bMasterPageMode) throw();
    virtual void FireChangeLayerMode (bool bLayerMode) throw();
    virtual void FireSwitchCurrentPage (SdPage* pCurrentPage) throw();

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException);

    // XDrawView
    virtual void SAL_CALL setCurrentPage( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getCurrentPage(  ) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual void FillPropertyTable (
        ::std::vector< ::com::sun::star::beans::Property>& rProperties);

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
    bool mbOldMasterPageMode;
    bool mbOldLayerMode;
    SdPage* mpCurrentPage;

    /** This is a shortcut for accessing the view shell data member of
        the base class casted to the correct class.
    */
    DrawViewShell& GetDrawViewShell (void) const;
};

} // end of namespace sd

#endif
