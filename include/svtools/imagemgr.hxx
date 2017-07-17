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

#include <svtools/svtdllapi.h>
#include <sal/types.h>

#define IMAGELIST_START                 3076  // must match to old Id's in SFX!

#define IMG_IMPRESS                     (IMAGELIST_START + 47)
#define IMG_BITMAP                      (IMAGELIST_START + 49)
#define IMG_CALC                        (IMAGELIST_START + 50)
#define IMG_CALCTEMPLATE                (IMAGELIST_START + 51)
#define IMG_DATABASE                    (IMAGELIST_START + 53)
#define IMG_IMPRESSTEMPLATE             (IMAGELIST_START + 54)
#define IMG_GIF                         (IMAGELIST_START + 61)
#define IMG_HTML                        (IMAGELIST_START + 63)
#define IMG_JPG                         (IMAGELIST_START + 64)
#define IMG_MATH                        (IMAGELIST_START + 68)
#define IMG_MATHTEMPLATE                (IMAGELIST_START + 69)
#define IMG_FILE                        (IMAGELIST_START + 74)
#define IMG_PCD                         (IMAGELIST_START + 76)
#define IMG_PCT                         (IMAGELIST_START + 77)
#define IMG_PCX                         (IMAGELIST_START + 78)
#define IMG_SIM                         (IMAGELIST_START + 79)
#define IMG_TEXTFILE                    (IMAGELIST_START + 80)
#define IMG_TIFF                        (IMAGELIST_START + 82)
#define IMG_WMF                         (IMAGELIST_START + 84)
#define IMG_WRITER                      (IMAGELIST_START + 86)
#define IMG_WRITERTEMPLATE              (IMAGELIST_START + 87)
#define IMG_FIXEDDEV                    (IMAGELIST_START + 88)
#define IMG_REMOVABLEDEV                (IMAGELIST_START + 89)
#define IMG_CDROMDEV                    (IMAGELIST_START + 90)
#define IMG_NETWORKDEV                  (IMAGELIST_START + 91)
#define IMG_TABLE                       (IMAGELIST_START + 112)
#define IMG_FOLDER                      (IMAGELIST_START + 113)
#define IMG_DXF                         (IMAGELIST_START + 141)
#define IMG_MET                         (IMAGELIST_START + 142)
#define IMG_PNG                         (IMAGELIST_START + 143)
#define IMG_SGF                         (IMAGELIST_START + 144)
#define IMG_SGV                         (IMAGELIST_START + 145)
#define IMG_SVM                         (IMAGELIST_START + 146)
#define IMG_GLOBAL_DOC                  (IMAGELIST_START + 150)
#define IMG_DRAW                        (IMAGELIST_START + 151)
#define IMG_DRAWTEMPLATE                (IMAGELIST_START + 152)
#define IMG_TEMPLATE                    (IMAGELIST_START + 166)
#define IMG_OO_DATABASE_DOC             (IMAGELIST_START + 169)
#define IMG_OO_DRAW_DOC                 (IMAGELIST_START + 170)
#define IMG_OO_MATH_DOC                 (IMAGELIST_START + 171)
#define IMG_OO_GLOBAL_DOC               (IMAGELIST_START + 172)
#define IMG_OO_IMPRESS_DOC              (IMAGELIST_START + 173)
#define IMG_OO_CALC_DOC                 (IMAGELIST_START + 174)
#define IMG_OO_WRITER_DOC               (IMAGELIST_START + 175)
#define IMG_OO_DRAW_TEMPLATE            (IMAGELIST_START + 176)
#define IMG_OO_IMPRESS_TEMPLATE         (IMAGELIST_START + 177)
#define IMG_OO_CALC_TEMPLATE            (IMAGELIST_START + 178)
#define IMG_OO_WRITER_TEMPLATE          (IMAGELIST_START + 179)
#define IMG_EXTENSION                   (IMAGELIST_START + 180)


class Image;
namespace rtl {
  class OUString;
};
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
    static rtl::OUString    GetDescription_Impl( const INetURLObject& rObject, bool bDetectFolder );

public:
    SVT_DLLPUBLIC static Image  GetImage( const INetURLObject& rURL, bool bBig = false );
    SVT_DLLPUBLIC static Image  GetFileImage( const INetURLObject& rURL );
    SVT_DLLPUBLIC static Image  GetImageNoDefault( const INetURLObject& rURL, bool bBig = false );
    SVT_DLLPUBLIC static Image  GetFolderImage( const svtools::VolumeInfo& rInfo );

    SVT_DLLPUBLIC static rtl::OUString GetDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static rtl::OUString GetFileDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static rtl::OUString GetFolderDescription( const svtools::VolumeInfo& rInfo );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
