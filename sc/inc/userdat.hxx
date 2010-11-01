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

#ifndef SC_USERDAT_HXX
#define SC_USERDAT_HXX

#include <svx/svdobj.hxx>
#include <svtools/imap.hxx>
#include "global.hxx"
#include "address.hxx"


//-------------------------------------------------------------------------

#define SC_DRAWLAYER 0x30334353     // Inventor: "SC30"

// Object-Ids fuer UserData
#define SC_UD_OBJDATA       1
#define SC_UD_IMAPDATA      2
#define SC_UD_MACRODATA     3

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
public:
    ScAddress           maStart;
    ScAddress           maEnd;
    bool                mbNote;

    explicit            ScDrawObjData();

private:
    virtual ScDrawObjData* Clone( SdrObject* pObj ) const;
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

//-------------------------------------------------------------------------

class ScMacroInfo : public SdrObjUserData
{
public:
                    ScMacroInfo();
    virtual         ~ScMacroInfo();

    virtual SdrObjUserData* Clone( SdrObject* pObj ) const;

    void            SetMacro( const rtl::OUString& rMacro ) { maMacro = rMacro; }
    const rtl::OUString& GetMacro() const { return maMacro; }

    void            SetHlink( const rtl::OUString& rHlink ) { maHlink = rHlink; }
    const rtl::OUString& GetHlink() const { return maHlink; }

private:
    rtl::OUString   maMacro;
    rtl::OUString   maHlink;
};

//-------------------------------------------------------------------------

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
