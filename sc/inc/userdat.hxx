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

#ifndef INCLUDED_SC_INC_USERDAT_HXX
#define INCLUDED_SC_INC_USERDAT_HXX

#include <svx/svdobj.hxx>
#include <tools/link.hxx>
#include <svtools/imap.hxx>
#include "global.hxx"
#include "address.hxx"

// Object IDs for UserData
#define SC_UD_OBJDATA       1
#define SC_UD_IMAPDATA      2
#define SC_UD_MACRODATA     3

class SC_DLLPUBLIC ScDrawObjData : public SdrObjUserData
{
public:
    enum Type { CellNote, ValidationCircle, DetectiveArrow, DrawingObject };

    ScAddress           maStart;
    ScAddress           maEnd;
    Point               maStartOffset;
    Point               maEndOffset;
    Type                meType;
    tools::Rectangle           maLastRect;

    explicit            ScDrawObjData();

private:
     virtual ScDrawObjData* Clone( SdrObject* pObj ) const override;
};

class ScIMapInfo : public SdrObjUserData
{
    ImageMap        aImageMap;

public:
                    ScIMapInfo( const ImageMap& rImageMap );
                    ScIMapInfo( const ScIMapInfo& rIMapInfo );
    virtual         ~ScIMapInfo() override;

    virtual SdrObjUserData* Clone( SdrObject* pObj ) const override;

    void    SetImageMap( const ImageMap& rIMap )    { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const             { return aImageMap; }
};

class ScMacroInfo : public SdrObjUserData
{
public:
                    ScMacroInfo();
    virtual         ~ScMacroInfo() override;

    virtual SdrObjUserData* Clone( SdrObject* pObj ) const override;

    void            SetMacro( const OUString& rMacro ) { maMacro = rMacro; }
    const OUString& GetMacro() const { return maMacro; }

    void            SetHlink( const OUString& rHlink ) { maHlink = rHlink; }
    const OUString& GetHlink() const { return maHlink; }

private:
    OUString   maMacro;
    OUString   maHlink;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
