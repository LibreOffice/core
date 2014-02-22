/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl_Module_Const.h>

#include <stdio.h>
#include <sal/types.h>




CPPUNIT_PLUGIN_IMPLEMENT();

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL firstfunc( sal_Bool bRes )
{
   return ( bRes = sal_True );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
