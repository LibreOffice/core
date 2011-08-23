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

#ifndef SC_USERDAT_HXX
#define SC_USERDAT_HXX

#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif

#ifndef _IMAP_HXX //autogen
#include <bf_svtools/imap.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {


//-------------------------------------------------------------------------

#define SC_DRAWLAYER 0x30334353		// Inventor: "SC30"

// Object-Ids fuer UserData
#define SC_UD_OBJDATA		1
#define SC_UD_IMAPDATA		2

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
    virtual void WriteData(SvStream& rOut);
    virtual void ReadData(SvStream& rIn);
public:
    ScTripel aStt, aEnd;
    BOOL bValidStart, bValidEnd;
    ScDrawObjData();
    ScDrawObjData( const ScDrawObjData& );
    virtual ~ScDrawObjData();
};

//-------------------------------------------------------------------------

class ScIMapInfo : public SdrObjUserData
{
    ImageMap		aImageMap;

public:
                    ScIMapInfo();
                    ScIMapInfo( const ImageMap& rImageMap );
                    ScIMapInfo( const ScIMapInfo& rIMapInfo );
    virtual			~ScIMapInfo();

    virtual	SdrObjUserData* Clone( SdrObject* pObj ) const;

    virtual void WriteData( SvStream& rOStm );
    virtual void ReadData( SvStream& rIStm );

    void 	SetImageMap( const ImageMap& rIMap )	{ aImageMap = rIMap; }
    const ImageMap&	GetImageMap() const				{ return aImageMap; }
};


} //namespace binfilter
#endif


