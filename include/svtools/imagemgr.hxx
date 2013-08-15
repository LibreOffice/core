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

#ifndef _SVTOOLS_IMAGEMGR_HXX
#define _SVTOOLS_IMAGEMGR_HXX

#include "svtools/svtdllapi.h"
#include "sal/types.h"

class Image;
namespace rtl {
  class OUString;
};
class INetURLObject;

namespace svtools {

struct VolumeInfo
{
    sal_Bool    m_bIsVolume;
    sal_Bool    m_bIsRemote;
    sal_Bool    m_bIsRemoveable;
    sal_Bool    m_bIsFloppy;
    sal_Bool    m_bIsCompactDisc;

    VolumeInfo() :
        m_bIsVolume     ( sal_False ),
        m_bIsRemote     ( sal_False ),
        m_bIsRemoveable ( sal_False ),
        m_bIsFloppy     ( sal_False ),
        m_bIsCompactDisc( sal_False ) {}

    VolumeInfo( sal_Bool _bIsVolume,
                sal_Bool _bIsRemote,
                sal_Bool _bIsRemoveable,
                sal_Bool _bIsFloppy,
                sal_Bool _bIsCompactDisc ) :
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
    SVT_DLLPRIVATE static rtl::OUString    GetDescription_Impl( const INetURLObject& rObject, sal_Bool bDetectFolder );

public:
    SVT_DLLPUBLIC static Image  GetImage( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    SVT_DLLPUBLIC static Image  GetFileImage( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    SVT_DLLPUBLIC static Image  GetImageNoDefault( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    SVT_DLLPUBLIC static Image  GetFolderImage( const svtools::VolumeInfo& rInfo, sal_Bool bBig = sal_False );

    SVT_DLLPUBLIC static rtl::OUString GetDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static rtl::OUString GetFileDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static rtl::OUString GetFolderDescription( const svtools::VolumeInfo& rInfo );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
