/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportComponent.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:14 $
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
#include "ReportComponent.hxx"

#ifndef REPORTDESIGN_SHARED_CORESTRINGS_HRC
#include "corestrings.hrc"
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#ifndef RPT_REPORTCONTROLMODEL_HXX
#include "ReportControlModel.hxx"
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_PARAGRAPHVERTALIGN_HPP_
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#endif
#include <svx/unoshape.hxx>
#include <svtools/syslocale.hxx>
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
OFormatProperties::OFormatProperties()
    :nAlign(0)
    ,nFontEmphasisMark(0)
    ,nFontRelief(0)
    ,nTextColor(0)
    ,nTextLineColor(0)
    ,nCharUnderlineColor(0xFFFFFFFF)
    ,nBackgroundColor(COL_TRANSPARENT)
    ,aVerticalAlignment(text::ParagraphVertAlign::TOP)
    ,nCharEscapement(0)
    ,nCharCaseMap(0)
    ,nCharKerning(0)
    ,nCharEscapementHeight(100)
    ,m_bBackgroundTransparent(sal_True)
    ,bCharFlash(sal_False)
    ,bCharAutoKerning(sal_False)
    ,bCharCombineIsOn(sal_False)
    ,bCharHidden(sal_False)
    ,bCharShadowed(sal_False)
    ,bCharContoured(sal_False)
{
    Font aInitFont = Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont();
    aFontDescriptor = VCLUnoHelper::CreateFontDescriptor(aInitFont);
    aFontDescriptor.Weight = awt::FontWeight::NORMAL;
    aFontDescriptor.CharacterWidth = awt::FontWidth::NORMAL;
    aCharLocale = SvtSysLocale().GetLocaleData().getLocale();
    //if ( !aFontDescriptor.StyleName.getLength() )
    //    aFontDescriptor.StyleName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test"));
}
// -----------------------------------------------------------------------------
void OReportComponentProperties::setShape(uno::Reference< drawing::XShape >& _xShape,const uno::Reference< report::XReportComponent>& _xTunnel,oslInterlockedCount& _rRefCount)
{
    osl_incrementInterlockedCount( &_rRefCount );
    {
        // decrement the count from ReportDrawPage.cxx aArgs[0] <<= SvxDrawPage::_CreateShape( pObj );
        SvxShape* pShape = SvxShape::getImplementation( _xShape );
        if ( pShape )
            pShape->release();
        m_xProxy.set(_xShape,uno::UNO_QUERY);
        ::comphelper::query_aggregation(m_xProxy,m_xShape);
        ::comphelper::query_aggregation(m_xProxy,m_xProperty);
        _xShape.clear();
        m_xTypeProvider.set(m_xShape,uno::UNO_QUERY);
        m_xUnoTunnel.set(m_xShape,uno::UNO_QUERY);
        m_xServiceInfo.set(m_xShape,uno::UNO_QUERY);

        // set ourself as delegator
        if ( m_xProxy.is() )
            m_xProxy->setDelegator( _xTunnel );
    }
    // do not decrement the refcount again, this will be done from the any ReportDrawPage.cxx aArgs[0] <<= SvxDrawPage::_CreateShape( pObj ); , otherwise it will delete the object
    //osl_decrementInterlockedCount( &_rRefCount );
}
// -----------------------------------------------------------------------------
void OReportComponentProperties::dispose(oslInterlockedCount& _rRefCount)
{
    if ( m_xProxy.is() )
        osl_decrementInterlockedCount( &_rRefCount );
    //m_xShape.clear();
    //m_xTypeProvider.clear();
    //m_xUnoTunnel.clear();
    //m_xServiceInfo.clear();
    //m_xProperty.clear();
    m_xContext.clear();
    m_xFactory.clear();
}
// -----------------------------------------------------------------------------
OReportComponentProperties::~OReportComponentProperties()
{
    if ( m_xProxy.is() )
    {
        m_xProxy->setDelegator( NULL );
        m_xProxy.clear();
    }
}
// =============================================================================
} // namespace reportdesign
// =============================================================================


