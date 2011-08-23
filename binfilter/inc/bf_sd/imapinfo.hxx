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

#ifndef _SD_IMAPINFO_HXX
#define _SD_IMAPINFO_HXX

#ifndef _SD_SDIOCMPT_HXX
#include "sdiocmpt.hxx"
#endif
#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <bf_svtools/imap.hxx>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {


/*************************************************************************
|*
|*
|*
\************************************************************************/

class SdIMapInfo : public SdrObjUserData, public SfxListener
{

    ImageMap		aImageMap;

public:
                    SdIMapInfo() :
                        SdrObjUserData( SdUDInventor, SD_IMAPINFO_ID, 0 ) {};

                    SdIMapInfo( const ImageMap& rImageMap ) :
                        SdrObjUserData( SdUDInventor, SD_IMAPINFO_ID, 0 ),
                        aImageMap( rImageMap ) {};

                    SdIMapInfo( const SdIMapInfo& rIMapInfo ) :
                        SdrObjUserData( SdUDInventor, SD_IMAPINFO_ID, 0 ),
                        aImageMap( rIMapInfo.aImageMap ) {};

    virtual 		~SdIMapInfo() {};

    virtual SdrObjUserData* Clone( SdrObject* pObj ) const { return new SdIMapInfo( *this ); }

    virtual void WriteData( SvStream& rOStm );
    virtual void ReadData( SvStream& rIStm );

    void			SetImageMap( const ImageMap& rIMap ) { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const { return aImageMap; }
};


/*************************************************************************
|*
|*
|*
\************************************************************************/

inline void SdIMapInfo::WriteData( SvStream& rOStm )
{
    SdrObjUserData::WriteData( rOStm );

    SdIOCompat aIO( rOStm, STREAM_WRITE, 1 );

    aImageMap.Write(
        rOStm, ::binfilter::StaticBaseUrl::GetBaseURL(INetURLObject::NO_DECODE));
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

inline void SdIMapInfo::ReadData( SvStream& rIStm )
{
    SdrObjUserData::ReadData( rIStm );

    SdIOCompat aIO( rIStm, STREAM_READ );

    aImageMap.Read(
        rIStm, ::binfilter::StaticBaseUrl::GetBaseURL(INetURLObject::NO_DECODE));
}


} //namespace binfilter
#endif		// _SD_IMAPINFO_HXX


