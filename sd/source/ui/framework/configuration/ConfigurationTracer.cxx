/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConfigurationTracer.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
    OSL_TRACE("%s at %p {\n", pMessage, rxConfiguration.get());
    if (rxConfiguration.is())
    {
        TraceBoundResources(rxConfiguration, NULL, 0);
    }
    else
    {
        OSL_TRACE("    empty\n");
    }
    OSL_TRACE("}\n");
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
        OSL_TRACE("%s\n", OUStringToOString(sLine, RTL_TEXTENCODING_UTF8).getStr());
        TraceBoundResources(rxConfiguration, aResourceList[nIndex], nIndentation+1);
    }
#else
    (void)rxConfiguration;
    (void)rxResourceId;
    (void)nIndentation;
#endif
}

} } // end of namespace sd::framework
