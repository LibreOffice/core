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


#undef SC_DLLIMPLEMENTATION



#include "tpsubt.hxx"
#include "scresid.hxx"
#include "subtdlg.hxx"
#include "subtdlg.hrc"


//==================================================================

ScSubTotalDlg::ScSubTotalDlg( Window*           pParent,
                              const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( RID_SCDLG_SUBTOTALS ),
                      pArgSet ),
        aBtnRemove  ( m_pActionArea, ScResId( BTN_REMOVE ) )
{
    AddTabPage( PAGE_GROUP1,  ScTpSubTotalGroup1::Create,  0 );
    AddTabPage( PAGE_GROUP2,  ScTpSubTotalGroup2::Create,  0 );
    AddTabPage( PAGE_GROUP3,  ScTpSubTotalGroup3::Create,  0 );
    AddTabPage( PAGE_OPTIONS, ScTpSubTotalOptions::Create, 0 );
    aBtnRemove.SetClickHdl( LINK( this, ScSubTotalDlg, RemoveHdl ) );
    FreeResource();
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScSubTotalDlg, RemoveHdl, PushButton *, pBtn )
{
    if ( pBtn == &aBtnRemove )
    {
        EndDialog( SCRET_REMOVE );
    }
    return 0;
}
IMPL_LINK_INLINE_END( ScSubTotalDlg, RemoveHdl, PushButton *, pBtn )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
