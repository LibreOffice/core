/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVTOOLS_IMAGEMGR_HXX
#define _SVTOOLS_IMAGEMGR_HXX

// includes ******************************************************************

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
    // depricated, because no high contrast mode
    SVT_DLLPUBLIC static Image  GetImage( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    static Image    GetFileImage( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    static Image    GetImageNoDefault( const INetURLObject& rURL, sal_Bool bBig = sal_False );
    SVT_DLLPUBLIC static Image  GetFolderImage( const svtools::VolumeInfo& rInfo, sal_Bool bBig = sal_False );

    // now with high contrast mode
    SVT_DLLPUBLIC static Image  GetImage( const INetURLObject& rURL, sal_Bool bBig, sal_Bool bHighContrast );
    SVT_DLLPUBLIC static Image  GetFileImage( const INetURLObject& rURL, sal_Bool bBig, sal_Bool bHighContrast );
    SVT_DLLPUBLIC static Image  GetImageNoDefault( const INetURLObject& rURL, sal_Bool bBig, sal_Bool bHighContrast );
    SVT_DLLPUBLIC static Image  GetFolderImage( const svtools::VolumeInfo& rInfo, sal_Bool bBig, sal_Bool bHighContrast );

    SVT_DLLPUBLIC static String GetDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static String GetFileDescription( const INetURLObject& rObject );
    SVT_DLLPUBLIC static String GetFolderDescription( const svtools::VolumeInfo& rInfo );
};

#endif

