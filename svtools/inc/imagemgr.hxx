/*************************************************************************
 *
 *  $RCSfile: imagemgr.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: pb $ $Date: 2002-04-11 09:59:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVTOOLS_IMAGEMGR_HXX
#define _SVTOOLS_IMAGEMGR_HXX

// includes ******************************************************************

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _IMAGE_HXX
#include <vcl/image.hxx>
#endif

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
    static String   GetDescription_Impl( const INetURLObject& rObject, sal_Bool bDetectFolder );

public:
    // depricated, because no high contrast mode
    static Image    GetImage( const INetURLObject& rURL, BOOL bBig = FALSE );
    static Image    GetFileImage( const INetURLObject& rURL, BOOL bBig = FALSE );
    static Image    GetImageNoDefault( const INetURLObject& rURL, BOOL bBig = FALSE );
    static Image    GetFolderImage( const svtools::VolumeInfo& rInfo, BOOL bBig = FALSE );

    // now with high contrast mode
    static Image    GetImage( const INetURLObject& rURL, BOOL bBig, BOOL bHighContrast );
    static Image    GetFileImage( const INetURLObject& rURL, BOOL bBig, BOOL bHighContrast );
    static Image    GetImageNoDefault( const INetURLObject& rURL, BOOL bBig, BOOL bHighContrast );
    static Image    GetFolderImage( const svtools::VolumeInfo& rInfo, BOOL bBig, BOOL bHighContrast );

    static String   GetDescription( const INetURLObject& rObject );
    static String   GetFileDescription( const INetURLObject& rObject );
    static String   GetFolderDescription( const svtools::VolumeInfo& rInfo );
};

#endif

