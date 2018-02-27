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
#include "drwlayer.hxx"

#define SC_DRAWLAYER 0x30334353     // Inventor: "SC30"

// Object-Ids fuer UserData
#define SC_UD_OBJDATA       1
#define SC_UD_IMAPDATA      2
#define SC_UD_MACRODATA     3

class ScDrawObjFactory
{
    DECL_STATIC_LINK_TYPED( ScDrawObjFactory, MakeUserData, SdrObjFactory *, void );
public:
    ScDrawObjFactory();
   ~ScDrawObjFactory();
};

class SC_DLLPUBLIC ScDrawObjData : public SdrObjUserData
{
public:
    enum Type { CellNote, ValidationCircle, DetectiveArrow, DrawingObject };

    ScAddress           maStart;
    ScAddress           maEnd;
    Point               maStartOffset;
    Point               maEndOffset;
    Type                meType;

    bool                mbResizeWithCell = false;
    bool                mbWasInHiddenRow = false;

    explicit            ScDrawObjData();

    Rectangle getShapeRect() { return maShapeRect; };
    Rectangle getLastCellRect() { return maLastCellRect; };
    void setShapeRect(const ScDocument* rDoc, Rectangle rNewRect, bool bIsVisible=true)
    {
        // bIsVisible should be false when the object is hidden obviously. we dont want to store the old cell rect in that
        // case because it will have height=0
        if (maStart.IsValid() && mbResizeWithCell && bIsVisible)
            maLastCellRect = ScDrawLayer::GetCellRect(const_cast<ScDocument&>(*rDoc), maStart, true);
        maShapeRect = rNewRect;
        mbWasInHiddenRow = !bIsVisible;
    };

private:
     virtual ScDrawObjData* Clone( SdrObject* pObj ) const override;

    // Stores the last cell rect this shape was anchored to.
    // Needed when the cell is resized to resize the image accordingly.
    Rectangle maLastCellRect;
    // Stores the rect of the shape to which this ScDrawObjData belongs.
    Rectangle maShapeRect;
};

class ScIMapInfo : public SdrObjUserData
{
    ImageMap        aImageMap;

public:
                    ScIMapInfo();
                    ScIMapInfo( const ImageMap& rImageMap );
                    ScIMapInfo( const ScIMapInfo& rIMapInfo );
    virtual         ~ScIMapInfo();

    virtual SdrObjUserData* Clone( SdrObject* pObj ) const override;

    void    SetImageMap( const ImageMap& rIMap )    { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const             { return aImageMap; }
};

class ScMacroInfo : public SdrObjUserData
{
public:
                    ScMacroInfo();
    virtual         ~ScMacroInfo();

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
