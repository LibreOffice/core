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

#ifndef INCLUDED_SC_INC_VIEWOPTI_HXX
#define INCLUDED_SC_INC_VIEWOPTI_HXX

#include <svx/optgrid.hxx>

#include <svx/svxids.hrc>
#include "scdllapi.h"
#include "optutil.hxx"
#include "global.hxx"

// View options

enum ScViewOption
{
    VOPT_FORMULAS = 0,
    VOPT_NULLVALS,
    VOPT_SYNTAX,
    VOPT_NOTES,
    VOPT_VSCROLL,
    VOPT_HSCROLL,
    VOPT_TABCONTROLS,
    VOPT_OUTLINER,
    VOPT_HEADER,
    VOPT_GRID,
    VOPT_GRID_ONTOP,
    VOPT_HELPLINES,
    VOPT_ANCHOR,
    VOPT_PAGEBREAKS,
    VOPT_CLIPMARKS
};

enum ScVObjType
{
    VOBJ_TYPE_OLE = 0,
    VOBJ_TYPE_CHART,
    VOBJ_TYPE_DRAW
};

#define MAX_OPT             (sal_uInt16)VOPT_CLIPMARKS+1
#define MAX_TYPE            (sal_uInt16)VOBJ_TYPE_DRAW+1

#define SC_STD_GRIDCOLOR    COL_LIGHTGRAY

// SvxGrid options with standard operators

class ScGridOptions : public SvxOptionsGrid
{
public:
                ScGridOptions() : SvxOptionsGrid() {}
                ScGridOptions( const SvxOptionsGrid& rOpt ) : SvxOptionsGrid( rOpt ) {}

    void                    SetDefaults();
    ScGridOptions&          operator=  ( const ScGridOptions& rCpy );
    bool                    operator== ( const ScGridOptions& rOpt ) const;
    bool                    operator!= ( const ScGridOptions& rOpt ) const { return !(operator==(rOpt)); }
};

// Options - View

class SC_DLLPUBLIC ScViewOptions
{
public:
                ScViewOptions();
                ScViewOptions( const ScViewOptions& rCpy );
                ~ScViewOptions();

    void                    SetDefaults();

    void                    SetOption( ScViewOption eOpt, bool bNew = true )    { aOptArr[eOpt] = bNew; }
    bool                    GetOption( ScViewOption eOpt ) const                { return aOptArr[eOpt]; }

    void                    SetObjMode( ScVObjType eObj, ScVObjMode eMode ) { aModeArr[eObj] = eMode; }
    ScVObjMode              GetObjMode( ScVObjType eObj ) const             { return aModeArr[eObj]; }

    void                    SetGridColor( const Color& rCol, const OUString& rName ) { aGridCol = rCol; aGridColName = rName;}
    Color const &           GetGridColor( OUString* pStrName = nullptr ) const;

    const ScGridOptions&    GetGridOptions() const                      { return aGridOpt; }
    void                    SetGridOptions( const ScGridOptions& rNew ) { aGridOpt = rNew; }
    SvxGridItem*            CreateGridItem() const;

    ScViewOptions&          operator=  ( const ScViewOptions& rCpy );
    bool                    operator== ( const ScViewOptions& rOpt ) const;
    bool                    operator!= ( const ScViewOptions& rOpt ) const { return !(operator==(rOpt)); }

private:
    bool            aOptArr     [MAX_OPT];
    ScVObjMode      aModeArr    [MAX_TYPE];
    Color           aGridCol;
    OUString        aGridColName;
    ScGridOptions   aGridOpt;
};

// Item for the options dialoge - View

class SC_DLLPUBLIC ScTpViewItem : public SfxPoolItem
{
public:
                static SfxPoolItem* CreateDefault();
                ScTpViewItem( sal_uInt16 nWhich, const ScViewOptions& rOpt );
                ScTpViewItem( const ScTpViewItem&  rItem );
                virtual ~ScTpViewItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const ScViewOptions&    GetViewOptions() const { return theOptions; }

private:
    ScViewOptions   theOptions;
};

// CfgItem for View options

class ScViewCfg : public ScViewOptions
{
    ScLinkConfigItem    aLayoutItem;
    ScLinkConfigItem    aDisplayItem;
    ScLinkConfigItem    aGridItem;

    DECL_LINK_TYPED( LayoutCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK_TYPED( DisplayCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK_TYPED( GridCommitHdl, ScLinkConfigItem&, void );

    static css::uno::Sequence<OUString> GetLayoutPropertyNames();
    static css::uno::Sequence<OUString> GetDisplayPropertyNames();
    static css::uno::Sequence<OUString> GetGridPropertyNames();

public:
            ScViewCfg();

    void            SetOptions( const ScViewOptions& rNew );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
