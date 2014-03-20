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
    SVT_DLLPRIVATE static rtl::OUString    GetDescription_Impl( const INetURLObject& rObject, bool bDetectFolder );

public:
    SVT_DLLPUBLIC static Image  GetImage( const INetURLObject& rURL, bool bBig = false );
    SVT_DLLPUBLIC static Image  GetFileImage( const INetURLObject& rURL, bool bBig = false );
    SVT_DLLPUBLIC static Image  GetImageNoDefault( const INetURLObject& rURL, bool bBig = false );
    SVT_DLLPUBLIC static Image  GetFolderImage( const svtools::VolumeInfo& rInfo, bool bBig = false );

    SVT_DLLPUBLIC static rtl::OUString GetDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static rtl::OUString GetFileDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static rtl::OUString GetFolderDescription( const svtools::VolumeInfo& rInfo );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
