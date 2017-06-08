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

#ifndef INCLUDED_SD_INC_SDGRFFILTER_HXX
#define INCLUDED_SD_INC_SDGRFFILTER_HXX

#include <com/sun/star/drawing/XShape.hpp>

#include <vcl/errinf.hxx>
#include "sdfilter.hxx"

// SdCGMFilter
class SdGRFFilter : public SdFilter
{
public:
    SdGRFFilter ( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell );

    virtual ~SdGRFFilter() override;

    bool    Import();
    bool    Export() override;

    static void HandleGraphicFilterError( ErrCode nFilterError, ErrCode nStreamError );
};

#endif // INCLUDED_SD_INC_SDGRFFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
