/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GeometryHandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:38:38 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPT_GEOMETRYHANDLER_HXX
#include "GeometryHandler.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#include "reportformula.hxx"
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_STRINGREPRESENTATION_HPP_
#include "com/sun/star/inspection/StringRepresentation.hpp"
#endif
#ifndef _COM_SUN_STAR_INSPECTION_PROPERTYLINEELEMENT_HPP_
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_PROPERTYCONTROLTYPE_HPP_
#include <com/sun/star/inspection/PropertyControlType.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XSTRINGLISTCONTROL_HPP_
#include <com/sun/star/inspection/XStringListControl.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#include <com/sun/star/report/XShape.hpp>
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XFORMATTEDFIELD_HPP_
#include <com/sun/star/report/XFormattedField.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XNUMERICCONTROL_HPP_
#include <com/sun/star/inspection/XNumericControl.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_MEASUREUNIT_HPP_
#include <com/sun/star/util/MeasureUnit.hpp>
#endif
#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#include "UITools.hxx"
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#include <com/sun/star/report/XFixedLine.hpp>
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#include <boost/bind.hpp>
#include <tools/string.hxx>
#ifndef RPTUI_METADATA_HXX_
#include "metadata.hxx"
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen wg. SfxItemSet
#include <svtools/itemset.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST
#include <svx/xdef.hxx>
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include <svx/xoutx.hxx>
#ifndef _SVX_XLNWTIT_HXX
#include <svx/xlnwtit.hxx>
#endif
#ifndef _SVX_XLNTRIT_HXX
#include <svx/xlntrit.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <svx/xlnclit.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX
#include <svx/xlnstit.hxx>
#endif
#ifndef _SVX_XLNEDIT_HXX
#include <svx/xlnedit.hxx>
#endif
#ifndef _SVX_XLNSTWIT_HXX
#include <svx/xlnstwit.hxx>
#endif
#ifndef _SVX_XLNEDWIT_HXX
#include <svx/xlnedwit.hxx>
#endif
#ifndef _SVX_XLNSTCIT_HXX
#include <svx/xlnstcit.hxx>
#endif
#ifndef _SVX_XLNEDCIT_HXX
#include <svx/xlnedcit.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX
#include <svx/xlineit0.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST
#ifndef _SVX_DRAWITEM_HXX
#include <svx/drawitem.hxx>
#endif
#define ITEMID_BRUSH            SID_ATTR_BRUSH
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef RPTUI_DLGPAGE_HXX
#include "dlgpage.hxx"
#endif
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#include <toolkit/helper/convert.hxx>

//........................................................................
namespace rptui
{
//........................................................................
using namespace ::com::sun::star;
// using namespace comphelper;

// comparing two property instances
struct PropertyCompare : public ::std::binary_function< beans::Property, ::rtl::OUString , bool >
{
    bool operator() (const beans::Property& x, const ::rtl::OUString& y) const
    {
        return x.Name.equals(y);// ? true : false;
    }
    bool operator() (const ::rtl::OUString& x,const beans::Property& y) const
    {
        return x.equals(y.Name);// ? true : false;
    }
};
void lcl_collectFunctionNames(const uno::Reference< report::XFunctions>& _xFunctions,::std::vector< ::rtl::OUString >& _rFunctionNames)
{
    const static ::rtl::OUString sLeftBracket(RTL_CONSTASCII_USTRINGPARAM("["));
    const static ::rtl::OUString sRightBracket(RTL_CONSTASCII_USTRINGPARAM("]"));

    const sal_Int32 nCount = _xFunctions->getCount();
    for (sal_Int32 i = 0; i < nCount ; ++i)
    {
        uno::Reference< report::XFunction > xFunction(_xFunctions->getByIndex(i),uno::UNO_QUERY);
        ::rtl::OUString sName = sLeftBracket;
        sName += xFunction->getName();
        sName += sRightBracket;
        _rFunctionNames.push_back(sName);
    }
}
void lcl_collectFunctionNames(const uno::Reference< report::XSection>& _xSection,::std::vector< ::rtl::OUString >& _rFunctionNames)
{
    uno::Reference< report::XReportDefinition> xReportDefinition = _xSection->getReportDefinition();
    uno::Reference< report::XGroups> xGroups = xReportDefinition->getGroups();
    sal_Int32 nPos = -1;
    uno::Reference< report::XGroup> xGroup = _xSection->getGroup();
    if ( xGroup.is() )
        nPos = getPositionInIndexAccess(xGroups.get(),xGroup);
    else if ( _xSection == xReportDefinition->getDetail() )
        nPos = xGroups->getCount()-1;

    for (sal_Int32 i = 0 ; i <= nPos ; ++i)
    {
        xGroup.set(xGroups->getByIndex(i),uno::UNO_QUERY);
        lcl_collectFunctionNames(xGroup->getFunctions(),_rFunctionNames);
    }
    lcl_collectFunctionNames(xReportDefinition->getFunctions(),_rFunctionNames);
}
// -----------------------------------------------------------------------------
void lcl_convertFormulaTo(const uno::Any& _aPropertyValue,uno::Any& _rControlValue)
{
    ::rtl::OUString sName;
    _aPropertyValue >>= sName;
    sal_Int32 nLen = sName.getLength();
    if ( nLen )
    {
        ReportFormula aFormula( sName );
        _rControlValue <<= aFormula.getUndecoratedContent();
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString GeometryHandler::impl_convertToFormula( const uno::Any& _rControlValue )
{
    ::rtl::OUString sName;
    _rControlValue >>= sName;

    if ( !sName.getLength() )
        return sName;

    ReportFormula aParser( sName );
    if ( aParser.isValid() )
        return sName;

    const ::rtl::OUString* pEnd = m_aFieldNames.getConstArray() + m_aFieldNames.getLength();
    bool bIsField = ( ::std::find( m_aFieldNames.getConstArray(), pEnd, sName ) != pEnd );

    pEnd = m_aParamNames.getConstArray() + m_aParamNames.getLength();
    bool bIsParam = ( ::std::find( m_aParamNames.getConstArray(), pEnd, sName ) != pEnd );

    aParser = ReportFormula( bIsField || bIsParam ? ReportFormula::Field : ReportFormula::Expression, sName );
    return aParser.getCompleteFormula();
}

GeometryHandler::GeometryHandler(uno::Reference< uno::XComponentContext > const & context) :
    GeometryHandler_Base(m_aMutex),
    m_xContext(context),
    m_pInfoService(new OPropertyInfoService())
{
    try
    {
        uno::Reference< lang::XMultiComponentFactory > xFac = m_xContext->getServiceManager();
        m_xFormComponentHandler.set(xFac->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.inspection.FormComponentPropertyHandler")),m_xContext),uno::UNO_QUERY_THROW);
        m_xTypeConverter.set(xFac->createInstanceWithContext( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter" )),m_xContext),uno::UNO_QUERY_THROW);
    }
    catch(const uno::Exception&)
    {
    }
}

//------------------------------------------------------------------------
::rtl::OUString SAL_CALL GeometryHandler::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL GeometryHandler::supportsService( const ::rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL GeometryHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

//------------------------------------------------------------------------
::rtl::OUString GeometryHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.GeometryHandler"));
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > GeometryHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.inspection.GeometryHandler"));
    return aSupported;
}

//------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL GeometryHandler::create( const uno::Reference< uno::XComponentContext >& _rxContext )
{
    return *(new GeometryHandler( _rxContext ));
}
// overload WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL GeometryHandler::disposing()
{
    ::comphelper::disposeComponent(m_xFormComponentHandler);
}
void SAL_CALL GeometryHandler::addEventListener(const uno::Reference< lang::XEventListener > & xListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->addEventListener(xListener);
}

void SAL_CALL GeometryHandler::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removeEventListener(aListener);
}

// inspection::XPropertyHandler:

/********************************************************************************/
void SAL_CALL GeometryHandler::inspect( const uno::Reference< uno::XInterface > & _rxInspectee ) throw (uno::RuntimeException, lang::NullPointerException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    try
    {
        uno::Reference< container::XNameContainer > xObjectAsContainer( _rxInspectee, uno::UNO_QUERY );
        m_xReportComponent.set( xObjectAsContainer->getByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ReportComponent" ) ) ), uno::UNO_QUERY );

        const ::rtl::OUString sRowSet(RTL_CONSTASCII_USTRINGPARAM("RowSet"));
        if ( xObjectAsContainer->hasByName( sRowSet ) )
        {
            uno::Any aRowSet = xObjectAsContainer->getByName(sRowSet);
            aRowSet >>= m_xRowSet;
            // forward the rowset to our delegator handler
            uno::Reference< beans::XPropertySet > xProp( m_xFormComponentHandler,uno::UNO_QUERY );
            xProp->setPropertyValue( sRowSet, aRowSet );
        }
        uno::Reference< report::XReportComponent> xReportComponent( m_xReportComponent, uno::UNO_QUERY );
        uno::Reference< report::XSection> xSection( m_xReportComponent, uno::UNO_QUERY );
        if ( !xSection.is() && xReportComponent.is() )
            xSection = xReportComponent->getSection();
        if ( xSection.is() )
            lcl_collectFunctionNames( xSection, m_aFunctionNames );
    }
    catch(uno::Exception)
    {
        throw lang::NullPointerException();
    }
    m_xFormComponentHandler->inspect(m_xReportComponent);
}

uno::Any SAL_CALL GeometryHandler::getPropertyValue(const ::rtl::OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue = m_xReportComponent->getPropertyValue( PropertyName );
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CONDITIONALPRINTEXPRESSION:
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
        case PROPERTY_ID_DATAFIELD:
            lcl_convertFormulaTo(aPropertyValue,aPropertyValue);
            break;
        case PROPERTY_ID_BACKCOLOR:
        case PROPERTY_ID_CONTROLBACKGROUND:
            {
                sal_Int32 nColor = COL_TRANSPARENT;
                if ( (aPropertyValue >>= nColor) && static_cast<sal_Int32>(COL_TRANSPARENT) == nColor )
                    aPropertyValue.clear();
            }
            break;
        default:
            break;
    }
    return aPropertyValue;
}

void SAL_CALL GeometryHandler::setPropertyValue(const ::rtl::OUString & PropertyName, const uno::Any & Value) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aNewValue = Value;
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    bool bHandled = false;
    switch(nId)
    {
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
        case PROPERTY_ID_DATAFIELD:
            // aNewValue <<= impl_convertToFormula( aNewValue );
            break;
        case PROPERTY_ID_POSITIONX:
        case PROPERTY_ID_POSITIONY:
        case PROPERTY_ID_HEIGHT:
        case PROPERTY_ID_WIDTH:
            {
                uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY);
                if ( xSourceReportComponent.is() ) // check only report components
                {
                    sal_Int32 nNewValue = 0;
                    Value >>= nNewValue;
                    awt::Point aAwtPoint = xSourceReportComponent->getPosition();
                    awt::Size aAwtSize = xSourceReportComponent->getSize();
                    if ( nId == PROPERTY_ID_POSITIONX )
                        aAwtPoint.X = nNewValue;
                    else if ( nId == PROPERTY_ID_POSITIONY )
                        aAwtPoint.Y = nNewValue;
                    else if ( nId == PROPERTY_ID_HEIGHT )
                        aAwtSize.Height = nNewValue;
                    else if ( nId == PROPERTY_ID_WIDTH )
                        aAwtSize.Width = nNewValue;

                    checkPosAndSize(aAwtPoint,aAwtSize);
                }
            }
            break;
        case PROPERTY_ID_CHARFONTNAME:
            {
                uno::Reference< report::XReportControlFormat > xReportControlFormat( m_xReportComponent,uno::UNO_QUERY_THROW );
                uno::Sequence< beans::NamedValue > aFontSettings;
                OSL_VERIFY( Value >>= aFontSettings );
                applyCharacterSettings( xReportControlFormat, aFontSettings );
                bHandled = true;
            }
            break;
        default:
            break;
    }

    if ( !bHandled )
        m_xReportComponent->setPropertyValue(PropertyName, aNewValue);
}

// -----------------------------------------------------------------------------
beans::PropertyState SAL_CALL GeometryHandler::getPropertyState(const ::rtl::OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_xFormComponentHandler->getPropertyState(PropertyName);
}
// -----------------------------------------------------------------------------
void GeometryHandler::implCreateListLikeControl(
        const uno::Reference< inspection::XPropertyControlFactory >& _rxControlFactory
        ,inspection::LineDescriptor & out_Descriptor
        ,sal_uInt16 _nResId
        ,sal_Bool _bReadOnlyControl
        ,sal_Bool _bTrueIfListBoxFalseIfComboBox
    )
{
    const String aEntries = String( ModuleRes( _nResId ) );
    implCreateListLikeControl(_rxControlFactory,out_Descriptor,aEntries,_bReadOnlyControl,_bTrueIfListBoxFalseIfComboBox);
}
// -----------------------------------------------------------------------------
void GeometryHandler::implCreateListLikeControl(
        const uno::Reference< inspection::XPropertyControlFactory >& _rxControlFactory
        ,inspection::LineDescriptor & out_Descriptor
        ,const ::rtl::OUString& _sEntries
        ,sal_Bool _bReadOnlyControl
        ,sal_Bool _bTrueIfListBoxFalseIfComboBox
    )
{
    uno::Reference< inspection::XStringListControl > xListControl(
        _rxControlFactory->createPropertyControl(
            _bTrueIfListBoxFalseIfComboBox ? inspection::PropertyControlType::ListBox : inspection::PropertyControlType::ComboBox, _bReadOnlyControl
        ),
        uno::UNO_QUERY_THROW
    );

    out_Descriptor.Control = xListControl.get();
    const String sEntries(_sEntries);
    const xub_StrLen nCount = sEntries.GetTokenCount();
    for ( xub_StrLen i=0; i < nCount; ++i )
        xListControl->appendListEntry( sEntries.GetToken(i) );
}
// -----------------------------------------------------------------------------
void lcl_collectParamNames_nothrow( const uno::Reference< container::XIndexAccess >& _rxParams, ::std::vector< ::rtl::OUString >& _out_rParamNames )
{
    _out_rParamNames.resize( 0 );
    if ( !_rxParams.is() )
        return;

    try
    {
        sal_Int32 count( _rxParams->getCount() );
        _out_rParamNames.reserve( count );

        uno::Reference< beans::XPropertySet > xParam;
        ::rtl::OUString sParamName;
        for ( sal_Int32 i=0; i<count; ++i )
        {
            xParam.set( _rxParams->getByIndex(i), uno::UNO_QUERY_THROW );
            OSL_VERIFY( xParam->getPropertyValue( PROPERTY_NAME ) >>= sParamName );
            _out_rParamNames.push_back( sParamName );
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------

inspection::LineDescriptor SAL_CALL GeometryHandler::describePropertyLine(const ::rtl::OUString & PropertyName, const uno::Reference< inspection::XPropertyControlFactory > & _xControlFactory) throw (beans::UnknownPropertyException, lang::NullPointerException,uno::RuntimeException)
{
    inspection::LineDescriptor aOut;
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_FORCENEWPAGE:
        case PROPERTY_ID_NEWROWORCOL:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_FORCENEWPAGE_CONST,sal_False,sal_True);
            break;
        case PROPERTY_ID_GROUPKEEPTOGETHER:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_GROUPKEEPTOGETHER_CONST,sal_False,sal_True);
            break;
        case PROPERTY_ID_PAGEHEADEROPTION:
        case PROPERTY_ID_PAGEFOOTEROPTION:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_REPORTPRINTOPTION_CONST,sal_False,sal_True);
            break;

        case PROPERTY_ID_VISIBLE:
        case PROPERTY_ID_CANGROW:
        case PROPERTY_ID_CANSHRINK:
        case PROPERTY_ID_REPEATSECTION:
        case PROPERTY_ID_PRINTREPEATEDVALUES:
        case PROPERTY_ID_STARTNEWCOLUMN:
        case PROPERTY_ID_RESETPAGENUMBER:
        case PROPERTY_ID_PRINTWHENGROUPCHANGE:
        case PROPERTY_ID_KEEPTOGETHER:
        case PROPERTY_ID_DEEPTRAVERSING:
        case PROPERTY_ID_PREEVALUATED:
        case PROPERTY_ID_PRESERVEIRI:
        case PROPERTY_ID_BACKTRANSPARENT:
        case PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT:
            {
                sal_uInt16 nResId = RID_STR_BOOL;
                if ( PROPERTY_ID_KEEPTOGETHER == nId && uno::Reference< report::XGroup>(m_xReportComponent,uno::UNO_QUERY).is())
                    nResId = RID_STR_KEEPTOGETHER_CONST;
                implCreateListLikeControl(_xControlFactory,aOut,nResId,sal_False,sal_True);
            }
            break;
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
        case PROPERTY_ID_CONDITIONALPRINTEXPRESSION:
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::MultiLineTextField , sal_False);
            break;
        case PROPERTY_ID_DATAFIELD:
            {
                uno::Reference< inspection::XStringListControl > xListControl;
                aOut = m_xFormComponentHandler->describePropertyLine(PropertyName, _xControlFactory);

                xListControl.set( aOut.Control, uno::UNO_QUERY );
                if ( xListControl.is() )
                {
                    // the usual field names
                    m_aFieldNames = xListControl->getListEntries();

                    // add parameter names
                    m_aParamNames = getParameterNames( m_xRowSet );
                    ::std::for_each( m_aParamNames.getConstArray(), m_aParamNames.getConstArray() + m_aParamNames.getLength(),
                        ::boost::bind( &inspection::XStringListControl::appendListEntry, xListControl, _1 ) );

                    // add function names
                    ::std::for_each( m_aFunctionNames.begin(), m_aFunctionNames.end(),
                        ::boost::bind( &inspection::XStringListControl::appendListEntry, xListControl, _1 ) );
                }
            }
            break;
        case PROPERTY_ID_BACKCOLOR:
        case PROPERTY_ID_CONTROLBACKGROUND:
            aOut.Control = _xControlFactory->createPropertyControl( inspection::PropertyControlType::ColorListBox, sal_False );
            break;
        case PROPERTY_ID_CHARFONTNAME:
            aOut.PrimaryButtonId = UID_RPT_PROP_DLG_FONT_TYPE;
            aOut.Control = _xControlFactory->createPropertyControl( inspection::PropertyControlType::TextField, sal_True );
            aOut.HasPrimaryButton = sal_True;
            break;
        default:
            {
            aOut = m_xFormComponentHandler->describePropertyLine(PropertyName, _xControlFactory);
            }
    }

    if ( nId != -1 )
    {
        aOut.Category = ((m_pInfoService->getPropertyUIFlags(nId ) & PROP_FLAG_DATA_PROPERTY) != 0) ?
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Data"))
                                                        :
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("General"));
        aOut.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( nId ) );
        aOut.DisplayName = m_pInfoService->getPropertyTranslation(nId);
    }

    if  (  ( nId == PROPERTY_ID_POSITIONX )
        || ( nId == PROPERTY_ID_POSITIONY )
        || ( nId == PROPERTY_ID_WIDTH )
        || ( nId == PROPERTY_ID_HEIGHT )
        )
    {
        const MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
        const sal_Int16 nDisplayUnit = VCLUnoHelper::ConvertToMeasurementUnit( MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH, 1 );
        uno::Reference< inspection::XNumericControl > xNumericControl(aOut.Control,uno::UNO_QUERY);
        xNumericControl->setDecimalDigits( 2 );
        xNumericControl->setMinValue(beans::Optional<double>(sal_True,0.0));
        xNumericControl->setValueUnit( util::MeasureUnit::MM_100TH );
        if ( nDisplayUnit != -1 )
            xNumericControl->setDisplayUnit( nDisplayUnit );
        uno::Reference< report::XReportComponent> xComp(m_xReportComponent,uno::UNO_QUERY);
        if ( xComp.is() )
        {
            OSL_ENSURE(xComp->getSection().is(),"No Section!");
            uno::Reference< report::XReportDefinition > xReport = xComp->getSection()->getReportDefinition();
            OSL_ENSURE(xReport.is(),"Why is the report definition NULL!");
            if ( xReport.is() )
            {
                const awt::Size aSize = getStyleProperty<awt::Size>(xReport,PROPERTY_PAPERSIZE);
                const sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReport,PROPERTY_LEFTMARGIN);
                const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReport,PROPERTY_RIGHTMARGIN);
                switch(nId)
                {
                    case PROPERTY_ID_POSITIONX:
                    case PROPERTY_ID_POSITIONY:
                    case PROPERTY_ID_WIDTH:
                        xNumericControl->setMinValue(beans::Optional<double>(sal_True,0.0));
                        xNumericControl->setMaxValue(beans::Optional<double>(sal_True,double(aSize.Width - nLeftMargin - nRightMargin) / 1000.0));
                        if ( PROPERTY_ID_WIDTH == nId )
                        {
                            uno::Reference<report::XFixedLine> xFixedLine(m_xReportComponent,uno::UNO_QUERY);
                            if ( xFixedLine.is() && xFixedLine->getOrientation() == 1 ) // vertical
                                xNumericControl->setMinValue(beans::Optional<double>(sal_True,0.08 ));
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        else if ( PROPERTY_ID_HEIGHT == nId )
        {
            uno::Reference< report::XSection> xSection(m_xReportComponent,uno::UNO_QUERY);
            if ( xSection.is() )
            {
                sal_Int32 nHeight = 0;
                const sal_Int32 nCount = xSection->getCount();
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    uno::Reference<drawing::XShape> xShape(xSection->getByIndex(i),uno::UNO_QUERY);
                    nHeight = ::std::max<sal_Int32>(nHeight,xShape->getPosition().Y + xShape->getSize().Height);
                }
                xNumericControl->setMinValue(beans::Optional<double>(sal_True,nHeight / 1000.0 ));
            }
        }
    }
    return aOut;
}
// -----------------------------------------------------------------------------
beans::Property GeometryHandler::getProperty(const ::rtl::OUString & PropertyName)
{
    uno::Sequence< beans::Property > aProps = getSupportedProperties();
    const beans::Property* pIter = aProps.getConstArray();
    const beans::Property* pEnd  = pIter + aProps.getLength();
    const beans::Property* pFind = ::std::find_if(pIter,pEnd,::std::bind2nd(PropertyCompare(),boost::cref(PropertyName)));
    if ( pFind == pEnd )
        return beans::Property();
    return *pFind;
}
uno::Any GeometryHandler::getConstantValue(sal_Bool _bToControlValue,sal_uInt16 nResId,const uno::Any& _aValue,const ::rtl::OUString& _sConstantName,const ::rtl::OUString & PropertyName )
{
    const String sConstantValues = String(ModuleRes(nResId));

    const xub_StrLen nCount = sConstantValues.GetTokenCount();
    uno::Sequence< ::rtl::OUString > aSeq(nCount);
    for (xub_StrLen i = 0; i < nCount; ++i)
    {
        aSeq[i] = sConstantValues.GetToken(i);
    }
    uno::Reference< inspection::XStringRepresentation > xConversionHelper = inspection::StringRepresentation::createConstant( m_xContext,m_xTypeConverter
        ,_sConstantName,aSeq);
    if ( _bToControlValue )
    {
        return uno::makeAny( xConversionHelper->convertToControlValue( _aValue ) );
    }
    else
    {
        ::rtl::OUString sControlValue;
        _aValue >>= sControlValue;
        const beans::Property aProp = getProperty(PropertyName);
        return xConversionHelper->convertToPropertyValue( sControlValue, aProp.Type );
    }
}

uno::Any SAL_CALL GeometryHandler::convertToPropertyValue(const ::rtl::OUString & PropertyName, const uno::Any & _rControlValue) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue( _rControlValue );
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_FORCENEWPAGE:
        case PROPERTY_ID_NEWROWORCOL:
            aPropertyValue = getConstantValue(sal_False,RID_STR_FORCENEWPAGE_CONST,_rControlValue,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.ForceNewPage")),PropertyName);
            break;
        case PROPERTY_ID_GROUPKEEPTOGETHER:
            aPropertyValue = getConstantValue(sal_False,RID_STR_GROUPKEEPTOGETHER_CONST,_rControlValue,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.GroupKeepTogether")),PropertyName);
            break;
        case PROPERTY_ID_PAGEHEADEROPTION:
        case PROPERTY_ID_PAGEFOOTEROPTION:
            aPropertyValue = getConstantValue(sal_False,RID_STR_REPORTPRINTOPTION_CONST,_rControlValue,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.ReportPrintOption")),PropertyName);
            break;
        case PROPERTY_ID_BACKCOLOR:
        case PROPERTY_ID_CONTROLBACKGROUND:
            if ( !_rControlValue.hasValue() )
            {
                aPropertyValue <<= static_cast<sal_Int32>(COL_TRANSPARENT);
                break;
            }
            // run through

        case PROPERTY_ID_KEEPTOGETHER:
            if ( uno::Reference< report::XGroup>(m_xReportComponent,uno::UNO_QUERY).is())
            {
                aPropertyValue = getConstantValue(sal_False,RID_STR_KEEPTOGETHER_CONST,_rControlValue,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.KeepTogether")),PropertyName);
                break;
            }
            // run through

        case PROPERTY_ID_VISIBLE:
        case PROPERTY_ID_CANGROW:
        case PROPERTY_ID_CANSHRINK:
        case PROPERTY_ID_REPEATSECTION:
        case PROPERTY_ID_PRINTREPEATEDVALUES:
        case PROPERTY_ID_STARTNEWCOLUMN:
        case PROPERTY_ID_RESETPAGENUMBER:
        case PROPERTY_ID_PRINTWHENGROUPCHANGE:
        case PROPERTY_ID_DEEPTRAVERSING:
        case PROPERTY_ID_PREEVALUATED:
        case PROPERTY_ID_PRESERVEIRI:
        case PROPERTY_ID_BACKTRANSPARENT:
        case PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT:
        {
            if ( !aPropertyValue.hasValue() )
                // NULL is converted to NULL
                break;

            const beans::Property aProp = getProperty(PropertyName);
            if ( aPropertyValue.getValueType().equals( aProp.Type ) )
                // nothing to do, type is already as desired
                return aPropertyValue;

            if ( _rControlValue.getValueType().getTypeClass() == uno::TypeClass_STRING )
            {
                ::rtl::OUString sControlValue;
                _rControlValue >>= sControlValue;

                uno::Reference< inspection::XStringRepresentation > xConversionHelper = inspection::StringRepresentation::create( m_xContext,m_xTypeConverter );
                aPropertyValue = xConversionHelper->convertToPropertyValue( sControlValue, aProp.Type );
            }
            else
            {
                try
                {
                    aPropertyValue = m_xTypeConverter->convertTo( _rControlValue, aProp.Type );
                }
                catch( const uno::Exception& )
                {
                    OSL_ENSURE( sal_False, "GeometryHandler::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
                }
            }
            break;
        }
        case PROPERTY_ID_CONDITIONALPRINTEXPRESSION:
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
        case PROPERTY_ID_DATAFIELD:
            return uno::makeAny( impl_convertToFormula( _rControlValue ) );

        case PROPERTY_ID_POSITIONX:
            {
                aPropertyValue = m_xFormComponentHandler->convertToPropertyValue(PropertyName, _rControlValue);
                sal_Int32 nPosX = 0;
                aPropertyValue >>= nPosX;
                uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY);
                nPosX += getStyleProperty<sal_Int32>(xSourceReportComponent->getSection()->getReportDefinition(),PROPERTY_LEFTMARGIN);
                aPropertyValue <<= nPosX;
            }
            break;
        case PROPERTY_ID_CHARFONTNAME:
            return m_xFormComponentHandler->convertToPropertyValue(PROPERTY_FONTNAME, _rControlValue);
        default:
            return m_xFormComponentHandler->convertToPropertyValue(PropertyName, _rControlValue);
    }
    return aPropertyValue;
}

uno::Any SAL_CALL GeometryHandler::convertToControlValue(const ::rtl::OUString & PropertyName, const uno::Any & _rPropertyValue, const uno::Type & _rControlValueType) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    uno::Any aControlValue( _rPropertyValue );
    if ( !aControlValue.hasValue() )
        // NULL is converted to NULL
        return aControlValue;

    uno::Any aPropertyValue(_rPropertyValue);

    ::osl::MutexGuard aGuard( m_aMutex );
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_FORCENEWPAGE:
        case PROPERTY_ID_NEWROWORCOL:
            aControlValue = getConstantValue(sal_True,RID_STR_FORCENEWPAGE_CONST,aPropertyValue,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.ForceNewPage")),PropertyName);
            break;
        case PROPERTY_ID_GROUPKEEPTOGETHER:
            aControlValue = getConstantValue(sal_True,RID_STR_GROUPKEEPTOGETHER_CONST,aPropertyValue,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.GroupKeepTogether")),PropertyName);
            break;
        case PROPERTY_ID_PAGEHEADEROPTION:
        case PROPERTY_ID_PAGEFOOTEROPTION:
            aControlValue = getConstantValue(sal_True,RID_STR_REPORTPRINTOPTION_CONST,aPropertyValue,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.ReportPrintOption")),PropertyName);
            break;
        case PROPERTY_ID_KEEPTOGETHER:
            if ( uno::Reference< report::XGroup>(m_xReportComponent,uno::UNO_QUERY).is())
            {
                aControlValue = getConstantValue(sal_True,RID_STR_KEEPTOGETHER_CONST,aPropertyValue,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.KeepTogether")),PropertyName);
                break;
            }
            // run through
        case PROPERTY_ID_VISIBLE:
        case PROPERTY_ID_CANGROW:
        case PROPERTY_ID_CANSHRINK:
        case PROPERTY_ID_REPEATSECTION:
        case PROPERTY_ID_PRINTREPEATEDVALUES:
        case PROPERTY_ID_STARTNEWCOLUMN:
        case PROPERTY_ID_RESETPAGENUMBER:
        case PROPERTY_ID_PRINTWHENGROUPCHANGE:
        case PROPERTY_ID_DEEPTRAVERSING:
        case PROPERTY_ID_PREEVALUATED:
        case PROPERTY_ID_PRESERVEIRI:
        case PROPERTY_ID_BACKTRANSPARENT:
        case PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT:
        {
            if ( _rControlValueType.getTypeClass() == uno::TypeClass_STRING )
            {
                uno::Reference< inspection::XStringRepresentation > xConversionHelper = inspection::StringRepresentation::create( m_xContext,m_xTypeConverter );
                aControlValue <<= xConversionHelper->convertToControlValue( aPropertyValue );
            }
            else
            {
                try
                {
                    aControlValue = m_xTypeConverter->convertTo( aPropertyValue, _rControlValueType );
                }
                catch( const uno::Exception& )
                {
                    OSL_ENSURE( sal_False, "GeometryHandler::convertToControlValue: caught an exception while converting via TypeConverter!" );
                }
            }
            break;
        }
        case PROPERTY_ID_CONDITIONALPRINTEXPRESSION:
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
        case PROPERTY_ID_DATAFIELD:
            lcl_convertFormulaTo(aPropertyValue,aControlValue);
            break;
        case PROPERTY_ID_CHARFONTNAME:
            aControlValue = m_xFormComponentHandler->convertToControlValue(PROPERTY_FONTNAME, aPropertyValue, _rControlValueType);
            break;
        case PROPERTY_ID_POSITIONX:
            {
                sal_Int32 nPosX = 0;
                aPropertyValue >>= nPosX;
                uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY);
                nPosX -= getStyleProperty<sal_Int32>(xSourceReportComponent->getSection()->getReportDefinition(),PROPERTY_LEFTMARGIN);
                aPropertyValue <<= nPosX;
                aControlValue = m_xFormComponentHandler->convertToControlValue(PropertyName, aPropertyValue, _rControlValueType);
            }
            break;
        case PROPERTY_ID_BACKCOLOR:
        case PROPERTY_ID_CONTROLBACKGROUND:
            {
                sal_Int32 nColor = COL_TRANSPARENT;
                if ( (aPropertyValue >>= nColor) && static_cast<sal_Int32>(COL_TRANSPARENT) == nColor )
                    aPropertyValue.clear();
            }
            // run through
        default:
            aControlValue = m_xFormComponentHandler->convertToControlValue(PropertyName, aPropertyValue, _rControlValueType);
    }
    return aControlValue;
}
void SAL_CALL GeometryHandler::addPropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & Listener) throw (uno::RuntimeException, lang::NullPointerException)
{
    m_xFormComponentHandler->addPropertyChangeListener(Listener);
}

void SAL_CALL GeometryHandler::removePropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removePropertyChangeListener(_rxListener);
}
// -----------------------------------------------------------------------------
//--------------------------------------------------------------------------
uno::Sequence< beans::Property > SAL_CALL GeometryHandler::getSupportedProperties() throw (uno::RuntimeException)
{
    ::std::vector< beans::Property > aNewProps;
    aNewProps.reserve(20); // only a guess
    m_pInfoService->getExcludeProperties( aNewProps, m_xFormComponentHandler );

    const ::rtl::OUString pIncludeProperties[] =
    {
         PROPERTY_FORCENEWPAGE
//        ,PROPERTY_NEWROWORCOL
        ,PROPERTY_KEEPTOGETHER
        ,PROPERTY_CANGROW
        ,PROPERTY_CANSHRINK
        ,PROPERTY_REPEATSECTION
        ,PROPERTY_PRINTREPEATEDVALUES
        ,PROPERTY_CONDITIONALPRINTEXPRESSION
        ,PROPERTY_STARTNEWCOLUMN
        ,PROPERTY_RESETPAGENUMBER
        ,PROPERTY_PRINTWHENGROUPCHANGE
        ,PROPERTY_VISIBLE
//        ,PROPERTY_GROUPKEEPTOGETHER
        ,PROPERTY_PAGEHEADEROPTION
        ,PROPERTY_PAGEFOOTEROPTION
        ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlLabel"))
        ,PROPERTY_POSITIONX
        ,PROPERTY_POSITIONY
        ,PROPERTY_WIDTH
        ,PROPERTY_HEIGHT
        ,PROPERTY_PREEVALUATED
        ,PROPERTY_DEEPTRAVERSING
        ,PROPERTY_FORMULA
        ,PROPERTY_INITIALFORMULA
        ,PROPERTY_PRESERVEIRI
        ,PROPERTY_DATAFIELD
        ,PROPERTY_CHARFONTNAME
        ,PROPERTY_BACKCOLOR
        ,PROPERTY_BACKTRANSPARENT
        ,PROPERTY_CONTROLBACKGROUND
        ,PROPERTY_CONTROLBACKGROUNDTRANSPARENT
        ,PROPERTY_LABEL
    };
    uno::Reference < beans::XPropertySetInfo > xInfo = m_xReportComponent->getPropertySetInfo();
    uno::Sequence< beans::Property> aSeq = xInfo->getProperties();
    for (size_t i = 0; i < sizeof(pIncludeProperties)/sizeof(pIncludeProperties[0]) ;++i )
    {
        const beans::Property* pIter = aSeq.getConstArray();
        const beans::Property* pEnd  = pIter + aSeq.getLength();
        const beans::Property* pFind = ::std::find_if(pIter,pEnd,::std::bind2nd(PropertyCompare(),boost::cref(pIncludeProperties[i])));
        if ( pFind != pEnd )
        {
            aNewProps.push_back(*pFind);
        }
    }

    return uno::Sequence< beans::Property > (&(*aNewProps.begin()),aNewProps.size());
}

uno::Sequence< ::rtl::OUString > SAL_CALL GeometryHandler::getSupersededProperties() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet;
    uno::Reference<report::XReportDefinition> xReport(m_xReportComponent,uno::UNO_QUERY);
    if ( xReport.is() && !uno::Reference< report::XSection>(xReport->getParent(),uno::UNO_QUERY).is() )
    {
        aRet.realloc(5);
        ::rtl::OUString* pIter = aRet.getArray();
        *pIter++ = PROPERTY_POSITIONX;
        *pIter++ = PROPERTY_POSITIONY;
        *pIter++ = PROPERTY_WIDTH;
        *pIter++ = PROPERTY_HEIGHT;
        *pIter++ = PROPERTY_DATAFIELD;
    }
    return aRet;
}

uno::Sequence< ::rtl::OUString > SAL_CALL GeometryHandler::getActuatingProperties() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Sequence< ::rtl::OUString > aSeq(2);
    aSeq[0] = PROPERTY_BACKTRANSPARENT;
    aSeq[1] = PROPERTY_CONTROLBACKGROUNDTRANSPARENT;
    return ::comphelper::concatSequences(m_xFormComponentHandler->getActuatingProperties(),aSeq);
}

::sal_Bool SAL_CALL GeometryHandler::isComposable(const ::rtl::OUString & _rPropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_pInfoService->isComposable( _rPropertyName, m_xFormComponentHandler );
}

inspection::InteractiveSelectionResult SAL_CALL GeometryHandler::onInteractivePropertySelection(const ::rtl::OUString & PropertyName, ::sal_Bool Primary, uno::Any & _rData, const uno::Reference< inspection::XObjectInspectorUI > & _rxInspectorUI) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::NullPointerException)
{
    if ( !_rxInspectorUI.is() )
        throw lang::NullPointerException();
    if ( PropertyName.equalsAscii(PROPERTY_FILTER) )
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        inspection::InteractiveSelectionResult eResult = inspection::InteractiveSelectionResult_Cancelled;
        ::rtl::OUString sClause;
        if ( impl_dialogFilter_nothrow( sClause, aGuard ) )
        {
            _rData <<= sClause;
            eResult = inspection::InteractiveSelectionResult_ObtainedValue;
        }
        return eResult;
    }
    else if ( PropertyName.equalsAscii(PROPERTY_CHARFONTNAME) )
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        inspection::InteractiveSelectionResult eResult = inspection::InteractiveSelectionResult_Cancelled;
        uno::Reference< awt::XWindow> xInspectorWindow(m_xContext->getValueByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DialogParentWindow"))) ,uno::UNO_QUERY);
        uno::Reference< report::XReportControlFormat> xReportControlFormat(m_xReportComponent,uno::UNO_QUERY);
        aGuard.clear();

        uno::Sequence< beans::NamedValue > aFontSettings;
        if ( rptui::openCharDialog( xReportControlFormat, xInspectorWindow, aFontSettings ) )
        {
            _rData <<= aFontSettings;
            eResult = inspection::InteractiveSelectionResult_ObtainedValue;
        }
        return eResult;
    }

    return m_xFormComponentHandler->onInteractivePropertySelection(PropertyName, Primary, _rData, _rxInspectorUI);
}

void SAL_CALL GeometryHandler::actuatingPropertyChanged(const ::rtl::OUString & ActuatingPropertyName, const uno::Any & NewValue, const uno::Any & OldValue, const uno::Reference< inspection::XObjectInspectorUI > & _rxInspectorUI, ::sal_Bool FirstTimeInit) throw (uno::RuntimeException, lang::NullPointerException)
{
    if ( !_rxInspectorUI.is() )
        throw lang::NullPointerException();

    ::osl::MutexGuard aGuard( m_aMutex );
    const sal_Int32 nId = m_pInfoService->getPropertyId(ActuatingPropertyName);
    switch(nId)
    {
        case PROPERTY_ID_BACKTRANSPARENT:
        case PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT:
            {
                sal_Bool bValue = sal_False;
                NewValue >>= bValue;
                bValue = !bValue;
                _rxInspectorUI->enablePropertyUI(PROPERTY_BACKCOLOR,bValue);
                _rxInspectorUI->enablePropertyUI(PROPERTY_CONTROLBACKGROUND,bValue);
                if ( bValue )
                    try
                    {
                        const ::rtl::OUString pProps[] ={PROPERTY_BACKCOLOR,PROPERTY_CONTROLBACKGROUND};
                        for (size_t i = 0; i < sizeof(pProps)/sizeof(pProps[0]); ++i)
                        {
                            if ( m_xReportComponent->getPropertySetInfo()->hasPropertyByName(pProps[i]) )
                            {
                                sal_Int32 nColor = COL_TRANSPARENT;
                                m_xReportComponent->getPropertyValue(pProps[i]) >>= nColor;
                                if ( COL_TRANSPARENT == static_cast<sal_uInt32>(nColor) )
                                    m_xReportComponent->setPropertyValue(pProps[i],uno::makeAny(COL_WHITE));
                                break;
                            }
                        }
                    }
                    catch(const uno::Exception&)
                    {
                        // not interested in
                    }
            }
            break;
        default:
            m_xFormComponentHandler->actuatingPropertyChanged(ActuatingPropertyName, NewValue, OldValue, _rxInspectorUI, FirstTimeInit);
            break;
    }
}

::sal_Bool SAL_CALL GeometryHandler::suspend(::sal_Bool Suspend) throw (uno::RuntimeException)
{
    return m_xFormComponentHandler->suspend(Suspend);
}
bool GeometryHandler::impl_dialogFilter_nothrow( ::rtl::OUString& _out_rSelectedClause, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
{
    _out_rSelectedClause = ::rtl::OUString();
    bool bSuccess = false;
    ::dbtools::SQLExceptionInfo aErrorInfo;
    uno::Reference< awt::XWindow > xInspectorWindow;
    uno::Reference< lang::XMultiComponentFactory > xFactory;
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory;
    try
    {
        xFactory = m_xContext->getServiceManager();
        xServiceFactory.set(xFactory,uno::UNO_QUERY);
        xInspectorWindow.set(m_xContext->getValueByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DialogParentWindow"))) ,uno::UNO_QUERY);
        uno::Reference<sdbc::XConnection> xCon(m_xContext->getValueByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ActiveConnection"))) ,uno::UNO_QUERY);
        if ( !xCon.is() )
            return false;

        uno::Reference< beans::XPropertySet> xRowSetProp(m_xRowSet,uno::UNO_QUERY);
        if ( !m_xRowSet.is() )
        {
            m_xRowSet.set(xFactory->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.RowSet")),m_xContext),uno::UNO_QUERY);
            xRowSetProp.set(m_xRowSet,uno::UNO_QUERY);
            xRowSetProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,uno::makeAny(xCon));
            ::comphelper::copyProperties(m_xReportComponent,xRowSetProp);
        }

        // get a composer for the statement which the form is currently based on
        uno::Reference< sdb::XSingleSelectQueryComposer > xComposer( ::dbtools::getCurrentSettingsComposer( xRowSetProp, xServiceFactory ) );
        OSL_ENSURE( xComposer.is(), "GeometryHandler::impl_dialogFilter_nothrow: could not obtain a composer!" );
        if ( !xComposer.is() )
            return false;

        // create the dialog
        uno::Reference< ui::dialogs::XExecutableDialog > xDialog(xFactory->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.FilterDialog")),m_xContext),uno::UNO_QUERY);
        if ( !xDialog.is() )
        {
            Window* pInspectorWindow = VCLUnoHelper::GetWindow( xInspectorWindow );
            ShowServiceNotAvailableError( pInspectorWindow, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.FilterDialog")), sal_True );
            return false;
        }

        const ::rtl::OUString sPropertyUIName( String(ModuleRes(RID_STR_FILTER)) );
        // initialize the dialog
        uno::Reference< beans::XPropertySet > xDialogProps( xDialog, uno::UNO_QUERY_THROW );
        xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "QueryComposer" ) ), uno::makeAny( xComposer ) );
        xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RowSet" ) ),        uno::makeAny( m_xRowSet ) );
        xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ) ),  uno::makeAny( xInspectorWindow ) );
        xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),         uno::makeAny( sPropertyUIName ) );

        _rClearBeforeDialog.clear();
        bSuccess = ( xDialog->execute() != 0 );
        if ( bSuccess )
            _out_rSelectedClause = xComposer->getFilter();
    }
    catch (sdb::SQLContext& e) { aErrorInfo = e; }
    catch (sdbc::SQLWarning& e) { aErrorInfo = e; }
    catch (sdbc::SQLException& e) { aErrorInfo = e; }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( sal_False, "GeometryHandler::impl_dialogFilter_nothrow: caught an exception!" );
    }

    if ( aErrorInfo.isValid() )
        ::dbtools::showError( aErrorInfo, xInspectorWindow, xServiceFactory );

    return bSuccess;
}
// -----------------------------------------------------------------------------
void GeometryHandler::checkPosAndSize(  const awt::Point& _aNewPos,
                                    const awt::Size& _aSize)
{
    uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY);
    uno::Reference< report::XSection> xSection(xSourceReportComponent->getParent(),uno::UNO_QUERY);
    if ( !xSection.is() || uno::Reference< report::XShape>(xSourceReportComponent,uno::UNO_QUERY).is() ) // shapes can overlap.
        return;

    ::Point aPos(VCLPoint(_aNewPos));
    if ( aPos.X() < 0 || aPos.Y() < 0 ) // TODO: have to check size with pos aka || (aPos.X() + aAwtSize.Width) > m_xSection->getReportDefinition()->
        throw beans::PropertyVetoException(String(ModuleRes(RID_STR_ILLEGAL_POSITION)),xSourceReportComponent);

    ::Rectangle aSourceRect(aPos,VCLSize(_aSize));

    sal_Int32 nCount = xSection->getCount();
    for (sal_Int32 i = 0; i < nCount ; ++i)
    {
        uno::Reference< report::XReportComponent> xReportComponent(xSection->getByIndex(i),uno::UNO_QUERY);
        if ( xReportComponent.is() && xReportComponent != xSourceReportComponent )
        {
            ::Rectangle aBoundRect(VCLPoint(xReportComponent->getPosition()),VCLSize(xReportComponent->getSize()));
            ::Rectangle aRect = aSourceRect.GetIntersection(aBoundRect);
            if ( !aRect.IsEmpty() && (aRect.Left() != aRect.Right() && aRect.Top() != aRect.Bottom() ) )
                throw beans::PropertyVetoException(String(ModuleRes( RID_STR_OVERLAP_OTHER_CONTROL)),xSourceReportComponent);
        }
    }
}
//........................................................................
} // namespace rptui
//........................................................................

