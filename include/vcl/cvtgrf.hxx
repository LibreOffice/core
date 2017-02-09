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

#ifndef INCLUDED_VCL_CVTGRF_HXX
#define INCLUDED_VCL_CVTGRF_HXX

#include <vcl/dllapi.h>
#include <tools/link.hxx>
#include <vcl/salctype.hxx>


struct  ConvertData;
class   Graphic;

class VCL_DLLPUBLIC GraphicConverter
{
private:

    Link<ConvertData&,bool>  maFilterHdl;

public:

                        GraphicConverter();
                        ~GraphicConverter();

    static ErrCode      Import( SvStream& rIStm, Graphic& rGraphic, ConvertDataFormat nFormat = ConvertDataFormat::Unknown );
    static ErrCode      Export( SvStream& rOStm, const Graphic& rGraphic, ConvertDataFormat nFormat );

    void                SetFilterHdl( const Link<ConvertData&,bool>& rLink ) { maFilterHdl = rLink; }
    const Link<ConvertData&,bool>&  GetFilterHdl() const { return maFilterHdl; }
};

#endif // INCLUDED_VCL_CVTGRF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
