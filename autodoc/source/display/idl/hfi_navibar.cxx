/*************************************************************************
 *
 *  $RCSfile: hfi_navibar.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 11:33:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


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

