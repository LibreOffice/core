/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigColorScheme.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:14:05 $
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
#ifndef CHART2_CONFIG_COLOR_SCHEME_HXX
#define CHART2_CONFIG_COLOR_SCHEME_HXX

#include "ServiceMacros.hxx"
#include "ConfigItemListener.hxx"

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XCOLORSCHEME_HPP_
#include <com/sun/star/chart2/XColorScheme.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <memory>


namespace chart
{
namespace impl
{
class ChartConfigItem;
}

class ConfigColorScheme :
    public ConfigItemListener,
    public ::cppu::WeakImplHelper2<
        ::com::sun::star::chart2::XColorScheme,
        ::com::sun::star::lang::XServiceInfo >
{
public:
    explicit ConfigColorScheme(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~ConfigColorScheme();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( ConfigColorScheme )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XColorScheme ____
    virtual ::sal_Int32 SAL_CALL getColorByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ ConfigItemListener ____
    virtual void notify( const ::rtl::OUString & rPropertyName );

private:
    void retrieveConfigColors();

    // member variables
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >        m_xContext;
    ::std::auto_ptr< impl::ChartConfigItem >              m_apChartConfigItem;
    mutable ::com::sun::star::uno::Sequence< sal_Int64 >  m_aColorSequence;
    mutable sal_Int32                                     m_nNumberOfColors;
    bool                                                  m_bNeedsUpdate;
};

} // namespace chart

// CHART2_CONFIG_COLOR_SCHEME_HXX
#endif
