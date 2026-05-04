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

#include <scdllapi.h>
#include <address.hxx>
#include "csvsplits.hxx"
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <tools/json_writer.hxx>
#include <vcl/customweld.hxx>

/** Minimum character count for a column in separators mode. */
const sal_Int32 CSV_MINCOLWIDTH         = 8;
/** Maximum length of a cell string. */
const sal_Int32 CSV_MAXSTRLEN          = 0x7FFF;
/** Transparency for header color of selected columns. */
const sal_uInt16 CSV_HDR_TRANSPARENCY   = 85;

//! TODO make string array dynamic
const sal_Int32 CSV_PREVIEW_LINES       = 32; // maximum count of preview lines
/** Maximum count of columns. */
const sal_Int32 CSV_MAXCOLCOUNT         = MAXCOLCOUNT;

/** Default column data type. */
const sal_Int32 CSV_TYPE_DEFAULT        = 0;
/** Multi selection with different types. */
const sal_Int32 CSV_TYPE_MULTI          = -1;
/** No column selected. */
const sal_Int32 CSV_TYPE_NOSELECTION    = -2;

// External used column types.
const sal_uInt8 SC_COL_STANDARD         = 1;
const sal_uInt8 SC_COL_TEXT             = 2;
const sal_uInt8 SC_COL_MDY              = 3;
const sal_uInt8 SC_COL_DMY              = 4;
const sal_uInt8 SC_COL_YMD              = 5;
const sal_uInt8 SC_COL_SKIP             = 9;
const sal_uInt8 SC_COL_ENGLISH          = 10;

/** Exported data of a column (data used in the dialog). */
struct ScCsvExpData
{
    sal_Int32                   mnIndex;        /// Index of a column.
    sal_uInt8                   mnType;         /// External type of the column.

    ScCsvExpData() : mnIndex( 0 ), mnType( SC_COL_STANDARD ) {}
    ScCsvExpData( sal_Int32 nIndex, sal_uInt8 nType ) :
                                    mnIndex( nIndex ), mnType( nType ) {}
};

typedef ::std::vector< ScCsvExpData > ScCsvExpDataVec;

/** Shared layout data for the CSV grid: total character positions and a
    repaint suppression counter. The historical scroll/cursor/dimension
    fields are gone - the client handles all of that. */
struct ScCsvLayoutData
{
    sal_Int32                   mnPosCount;         /// Number of character positions.
    mutable sal_Int32           mnNoRepaint;        /// >0 = no repaint.

    explicit                    ScCsvLayoutData();
};

/** Enumeration of possible commands to change any settings of the CSV controls.
    @descr  Controls have to send commands instead of changing their settings directly.
    A command can contain 0 to 2 sal_Int32 parameters. */
enum ScCsvCmdType
{
    CSVCMD_NONE,                /// No command. [-]
    CSVCMD_REPAINT,             /// Repaint all controls. [-]

    CSVCMD_SETPOSCOUNT,         /// Change position/column count. [character count]

    // table contents
    CSVCMD_NEWCELLTEXTS,        /// Recalculate splits and cell texts. [-]
    CSVCMD_UPDATECELLTEXTS,     /// Update cell texts with current split settings. [-]
    CSVCMD_SETCOLUMNTYPE,       /// Change data type of selected columns. [column type]
    CSVCMD_EXPORTCOLUMNTYPE,    /// Send selected column type to external controls. [-]
    CSVCMD_SETFIRSTIMPORTLINE,  /// Set number of first imported line. [line index]

    // splits
    CSVCMD_INSERTSPLIT,         /// Insert a split. [position]
    CSVCMD_REMOVESPLIT,         /// Remove a split. [position]
    CSVCMD_TOGGLESPLIT          /// Inserts or removes a split. [position]
};

/** Data for a CSV control command. The stored position data is always character based,
    it's never a column index (required for internal consistency). */
class ScCsvCmd
{
private:
    ScCsvCmdType                meType;         /// The command.
    sal_Int32                   mnParam1;       /// First parameter.
    sal_Int32                   mnParam2;       /// Second parameter.

public:
    explicit             ScCsvCmd() : meType( CSVCMD_NONE ),
                                    mnParam1( CSV_POS_INVALID ), mnParam2( CSV_POS_INVALID ) {}

    inline void                 Set( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 );

    ScCsvCmdType         GetType() const     { return meType; }
    sal_Int32            GetParam1() const   { return mnParam1; }
    sal_Int32            GetParam2() const   { return mnParam2; }
};

inline void ScCsvCmd::Set( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 )
{
    meType = eType; mnParam1 = nParam1; mnParam2 = nParam2;
}

/** Base class for the CSV grid control. Implements command handling. */
class UNLESS_MERGELIBS_MORE(SAL_DLLPUBLIC_RTTI) ScCsvControl : public weld::CustomClientWidgetController
{
private:
    Link<ScCsvControl&,void>    maCmdHdl;           /// External command handler.
    ScCsvCmd                    maCmd;              /// Data of last command.
    const ScCsvLayoutData&      mrData;             /// Shared layout data.

public:
    explicit                    ScCsvControl(const ScCsvLayoutData& rData);
    virtual                     ~ScCsvControl() override;

    // repaint helpers --------------------------------------------------------

    /** Repaints all controls. */
    void                        Repaint();
    /** Increases no-repaint counter (controls do not repaint until the last EnableRepaint()). */
    void                        DisableRepaint();
    /** Decreases no-repaint counter and repaints if counter reaches 0. */
    void                        EnableRepaint();
    /** Returns true, if controls will not repaint. */
    bool                 IsNoRepaint() const { return mrData.mnNoRepaint > 0; }

    // command handling -------------------------------------------------------

    /** Sets a new command handler. */
    void                 SetCmdHdl( const Link<ScCsvControl&,void>& rHdl ) { maCmdHdl = rHdl; }
    /** Returns data of the last command. */
    const ScCsvCmd&      GetCmd() const { return maCmd; }

    /** Executes a command by calling command handler. */
    SC_DLLPUBLIC void    Execute(
                                    ScCsvCmdType eType,
                                    sal_Int32 nParam1 = CSV_POS_INVALID,
                                    sal_Int32 nParam2 = CSV_POS_INVALID );

    /** Returns the number of character positions across the data. */
    sal_Int32            GetPosCount() const { return mrData.mnPosCount; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
