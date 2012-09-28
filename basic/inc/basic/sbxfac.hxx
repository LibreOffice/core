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

#ifndef __SBX_SBX_FACTORY_HXX
#define __SBX_SBX_FACTORY_HXX

#include <basic/sbxdef.hxx>
#include <rtl/ustring.hxx>
#include "basicdllapi.h"

class SbxBase;
class SbxObject;

class BASIC_DLLPUBLIC SbxFactory
{
    bool bHandleLast;   // true: Factory is asked at last because of its expensiveness
public:
    virtual ~SbxFactory();
    SbxFactory( bool bLast=false )  { bHandleLast = bLast; }
    bool IsHandleLast( void )       { return bHandleLast; }
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const rtl::OUString& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
