/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: userdat.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:46:30 $
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

#ifndef SC_USERDAT_HXX
#define SC_USERDAT_HXX

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif


//-------------------------------------------------------------------------

#define SC_DRAWLAYER 0x30334353     // Inventor: "SC30"

// Object-Ids fuer UserData
#define SC_UD_OBJDATA       1
#define SC_UD_IMAPDATA      2

//-------------------------------------------------------------------------

class ScDrawObjFactory
{
    DECL_LINK( MakeUserData, SdrObjFactory * );
public:
    ScDrawObjFactory();
   ~ScDrawObjFactory();
};

//-------------------------------------------------------------------------

class ScDrawObjData : public SdrObjUserData
{
    virtual SdrObjUserData* Clone(SdrObject* pObj) const;

public:
    ScAddress aStt, aEnd;
    BOOL bValidStart, bValidEnd;
    ScDrawObjData();
    ScDrawObjData( const ScDrawObjData& );
    virtual ~ScDrawObjData();
};

//-------------------------------------------------------------------------

class ScIMapInfo : public SdrObjUserData
{
    ImageMap        aImageMap;

public:
                    ScIMapInfo();
                    ScIMapInfo( const ImageMap& rImageMap );
                    ScIMapInfo( const ScIMapInfo& rIMapInfo );
    virtual         ~ScIMapInfo();

    virtual SdrObjUserData* Clone( SdrObject* pObj ) const;

    void    SetImageMap( const ImageMap& rIMap )    { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const             { return aImageMap; }
};


#endif


