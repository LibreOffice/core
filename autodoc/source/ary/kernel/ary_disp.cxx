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
#include <ary/ary_disp.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_ce.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>



namespace ary
{

void
Display::DisplaySlot_Rid( ary::Rid i_nId )
{
    const cpp::Gate *
        pGate = Get_ReFinder();
    if (pGate != 0)
    {
         const  ary::cpp::CodeEntity *
             pRE = pGate->Ces().Search_Ce( cpp::Ce_id(i_nId) );
        if (pRE != 0)
        {
             pRE->Accept( *this );
            return;
        }
    }

    do_DisplaySlot_Rid( i_nId );
}


void
Display::DisplaySlot_LocalCe( ary::cpp::Ce_id   i_nId,
                              const String  &	i_sName )
{
    const cpp::Gate *
        pGate = Get_ReFinder();
    if (pGate != 0)
    {
         const ary::cpp::CodeEntity *
             pRE = pGate->Ces().Search_Ce(i_nId);
        if (pRE != 0)
        {
             pRE->Accept( *this );
            return;
        }
    }

    do_DisplaySlot_LocalCe( i_nId, i_sName );
}



// Dummy implementations for class Display

void
Display::do_StartSlot()
{
}

void
Display::do_FinishSlot()
{
}

void
Display::do_DisplaySlot_Rid( ary::Rid )
{
}

void
Display::do_DisplaySlot_LocalCe( ary::cpp::Ce_id    ,
                                 const String  &	)
{
}


}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
