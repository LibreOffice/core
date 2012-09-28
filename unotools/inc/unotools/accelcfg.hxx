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
#ifndef INCLUDED_unotools_ACCELCFG_HXX
#define INCLUDED_unotools_ACCELCFG_HXX

#include <com/sun/star/awt/KeyEvent.hpp>
#include <unotools/options.hxx>

#include <rtl/ustring.hxx>
#include <tools/stream.hxx>

struct SvtAcceleratorConfigItem
{
    sal_uInt16      nCode;
    sal_uInt16      nModifier;
    OUString        aCommand;
};

#include <list>
typedef ::std::list < SvtAcceleratorConfigItem > SvtAcceleratorItemList;

class SvStream;
class KeyEvent;
class String;
class SvtAcceleratorConfig_Impl;

class SvtAcceleratorConfiguration: public utl::detail::Options
{
    SvtAcceleratorConfig_Impl*    pImp;

private:

public:
                    // get the global accelerators
                    SvtAcceleratorConfiguration();

                    // get special accelerators
    static String   GetStreamName();
    static SvStream* GetDefaultStream( StreamMode );

                    virtual ~SvtAcceleratorConfiguration();

                    // save the configuration to a stream, f.e. into a document
    bool            Commit( SvStream& rStream );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
