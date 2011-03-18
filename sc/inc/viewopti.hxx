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

#ifndef SC_VIEWOPTI_HXX
#define SC_VIEWOPTI_HXX

#include <svx/optgrid.hxx>

#include <svx/svxids.hrc>
#include "scdllapi.h"
#include "optutil.hxx"
#include "global.hxx"

//==================================================================
// View-Optionen
//==================================================================

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
    VOPT_HELPLINES,
    VOPT_ANCHOR,
    VOPT_PAGEBREAKS,
    VOPT_SOLIDHANDLES,
    VOPT_CLIPMARKS,
    VOPT_BIGHANDLES
};

enum ScVObjType
{
    VOBJ_TYPE_OLE = 0,
    VOBJ_TYPE_CHART,
    VOBJ_TYPE_DRAW
};

#define MAX_OPT             (sal_uInt16)VOPT_BIGHANDLES+1
#define MAX_TYPE            (sal_uInt16)VOBJ_TYPE_DRAW+1

#define SC_STD_GRIDCOLOR    COL_LIGHTGRAY

//==================================================================
// SvxGrid-Optionen mit Standard-Operatoren
//==================================================================

class ScGridOptions : public SvxOptionsGrid
{
public:
                ScGridOptions() : SvxOptionsGrid() {}
                ScGridOptions( const SvxOptionsGrid& rOpt ) : SvxOptionsGrid( rOpt ) {}

    void                    SetDefaults();
    const ScGridOptions&    operator=  ( const ScGridOptions& rCpy );
    int                     operator== ( const ScGridOptions& rOpt ) const;
    int                     operator!= ( const ScGridOptions& rOpt ) const { return !(operator==(rOpt)); }
};

//==================================================================
// Einstellungen - Ansicht
//==================================================================

class SC_DLLPUBLIC ScViewOptions
{
public:
                ScViewOptions();
                ScViewOptions( const ScViewOptions& rCpy );
                ~ScViewOptions();

    void                    SetDefaults();

    void                    SetOption( ScViewOption eOpt, sal_Bool bNew = sal_True )    { aOptArr[eOpt] = bNew; }
    sal_Bool                    GetOption( ScViewOption eOpt ) const                { return aOptArr[eOpt]; }

    void                    SetObjMode( ScVObjType eObj, ScVObjMode eMode ) { aModeArr[eObj] = eMode; }
    ScVObjMode              GetObjMode( ScVObjType eObj ) const             { return aModeArr[eObj]; }

    void                    SetGridColor( const Color& rCol, const String& rName ) { aGridCol = rCol; aGridColName = rName;}
    Color                   GetGridColor( String* pStrName = NULL ) const;

    const ScGridOptions&    GetGridOptions() const                      { return aGridOpt; }
    void                    SetGridOptions( const ScGridOptions& rNew ) { aGridOpt = rNew; }
    SvxGridItem*            CreateGridItem( sal_uInt16 nId = SID_ATTR_GRID_OPTIONS ) const;

    const ScViewOptions&    operator=  ( const ScViewOptions& rCpy );
    int                     operator== ( const ScViewOptions& rOpt ) const;
    int                     operator!= ( const ScViewOptions& rOpt ) const { return !(operator==(rOpt)); }

private:
    sal_Bool            aOptArr     [MAX_OPT];
    ScVObjMode      aModeArr    [MAX_TYPE];
    Color           aGridCol;
    String          aGridColName;
    ScGridOptions   aGridOpt;
};

//==================================================================
// Item fuer Einstellungsdialog - Ansicht
//==================================================================

class SC_DLLPUBLIC ScTpViewItem : public SfxPoolItem
{
public:
                TYPEINFO();
                ScTpViewItem( sal_uInt16 nWhich, const ScViewOptions& rOpt );
                ScTpViewItem( const ScTpViewItem&  rItem );
                ~ScTpViewItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScViewOptions&    GetViewOptions() const { return theOptions; }

private:
    ScViewOptions   theOptions;
};


//==================================================================
// CfgItem fuer View-Optionen
//==================================================================

class ScViewCfg : public ScViewOptions
{
    ScLinkConfigItem    aLayoutItem;
    ScLinkConfigItem    aDisplayItem;
    ScLinkConfigItem    aGridItem;

    DECL_LINK( LayoutCommitHdl, void* );
    DECL_LINK( DisplayCommitHdl, void* );
    DECL_LINK( GridCommitHdl, void* );

    com::sun::star::uno::Sequence<rtl::OUString> GetLayoutPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetDisplayPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetGridPropertyNames();

public:
            ScViewCfg();

    void            SetOptions( const ScViewOptions& rNew );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
