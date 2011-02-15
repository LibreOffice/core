/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SDXMLWRP_HXX
#define _SDXMLWRP_HXX

#include <tools/errcode.hxx>
#include "sdfilter.hxx"

// ---------------
// - SdXMLFilter -
// ---------------

enum SdXMLFilterMode
{
    SDXMLMODE_Normal,           // standard load and save of the complete document
    SDXMLMODE_Preview,          // only for import, only the first draw page and its master page is loaded
    SDXMLMODE_Organizer         // only for import, only the styles are loaded
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
