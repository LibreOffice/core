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
#ifndef TOOLS_STRINGLISTRESOURCE_HXX
#define TOOLS_STRINGLISTRESOURCE_HXX

#include <vector>
#include <tools/resid.hxx>
#include <tools/rcid.h>
#include <tools/rc.hxx>

namespace tools
{
    class StringListResource : public Resource
    {
    public:
        StringListResource(const ResId& _aResId,::std::vector< ::rtl::OUString>& _rToFill ) : Resource(_aResId)
        {
            sal_uInt16 i = 1;
            while( IsAvailableRes(ResId(i,*m_pResMgr).SetRT(RSC_STRING)) )
            {
                String sStr = String(ResId(i,*m_pResMgr));
                _rToFill.push_back(sStr);
                ++i;
            }
        }
        ~StringListResource()
        {
            FreeResource();
        }
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
