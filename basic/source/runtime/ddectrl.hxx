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

#ifndef _DDECTRL_HXX
#define _DDECTRL_HXX

#include <tools/link.hxx>
#include <basic/sberrors.hxx>
#include <tools/string.hxx>

class DdeConnection;
class DdeData;

class SbiDdeControl
{
private:
    DECL_LINK( Data, DdeData* );
    SbError GetLastErr( DdeConnection* );
    sal_Int16 GetFreeChannel();
    std::vector<DdeConnection*> aConvList;
    String aData;

public:

    SbiDdeControl();
    ~SbiDdeControl();

    SbError Initiate( const String& rService, const String& rTopic,
                     sal_Int16& rnHandle );
    SbError Terminate( sal_uInt16 nChannel );
    SbError TerminateAll();
    SbError Request( sal_uInt16 nChannel, const String& rItem, String& rResult );
    SbError Execute( sal_uInt16 nChannel, const String& rCommand );
    SbError Poke( sal_uInt16 nChannel, const String& rItem, const String& rData );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
