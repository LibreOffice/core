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

#ifndef INCLUDED_SVTOOLS_IMAGEMGR_HXX
#define INCLUDED_SVTOOLS_IMAGEMGR_HXX

#include <rtl/ustring.hxx>
#include <svtools/svtdllapi.h>

enum class SvImageId {
    NONE                   =    0,
    START                  = 3076,  // must match to old Id's in SFX!

    Impress                = START + 47,
    Bitmap                 = START + 49,
    Calc                   = START + 50,
    CalcTemplate           = START + 51,
    Database               = START + 53,
    ImpressTemplate        = START + 54,
    GIF                    = START + 61,
    HTML                   = START + 63,
    JPG                    = START + 64,
    Math                   = START + 68,
    MathTemplate           = START + 69,
    File                   = START + 74,
    PCD                    = START + 76,
    PCT                    = START + 77,
    PCX                    = START + 78,
    SIM                    = START + 79,
    TextFile               = START + 80,
    TIFF                   = START + 82,
    WMF                    = START + 84,
    Writer                 = START + 86,
    WriterTemplate         = START + 87,
    FixedDevice            = START + 88,
    RemoveableDevice       = START + 89,
    CDRomDevice            = START + 90,
    NetworkDevice          = START + 91,
    Table                  = START + 112,
    Folder                 = START + 113,
    DXF                    = START + 141,
    MET                    = START + 142,
    PNG                    = START + 143,
    //retired SGF          = START + 144,
    //retired SGV          = START + 145,
    SVM                    = START + 146,
    GlobalDoc              = START + 150,
    Draw                   = START + 151,
    DrawTemplate           = START + 152,
    OO_DatabaseDoc         = START + 169,
    OO_DrawDoc             = START + 170,
    OO_MathDoc             = START + 171,
    OO_GlobalDoc           = START + 172,
    OO_ImpressDoc          = START + 173,
    OO_CalcDoc             = START + 174,
    OO_WriterDoc           = START + 175,
    OO_DrawTemplate        = START + 176,
    OO_ImpressTemplate     = START + 177,
    OO_CalcTemplate        = START + 178,
    OO_WriterTemplate      = START + 179,
    Extension              = START + 180,
};


class Image;
class INetURLObject;

namespace svtools {

struct VolumeInfo
{
    bool    m_bIsVolume;
    bool    m_bIsRemote;
    bool    m_bIsRemoveable;
    bool    m_bIsFloppy;
    bool    m_bIsCompactDisc;

    VolumeInfo() :
        m_bIsVolume     ( false ),
        m_bIsRemote     ( false ),
        m_bIsRemoveable ( false ),
        m_bIsFloppy     ( false ),
        m_bIsCompactDisc( false ) {}

    VolumeInfo( bool _bIsVolume,
                bool _bIsRemote,
                bool _bIsRemoveable,
                bool _bIsFloppy,
                bool _bIsCompactDisc ) :
        m_bIsVolume     ( _bIsVolume ),
        m_bIsRemote     ( _bIsRemote ),
        m_bIsRemoveable ( _bIsRemoveable ),
        m_bIsFloppy     ( _bIsFloppy ),
        m_bIsCompactDisc( _bIsCompactDisc ) {}
};

}

class SvFileInformationManager
{
private:
    static OUString    GetDescription_Impl( const INetURLObject& rObject, bool bDetectFolder );

public:
    SVT_DLLPUBLIC static OUString GetImageId( const INetURLObject& rURL, bool bBig = false );
    SVT_DLLPUBLIC static Image  GetImage( const INetURLObject& rURL, bool bBig = false );
    SVT_DLLPUBLIC static OUString GetFileImageId( const INetURLObject& rURL );
    SVT_DLLPUBLIC static Image  GetFileImage( const INetURLObject& rURL );
    SVT_DLLPUBLIC static Image  GetImageNoDefault( const INetURLObject& rURL, bool bBig = false );
    SVT_DLLPUBLIC static Image  GetFolderImage( const svtools::VolumeInfo& rInfo );

    SVT_DLLPUBLIC static OUString GetDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static OUString GetFileDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static OUString GetFolderDescription( const svtools::VolumeInfo& rInfo );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
