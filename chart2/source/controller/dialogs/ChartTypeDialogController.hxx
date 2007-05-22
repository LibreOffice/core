/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartTypeDialogController.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:27:03 $
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

#ifndef _CHART2_CHARTTYPE_DIALOG_CONTROLLER_HXX
#define _CHART2_CHARTTYPE_DIALOG_CONTROLLER_HXX

#include "ChangingResource.hxx"
#include "Pseudo3DHelper.hxx"

#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#include <comphelper/InlineContainer.hxx>
#endif

#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>

#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class FixedText
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class MetricField
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

enum GlobalStackMode
{
    GlobalStackMode_NONE,
    GlobalStackMode_STACK_Y,
    GlobalStackMode_STACK_Y_PERCENT,
    GlobalStackMode_STACK_Z
};

class ChartTypeParameter
{
public:
    ChartTypeParameter( sal_Int32 nSubTypeIndex, bool bXAxisWithValues=false
                    ,  bool b3DLook=false,  GlobalStackMode eStackMode=GlobalStackMode_NONE
                    ,  bool _bSymbols = true, bool _bLines = true
                    , ::com::sun::star::chart2::CurveStyle eCurveStyle = ::com::sun::star::chart2::CurveStyle_LINES );
    ChartTypeParameter();
    virtual ~ChartTypeParameter();

    bool mapsToSameService( const ChartTypeParameter& rParameter ) const;
    bool mapsToSimilarService( const ChartTypeParameter& rParameter, sal_Int32 nTheHigherTheLess ) const;

    sal_Int32       nSubTypeIndex;//starting with 1

    bool            bXAxisWithValues;
    bool            b3DLook;
    bool            bSymbols;
    bool            bLines;

    GlobalStackMode eStackMode;
    ::com::sun::star::chart2::CurveStyle      eCurveStyle;

    sal_Int32       nCurveResolution;
    sal_Int32       nSplineOrder;

    sal_Int32       nGeometry3D;

    ThreeDLookScheme                    eThreeDLookScheme;
    sal_Bool                            bSortByXValues;
};

typedef ::comphelper::MakeMap< ::rtl::OUString, ChartTypeParameter > tTemplateServiceChartTypeParameterMap;

class ChartTypeDialogController : public ChangingResource
{
public:
    ChartTypeDialogController();
    virtual ~ChartTypeDialogController();

    virtual String  getName()=0;
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const = 0;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );

    virtual bool    shouldShow_XAxisTypeControl() const;
    virtual bool    shouldShow_3DLookControl() const;
    virtual bool    shouldShow_StackingControl() const;
    virtual bool    shouldShow_DeepStackingControl() const;
    virtual bool    shouldShow_SplineControl() const;
    virtual bool    shouldShow_GeometryControl() const;
    virtual bool    shouldShow_SortByXValuesResourceGroup() const;

    virtual void    showExtraControls( Window* pParent, const Point& rPosition, const Size& rSize );
    virtual void    hideExtraControls() const;
    virtual void    fillExtraControls( const ChartTypeParameter& rParameter
                                     , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartModel
                                     , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xTemplateProps=::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >() ) const;
    virtual void    setTemplateProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xTemplateProps ) const throw (::com::sun::star::uno::RuntimeException);

    virtual bool                isSubType( const rtl::OUString& rServiceName );
    virtual ChartTypeParameter  getChartTypeParameterForService( const rtl::OUString& rServiceName, const ::com::sun::star::uno::Reference<
                                                                ::com::sun::star::beans::XPropertySet >& xTemplateProps );
    virtual void                adjustSubTypeAndEnableControls( ChartTypeParameter& rParameter );//if you have different counts of subtypes you may need to adjust the index
    virtual void                adjustParameterToSubType( ChartTypeParameter& rParameter );
    virtual void                adjustParameterToMainType( ChartTypeParameter& rParameter );
    virtual rtl::OUString       getServiceNameForParameter( const ChartTypeParameter& rParameter ) const;
    virtual bool                commitToModel( const ChartTypeParameter& rParameter
        , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartModel );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartTypeTemplate > getCurrentTemplate( const ChartTypeParameter& rParameter
                , const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xTemplateManager ) const;

protected:
    bool bSupportsXAxisWithValues;
    bool bSupports3D;
};

class ColumnOrBarChartDialogController_Base : public ChartTypeDialogController
{
public:
    ColumnOrBarChartDialogController_Base();
    virtual ~ColumnOrBarChartDialogController_Base();

    virtual bool    shouldShow_3DLookControl() const;
    virtual bool    shouldShow_GeometryControl() const;

    virtual void    adjustSubTypeAndEnableControls( ChartTypeParameter& rParameter );
};

class ColumnChartDialogController : public ColumnOrBarChartDialogController_Base
{
public:
    ColumnChartDialogController();
    virtual ~ColumnChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
};

class BarChartDialogController : public ColumnOrBarChartDialogController_Base
{
public:
    BarChartDialogController();
    virtual ~BarChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
};

class PieChartDialogController : public ChartTypeDialogController
{
public:
    PieChartDialogController();
    virtual ~PieChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
    virtual void adjustParameterToSubType( ChartTypeParameter& rParameter );

    virtual bool    shouldShow_3DLookControl() const;
};

class LineChartDialogController : public ChartTypeDialogController
{
public:
    LineChartDialogController();
    virtual ~LineChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
    virtual void adjustParameterToSubType( ChartTypeParameter& rParameter );
    virtual void adjustParameterToMainType( ChartTypeParameter& rParameter );

    virtual bool    shouldShow_StackingControl() const;
    virtual bool    shouldShow_DeepStackingControl() const;
    virtual bool    shouldShow_SplineControl() const;
};

class XYChartDialogController : public ChartTypeDialogController
{
public:
    XYChartDialogController();
    virtual ~XYChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
    virtual void adjustParameterToSubType( ChartTypeParameter& rParameter );

    virtual bool    shouldShow_SplineControl() const;
    virtual bool    shouldShow_SortByXValuesResourceGroup() const;
};

class AreaChartDialogController : public ChartTypeDialogController
{
public:
    AreaChartDialogController();
    virtual ~AreaChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
    virtual void adjustParameterToSubType( ChartTypeParameter& rParameter );
    virtual void adjustParameterToMainType( ChartTypeParameter& rParameter );

    virtual bool    shouldShow_3DLookControl() const;
};

class NetChartDialogController : public ChartTypeDialogController
{
public:
    NetChartDialogController();
    virtual ~NetChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
    virtual void adjustParameterToSubType( ChartTypeParameter& rParameter );

    virtual bool    shouldShow_StackingControl() const;
};

class StockChartDialogController : public ChartTypeDialogController
{
public:
    StockChartDialogController();
    virtual ~StockChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
    virtual void adjustParameterToSubType( ChartTypeParameter& rParameter );
};

class CombiColumnLineChartDialogController : public ChartTypeDialogController
{
public:
    CombiColumnLineChartDialogController();
    virtual ~CombiColumnLineChartDialogController();

    virtual String  getName();
    virtual Image   getImage( bool bIsHighContrast );
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const;
    virtual void fillSubTypeList( ValueSet& rSubTypeList, bool bIsHighContrast, const ChartTypeParameter& rParameter );
    virtual void adjustParameterToSubType( ChartTypeParameter& rParameter );

    virtual void    showExtraControls( Window* pParent, const Point& rPosition, const Size& rSize );
    virtual void    hideExtraControls() const;
    virtual void    fillExtraControls( const ChartTypeParameter& rParameter
                                     , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartModel
                                     , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xTemplateProps=::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >() ) const;

    virtual void    setTemplateProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xTemplateProps ) const throw (::com::sun::star::uno::RuntimeException);

private:
    DECL_LINK( ChangeLineCountHdl, void* );

private:
    FixedText*      m_pFT_NumberOfLines;
    MetricField*    m_pMF_NumberOfLines;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
