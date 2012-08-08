/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVTOOLS_IMAGEMGR_HXX
#define _SVTOOLS_IMAGEMGR_HXX

#include "svtools/svtdllapi.h"
#include "sal/types.h"

class Image;
class String;
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
    SVT_DLLPRIVATE static String    GetDescription_Impl( const INetURLObject& rObject, sal_Bool bDetectFolder );

public:
    SVT_DLLPUBLIC static Image  GetImage( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    SVT_DLLPUBLIC static Image  GetFileImage( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    SVT_DLLPUBLIC static Image  GetImageNoDefault( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    SVT_DLLPUBLIC static Image  GetFolderImage( const svtools::VolumeInfo& rInfo, sal_Bool bBig = sal_False );

    SVT_DLLPUBLIC static String GetDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static String GetFileDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static String GetFolderDescription( const svtools::VolumeInfo& rInfo );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
