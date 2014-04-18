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

#ifndef INCLUDED_SC_INC_STRLOAD_HXX
#define INCLUDED_SC_INC_STRLOAD_HXX

#include "scresid.hxx"
#include <tools/rc.hxx>

class ScRscStrLoader : public Resource
{
public:
    ScRscStrLoader( sal_uInt16 nRsc, sal_uInt16 nStrId ) :
        Resource( ScResId( nRsc ) ), theStr( ScResId( nStrId ) )
    {
        FreeResource();
    }

    const OUString& GetString() const { return theStr; }

private:
    OUString theStr;
};

#define SC_STRLOAD(resource,strid) ScRscStrLoader( resource, strid).GetString()

#endif // INCLUDED_SC_INC_STRLOAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
