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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _INDEX_HXX
#include <index.hxx>
#endif
#ifndef _NDARR_HXX
#include <ndarr.hxx>
#endif
namespace binfilter {


void SwNds::Insert(const SwNodePtr &aElement, const SwIndex & aPos)
{
    theArr.Insert((const ElementPtr&) aElement, aPos.GetIndex());
    SwIndexReg::Update(aPos, 1);
}

void SwNds::Insert(const SwNodePtr* pElement, USHORT nLen, const SwIndex & aPos)
{
    theArr.Insert((const ElementPtr*) pElement, nLen, aPos.GetIndex());
    SwIndexReg::Update(aPos, nLen);
}

void SwNds::Remove(const SwIndex & aPos, USHORT nLen)
{
    if(nLen)
    {
        theArr.Remove(aPos.GetIndex(), nLen);
        SwIndexReg::Update(aPos, nLen, TRUE);
    }
}

BOOL SwNds::Move( const SwIndex & rOldPos, const SwIndex & rNewPos )
{
    register USHORT nDelPos = rOldPos.GetIndex(),
                    nInsPos = rNewPos.GetIndex();
    if( nDelPos == nInsPos || nDelPos +1 == nInsPos )
        return FALSE;
    theArr.Move( nDelPos, nInsPos );
    SwIndexReg::MoveIdx( rOldPos, rNewPos );
    return TRUE;
}

}
