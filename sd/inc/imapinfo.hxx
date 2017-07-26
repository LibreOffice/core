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

#ifndef INCLUDED_SD_INC_IMAPINFO_HXX
#define INCLUDED_SD_INC_IMAPINFO_HXX

#include "sdiocmpt.hxx"
#include "glob.hxx"
#include <svx/svdobj.hxx>
#include <svtools/imap.hxx>

class SdIMapInfo : public SdrObjUserData, public SfxListener
{

    ImageMap        aImageMap;

public:
                    SdIMapInfo( const ImageMap& rImageMap ) :
                        SdrObjUserData( SdrInventor::StarDrawUserData, SD_IMAPINFO_ID ),
                        aImageMap( rImageMap ) {};

                    SdIMapInfo( const SdIMapInfo& rIMapInfo ) :
                        SdrObjUserData( SdrInventor::StarDrawUserData, SD_IMAPINFO_ID ),
                        SfxListener(),
                        aImageMap( rIMapInfo.aImageMap ) {};

    virtual SdrObjUserData* Clone( SdrObject* ) const override { return new SdIMapInfo( *this ); }

    void            SetImageMap( const ImageMap& rIMap ) { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const { return aImageMap; }
};

#endif // INCLUDED_SD_INC_IMAPINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
