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

#include <precomp.h>
#include "pm_base.hxx"


// NOT FULLY DEFINED SERVICES
#include "opageenv.hxx"
#include "pagemake.hxx"


//********************       SpecializedPageMaker        *********************//

SpecializedPageMaker::SpecializedPageMaker( PageDisplay &  io_rPage )
    :   pEnv( &io_rPage.Env() ),
        pCurOut( &io_rPage.CurOut() ),
        pPage( &io_rPage )
{
}

void
SpecializedPageMaker::Write_NavBar()
{
     // Dummy
}

void
SpecializedPageMaker::Write_TopArea()
{
     // Dummy
}

void
SpecializedPageMaker::Write_DocuArea()
{
     // Dummy
}

//void
//SpecializedPageMaker::Write_ChildList( ary::SlotAccessId   ,
//                                       const char *        ,
//                                       const char *        )
//{
// 	// Dummy
//}

csi::xml::Element &
SpecializedPageMaker::CurOut()
{
    return Page().CurOut();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
