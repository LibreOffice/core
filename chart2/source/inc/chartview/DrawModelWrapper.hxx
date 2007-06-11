/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawModelWrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-06-11 14:59:40 $
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
#ifndef _CHART2_DRAWMODELWRAPPER_HXX
#define _CHART2_DRAWMODELWRAPPER_HXX

//----
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
// header for class SdrObject
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

//----
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class DrawModelWrapper : private SdrModel
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory > m_xMCF;
    SfxItemPool* m_pChartItemPool;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > m_xMainDrawPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > m_xHiddenDrawPage;

    std::auto_ptr< OutputDevice > m_apRefDevice;

    //no default constructor
    DrawModelWrapper();

public:
    DrawModelWrapper(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~DrawModelWrapper();

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getShapeFactory();

    // the main page will contain the normal view objects
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > getMainDrawPage();
    void clearMainDrawPage();

    // the extra page is not visible, but contains some extras like the symbols for data points
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > getHiddenDrawPage();

    static ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
         getChartRootShape( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& xPage );


    void lockControllers();
    void unlockControllers();

    /// tries to get an OutputDevice from the XParent of the model to use as reference device
    void attachParentReferenceDevice(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel );

    OutputDevice* getReferenceDevice() const;

    SfxItemPool&            GetItemPool();
    const SfxItemPool&      GetItemPool() const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        createUnoModel();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        getUnoModel();
    SdrModel& getSdrModel();

    void updateTablesFromChartModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );

    XColorTable*    GetColorTable() const;
    XDashList*      GetDashList() const;
    XLineEndList*   GetLineEndList() const;
    XGradientList*  GetGradientList() const;
    XHatchList*     GetHatchList() const;
    XBitmapList*    GetBitmapList() const;

    SdrObject* getNamedSdrObject( const rtl::OUString& rName );
    static SdrObject* getNamedSdrObject( const String& rName, SdrObjList* pObjList );

    static bool removeShape( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >& xShape );
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
