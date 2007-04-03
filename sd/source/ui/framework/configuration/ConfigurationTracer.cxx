/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationTracer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:46:18 $
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

#include "precompiled_sd.hxx"

#include "ConfigurationTracer.hxx"

#include <cstdio>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {

void ConfigurationTracer::TraceConfiguration (
    const Reference<XConfiguration>& rxConfiguration,
    const char* pMessage)
{
#ifdef DEBUG
    OSL_TRACE("%s at %p {", pMessage, rxConfiguration.get());
    if (rxConfiguration.is())
    {
        TraceBoundResources(rxConfiguration, NULL, 0);
    }
    else
    {
        OSL_TRACE("    empty");
    }
    OSL_TRACE("}");
#else
    (void)rxConfiguration;
    (void)pMessage;
#endif
}




void ConfigurationTracer::TraceBoundResources (
    const Reference<XConfiguration>& rxConfiguration,
    const Reference<XResourceId>& rxResourceId,
    const int nIndentation)
{
#ifdef DEBUG
    Sequence<Reference<XResourceId> > aResourceList (
        rxConfiguration->getResources(rxResourceId, ::rtl::OUString(), AnchorBindingMode_DIRECT));
    const ::rtl::OUString sIndentation (::rtl::OUString::createFromAscii("    "));
    for (sal_Int32 nIndex=0; nIndex<aResourceList.getLength(); ++nIndex)
    {
        ::rtl::OUString sLine (aResourceList[nIndex]->getResourceURL());
        for (int i=0; i<nIndentation; ++i)
            sLine = sIndentation + sLine;
        OSL_TRACE("%s", OUStringToOString(sLine, RTL_TEXTENCODING_UTF8).getStr());
        TraceBoundResources(rxConfiguration, aResourceList[nIndex], nIndentation+1);
    }
#else
    (void)rxConfiguration;
    (void)rxResourceId;
    (void)nIndentation;
#endif
}

} } // end of namespace sd::framework
