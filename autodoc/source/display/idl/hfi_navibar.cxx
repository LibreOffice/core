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

#include <precomp.h>
#include "hfi_navibar.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <toolkit/hf_navi_main.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include "hfi_interface.hxx"
#include "hfi_module.hxx"
#include "hfi_service.hxx"
#include "hi_linkhelper.hxx"


extern const String
    C_sLocalManualLinks("#devmanual");


const String        C_sTop      = "Overview";
const String        C_sModule   = "Module";
const String        C_sUse      = "Use";
const String        C_sManual   = "Devguide";
const String        C_sIndex    = "Index";




HF_IdlNavigationBar::HF_IdlNavigationBar( Environment &         io_rEnv,
                                          Xml::Element &        o_rOut )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut)
{
}

HF_IdlNavigationBar::~HF_IdlNavigationBar()
{
}

void
HF_IdlNavigationBar::Produce_CeMainRow( const client & i_ce,
                                        bool  i_bNoUsePage )
{
    HF_NaviMainRow
                aNaviMain( CurOut() );

    StreamLock  aLink(500);
    StreamStr & rLink = aLink();

    Env().Get_LinkTo( rLink.reset(),
                      Env().OutputTree().Overview() );
    aNaviMain.Add_StdItem( C_sTop, rLink.c_str() );

    Env().Get_LinkTo( rLink.reset(),
                      Env().Linker().PositionOf_CurModule() );
    aNaviMain.Add_StdItem( C_sModule, rLink.c_str() );

    if (i_bNoUsePage)
    {
        aNaviMain.Add_NoneItem( C_sUse );
    }
    else
    {
        Env().Get_LinkTo( rLink.reset(),
                          Env().Linker().PositionOf_CurXRefs(i_ce.LocalName()) );
        aNaviMain.Add_StdItem( C_sUse, rLink.c_str() );
    }

    const StringVector &
        rManualDescrs = i_ce.Secondaries().Links2DescriptionInManual();
    if (rManualDescrs.size() == 2)
    {
        aNaviMain.Add_StdItem(C_sManual, Env().Link2Manual( rManualDescrs.front() ));
    }
    else if (rManualDescrs.size() > 2)
    {
        aNaviMain.Add_StdItem(C_sManual, C_sLocalManualLinks);
    }
    else
    {
        aNaviMain.Add_NoneItem( C_sManual );
    }

    Env().Get_LinkTo( rLink.reset(),
                      Env().Linker().PositionOf_Index() );
    aNaviMain.Add_StdItem( C_sIndex, rLink.c_str() );

    aNaviMain.Produce_Row();
}

void
HF_IdlNavigationBar::Produce_CeXrefsMainRow( const client & i_ce )
{
    HF_NaviMainRow
                aNaviMain( CurOut() );

    StreamLock  aLink(500);
    StreamStr & rLink = aLink();

    Env().Get_LinkTo( rLink.reset(),
                      Env().OutputTree().Overview() );
    aNaviMain.Add_StdItem( C_sTop, rLink.c_str() );

    Env().Get_LinkTo( rLink.reset(),
                      Env().Linker().PositionOf_CurModule() );
    aNaviMain.Add_StdItem( C_sModule, rLink.c_str() );

    aNaviMain.Add_SelfItem( C_sUse );

    const StringVector &
        rManualDescrs = i_ce.Secondaries().Links2DescriptionInManual();
    if (rManualDescrs.size() == 2)
    {
        aNaviMain.Add_StdItem(C_sManual, Env().Link2Manual( rManualDescrs.front() ));
    }
    else if (rManualDescrs.size() > 2)
    {
        aNaviMain.Add_StdItem(C_sManual, C_sLocalManualLinks);
    }
    else
    {
        aNaviMain.Add_NoneItem( C_sManual );
    }

    Env().Get_LinkTo( rLink.reset(),
                      Env().Linker().PositionOf_Index() );
    aNaviMain.Add_StdItem( C_sIndex, rLink.c_str() );

    aNaviMain.Produce_Row();
}

void
HF_IdlNavigationBar::Produce_ModuleMainRow( const client & i_ce )
{
    HF_NaviMainRow
        aNaviMain( CurOut() );

    StreamLock  aLink(500);
    StreamStr & rLink = aLink();

    Env().Get_LinkTo( rLink.reset(),
                      Env().OutputTree().Overview() );
    aNaviMain.Add_StdItem( C_sTop, rLink.c_str() );

    aNaviMain.Add_SelfItem( C_sModule );

    aNaviMain.Add_NoneItem( C_sUse );

    const StringVector &
        rManualDescrs = i_ce.Secondaries().Links2DescriptionInManual();
    if (rManualDescrs.size() == 1)
    {
        aNaviMain.Add_StdItem(C_sManual, Env().Link2Manual( rManualDescrs.front() ));
    }
    else if (rManualDescrs.size() > 1)
    {
        aNaviMain.Add_StdItem(C_sManual, C_sLocalManualLinks);
    }
    else
    {
        aNaviMain.Add_NoneItem( C_sManual );
    }

    Env().Get_LinkTo( rLink.reset(),
                      Env().Linker().PositionOf_Index() );
    aNaviMain.Add_StdItem( C_sIndex, rLink.c_str() );

    aNaviMain.Produce_Row();
}

void
HF_IdlNavigationBar::Produce_IndexMainRow()
{
    HF_NaviMainRow
                aNaviMain( CurOut() );

    StreamLock  aLink(500);
    StreamStr & rLink = aLink();

    Env().Get_LinkTo( rLink.reset(),
                      Env().OutputTree().Overview() );
    aNaviMain.Add_StdItem( C_sTop, rLink.c_str() );

    aNaviMain.Add_NoneItem( C_sModule );
    aNaviMain.Add_NoneItem( C_sUse );
    aNaviMain.Add_NoneItem( C_sManual );

    aNaviMain.Add_SelfItem( C_sIndex );

    aNaviMain.Produce_Row();

    CurOut() << new Html::HorizontalLine();
}

DYN HF_NaviSubRow &
HF_IdlNavigationBar::Add_SubRow()
{
    return *new HF_NaviSubRow( CurOut() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
