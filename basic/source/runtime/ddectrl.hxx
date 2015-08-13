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

#ifndef INCLUDED_BASIC_SOURCE_RUNTIME_DDECTRL_HXX
#define INCLUDED_BASIC_SOURCE_RUNTIME_DDECTRL_HXX

#include <tools/link.hxx>
#include <basic/sberrors.hxx>

class DdeConnection;
class DdeData;

class SbiDdeControl
{
private:
    DECL_LINK_TYPED( Data, const DdeData*, void );
    static SbError GetLastErr( DdeConnection* );
    size_t GetFreeChannel();
    std::vector<DdeConnection*> aConvList;
    OUString aData;

public:

    SbiDdeControl();
    ~SbiDdeControl();

    SbError Initiate( const OUString& rService, const OUString& rTopic,
                     size_t& rnHandle );
    SbError Terminate( size_t nChannel );
    SbError TerminateAll();
    SbError Request( size_t nChannel, const OUString& rItem, OUString& rResult );
    SbError Execute( size_t nChannel, const OUString& rCommand );
    SbError Poke( size_t nChannel, const OUString& rItem, const OUString& rData );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
