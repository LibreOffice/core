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
#ifndef _SDXMLWRP_HXX
#define _SDXMLWRP_HXX

#include <tools/errcode.hxx>
#include "sdfilter.hxx"

// ---------------
// - SdXMLFilter -
// ---------------

enum SdXMLFilterMode
{
    SDXMLMODE_Normal,   ///< standard load and save of the complete document
    SDXMLMODE_Preview,  ///< only for import, only the first draw page and its master page is loaded
    SDXMLMODE_Organizer ///< only for import, only the styles are loaded
};

class SdXMLFilter : public SdFilter
{
public:
    SdXMLFilter(
        SfxMedium& rMedium,
        ::sd::DrawDocShell& rDocShell,
        sal_Bool bShowProgress,
        SdXMLFilterMode eFilterMode = SDXMLMODE_Normal,
        sal_uLong nStoreVer = SOFFICE_FILEFORMAT_8 );
    virtual ~SdXMLFilter (void);

    sal_Bool        Import( ErrCode& nError );
    sal_Bool        Export();

private:
    SdXMLFilterMode meFilterMode;
    sal_uLong mnStoreVer;
};

#endif  // _SDXMLWRP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
