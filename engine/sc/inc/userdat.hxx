/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <svx/svdobj.hxx>
#include "address.hxx"
#include "drwlayer.hxx"

class SC_DLLPUBLIC ScDrawObjData final
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

    explicit            ScDrawObjData()
    :   maStart( ScAddress::INITIALIZE_INVALID ),
        maEnd( ScAddress::INITIALIZE_INVALID ),
        meType( DrawingObject ),
        mbResizeWithCell( false )
    {
    }

    const tools::Rectangle & getShapeRect() const { return maShapeRect; };
    const tools::Rectangle & getLastCellRect() const { return maLastCellRect; };
    void setShapeRect(const ScDocument* rDoc, tools::Rectangle rNewRect, bool bIsVisible=true)
    {
        // bIsVisible should be false when the object is hidden obviously. We don't want to store the old cell rect in that
        // case because it will have height=0
        if (maStart.IsValid() && mbResizeWithCell && bIsVisible)
            maLastCellRect = ScDrawLayer::GetCellRect(*rDoc, maStart, true);
        maShapeRect = rNewRect;
        mbWasInHiddenRow = !bIsVisible;
    };

private:
    // Stores the last cell rect this shape was anchored to.
    // Needed when the cell is resized to resize the image accordingly.
    tools::Rectangle maLastCellRect;
    // Stores the rect of the shape to which this ScDrawObjData belongs.
    tools::Rectangle maShapeRect;
};

// tooling class to allow adding a ScDrawObjData to SdrObject as SdrObjUserData
class SC_DLLPUBLIC ScDrawObjData_UserData final : public SdrObjUserData
{
    ScDrawObjData       m_aScDrawObjData;

    virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject* /*pObj*/ ) const override;

public:
    explicit ScDrawObjData_UserData();

    ScDrawObjData& getScDrawObjData() { return m_aScDrawObjData; }
};

// tooling class to allow adding a ScDrawObjData to SdrObject as SdrObjUserData,
// this time with another UserDataID
class SC_DLLPUBLIC ScDrawObjData_UserDataNonRotated final : public SdrObjUserData
{
    ScDrawObjData       m_aScDrawObjData;

    virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject* /*pObj*/ ) const override;

public:
    explicit ScDrawObjData_UserDataNonRotated();

    ScDrawObjData& getScDrawObjData() { return m_aScDrawObjData; }
};

class SAL_DLLPUBLIC_RTTI ScMacroInfo final : public SdrObjUserData
{
public:
                    ScMacroInfo();
    virtual         ~ScMacroInfo() override;

    ScMacroInfo(ScMacroInfo const &) = default;
    ScMacroInfo(ScMacroInfo &&) = default;
    ScMacroInfo & operator =(ScMacroInfo const &) = delete; // due to SdrObjUserData
    ScMacroInfo & operator =(ScMacroInfo &&) = delete; // due to SdrObjUserData

    virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject* pObj ) const override;

    void            SetMacro( const OUString& rMacro ) { maMacro = rMacro; }
    const OUString& GetMacro() const { return maMacro; }

private:
    OUString   maMacro;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
