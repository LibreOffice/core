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
#pragma once

#include <memory>
#include <sfx2/objsh.hxx>
#include <vcl/gdimtf.hxx>

class SvStream;

class Impl_OlePres
{
    SotClipboardFormatId nFormat;
    sal_uInt16      nAspect;
    std::unique_ptr<GDIMetaFile>
                    pMtf;

    sal_uInt32      nAdvFlags;
    Size            aSize;      // size in 100TH_MM
public:
    explicit Impl_OlePres()
        : nFormat( SotClipboardFormatId::GDIMETAFILE )
        , nAspect( ASPECT_CONTENT )
        , nAdvFlags( 0x2 )  // found in document
    {}
    void    SetMtf( const GDIMetaFile & rMtf )
            {
                pMtf.reset( new GDIMetaFile( rMtf ) );
            }
    void                 SetAspect( sal_uInt16 nAsp ) { nAspect = nAsp; }
    void                 SetAdviseFlags( sal_uInt32 nAdv ) { nAdvFlags = nAdv; }
    void                 SetSize( const Size & rSize ) { aSize = rSize; }
    void                 Write( SvStream & rStm );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
