/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "ConfigurationTracer.hxx"

#include <cstdio>
#include "sal/log.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {

void ConfigurationTracer::TraceConfiguration (
    const Reference<XConfiguration>& rxConfiguration,
    const char* pMessage)
{
#ifdef DEBUG
    SAL_INFO("sd.ui","" << pMessage << " at " << rxConfiguration.get() << " {");
    if (rxConfiguration.is())
    {
        TraceBoundResources(rxConfiguration, NULL, 0);
    }
    else
    {
        SAL_INFO("sd.ui","    empty");
    }
    SAL_INFO("sd.ui","}");
#else
    (void)rxConfiguration;
    (void)pMessage;
#endif
}




#ifdef DEBUG
void ConfigurationTracer::TraceBoundResources (
    const Reference<XConfiguration>& rxConfiguration,
    const Reference<XResourceId>& rxResourceId,
    const int nIndentation)
{
    Sequence<Reference<XResourceId> > aResourceList (
        rxConfiguration->getResources(rxResourceId, OUString(), AnchorBindingMode_DIRECT));
    const OUString sIndentation ("    ");
    for (sal_Int32 nIndex=0; nIndex<aResourceList.getLength(); ++nIndex)
    {
        OUString sLine (aResourceList[nIndex]->getResourceURL());
        for (int i=0; i<nIndentation; ++i)
            sLine = sIndentation + sLine;
        SAL_INFO("sd.ui","" << OUStringToOString(sLine, RTL_TEXTENCODING_UTF8).getStr());
        TraceBoundResources(rxConfiguration, aResourceList[nIndex], nIndentation+1);
    }
}
#endif

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
