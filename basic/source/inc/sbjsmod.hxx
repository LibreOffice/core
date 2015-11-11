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

#ifndef INCLUDED_BASIC_SOURCE_INC_SBJSMOD_HXX
#define INCLUDED_BASIC_SOURCE_INC_SBJSMOD_HXX

#include <basic/sbmod.hxx>

// basic module for JavaScript sources
// All the basic-specific methods must be overridden virtually and must
// be deactivated. The differentiation of normal modules is done by RTTI.

class SbJScriptModule : public SbModule
{
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_JSCRIPTMOD,1);
    SbJScriptModule( const OUString& );       // hand through
};

#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
