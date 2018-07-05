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
#include <svtools/imap.hxx>
#include "address.hxx"
#include "drwlayer.hxx"

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
    bool                mbResizeWithCell = false;
    bool                mbWasInHiddenRow = false;

    explicit            ScDrawObjData();

    const tools::Rectangle & getShapeRect() { return maShapeRect; };
    const tools::Rectangle & getLastCellRect() { return maLastCellRect; };
    void setShapeRect(const ScDocument* rDoc, tools::Rectangle rNewRect, bool bIsVisible=true)
    {
        // bIsVisible should be false when the object is hidden obviously. we dont want to store the old cell rect in that
        // case because it will have height=0
        if (maStart.IsValid() && mbResizeWithCell && bIsVisible)
            maLastCellRect = ScDrawLayer::GetCellRect(*rDoc, maStart, true);
        maShapeRect = rNewRect;
        mbWasInHiddenRow = !bIsVisible;
    };

private:
     virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject* pObj ) const override;

    // Stores the last cell rect this shape was anchored to.
    // Needed when the cell is resized to resize the image accordingly.
    tools::Rectangle maLastCellRect;
    // Stores the rect of the shape to which this ScDrawObjData belongs.
    tools::Rectangle maShapeRect;
};

class ScIMapInfo : public SdrObjUserData
{
    ImageMap        aImageMap;

public:
                    ScIMapInfo( const ImageMap& rImageMap );
                    ScIMapInfo( const ScIMapInfo& rIMapInfo );
    virtual         ~ScIMapInfo() override;

    virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject* pObj ) const override;

    void    SetImageMap( const ImageMap& rIMap )    { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const             { return aImageMap; }
};

class ScMacroInfo : public SdrObjUserData
{
public:
                    ScMacroInfo();
    virtual         ~ScMacroInfo() override;

    ScMacroInfo(ScMacroInfo const &) = default;
    ScMacroInfo(ScMacroInfo &&) = default;
    ScMacroInfo & operator =(ScMacroInfo const &) = default;
    ScMacroInfo & operator =(ScMacroInfo &&) = default;

    virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject* pObj ) const override;

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
