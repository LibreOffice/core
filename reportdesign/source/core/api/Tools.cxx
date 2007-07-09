/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Tools.cxx,v $
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
#ifndef REPORTDESIGN_TOOLS_HXX
#include "Tools.hxx"
#endif
#ifndef REPORTDESIGN_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef REPORTDESIGN_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

// =============================================================================
namespace reportdesign
{
// =============================================================================
using namespace com::sun::star;

uno::Reference< report::XSection> lcl_getSection(const uno::Reference< uno::XInterface>& _xReportComponent)
{
    uno::Reference< container::XChild> xChild(_xReportComponent,uno::UNO_QUERY);
    uno::Reference< report::XSection> xRet(_xReportComponent,uno::UNO_QUERY);
    while( !xRet.is() && xChild.is() )
    {
        uno::Reference< uno::XInterface> xTemp = xChild->getParent();
        xChild.set(xTemp,uno::UNO_QUERY);
        xRet.set(xTemp,uno::UNO_QUERY);
    }
    return xRet;
}
// -----------------------------------------------------------------------------
void throwIllegallArgumentException( const ::rtl::OUString& _sTypeName
                                    ,const uno::Reference< uno::XInterface >& ExceptionContext_
                                    ,const ::sal_Int16& ArgumentPosition_
                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context_
                                    )
{
    ::rtl::OUString sErrorMessage(RPT_RESSTRING(RID_STR_ERROR_WRONG_ARGUMENT,Context_->getServiceManager()));
    sErrorMessage = sErrorMessage.replaceAt(sErrorMessage.indexOf('#'),2,_sTypeName);
    throw lang::IllegalArgumentException(sErrorMessage,ExceptionContext_,ArgumentPosition_);
}
// -----------------------------------------------------------------------------
uno::Reference< util::XCloneable > cloneObject(const uno::Reference< report::XReportComponent>& _xReportComponent
    ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
    ,const ::rtl::OUString& _sServiceName)
{
    OSL_ENSURE(_xReportComponent.is() && _xFactory.is() ,"reportcomponent is null -> GPF");
    uno::Reference< report::XReportComponent> xClone(_xFactory->createInstance(_sServiceName),uno::UNO_QUERY_THROW);
    ::comphelper::copyProperties(_xReportComponent.get(),xClone.get());
    return xClone.get();
}
// -----------------------------------------------------------------------------

// =============================================================================
} // namespace reportdesign
// =============================================================================
