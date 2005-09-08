/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imagemgr.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:40:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVTOOLS_IMAGEMGR_HXX
#define _SVTOOLS_IMAGEMGR_HXX

// includes ******************************************************************

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

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

