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
#include <ary/cessentl.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_ce.hxx>
#include <ary/doc/d_oldcppdocu.hxx>


namespace ary
{
namespace cpp
{


CeEssentials::CeEssentials()
    :   sLocalName(),
        nOwner(0),
        nLocation(0)
{
}

CeEssentials::CeEssentials( const String  &     i_sLocalName,
                            Cid					i_nOwner,
                            loc::Le_id  		i_nLocation )
    :   sLocalName(i_sLocalName),
        nOwner(i_nOwner),
        nLocation(i_nLocation)
{
}

CeEssentials::~CeEssentials()
{
}



inline bool
IsInternal(const doc::Documentation & i_doc)
{
    const ary::doc::OldCppDocu *
        docu = dynamic_cast< const ary::doc::OldCppDocu* >(i_doc.Data());
    if (docu != 0)
        return docu->IsInternal();
    return false;
}


bool
CodeEntity::IsVisible() const
{
    // KORR_FUTURE:   Improve the whole handling of internal and visibility.
    return bIsVisible && NOT IsInternal(Docu());
}



}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
