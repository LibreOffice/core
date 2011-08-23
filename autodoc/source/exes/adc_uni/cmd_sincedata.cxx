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
#include "cmd_sincedata.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/file.hxx>
#include <cosv/tpl/tpltools.hxx>
#include "adc_cmds.hxx"



namespace autodoc
{
namespace command
{

SinceTagTransformationData::SinceTagTransformationData()
    :   aTransformationTable()
{
}

SinceTagTransformationData::~SinceTagTransformationData()
{
}

bool
SinceTagTransformationData::DoesTransform() const
{
    return NOT aTransformationTable.empty();
}

const String &
SinceTagTransformationData::DisplayOf( const String & i_versionNumber ) const
{
    if (DoesTransform())
    {
        StreamLock
            sl(200);
        sl() << i_versionNumber;
        sl().strip_frontback_whitespace();
        String
            sVersionNumber(sl().c_str());

        const String *
            ret = csv::find_in_map(aTransformationTable, sVersionNumber);
        return ret != 0
                ?   *ret
                :   String::Null_();
    }
    else
    {
        return i_versionNumber;
    }
}

void
SinceTagTransformationData::do_Init( opt_iter &  it,
                                     opt_iter    itEnd )
{
    ++it;   // Cur is since-file path.

    CHECKOPT(   it != itEnd ,
              "file path",
              C_opt_SinceFile );

    csv::File           aSinceFile(*it);
    csv::OpenCloseGuard aSinceFileGuard(aSinceFile);
    StreamStr           sLine(200);

    if (aSinceFileGuard)
    {
        for ( sLine.operator_read_line(aSinceFile);
              NOT sLine.empty();
              sLine.operator_read_line(aSinceFile) )
        {

            if (*sLine.begin() != '"')
                continue;

            const char * pVersion = sLine.c_str() + 1;
            const char * pVersionEnd = strchr(pVersion, '"');
            if (pVersionEnd == 0)
                continue;
            const char * pDisplay = strchr(pVersionEnd+1, '"');
            if (pDisplay == 0)
                continue;
            ++pDisplay;
            const char * pDisplayEnd = strchr(pDisplay, '"');
            if (pDisplayEnd == 0)
                continue;

            aTransformationTable[ String(pVersion,pVersionEnd) ]
                                        = String(pDisplay,pDisplayEnd);
            sLine.clear();
        }   // end for
    }   // end if

    ++it;   // Cur is next option.
}

}   // namespace command
}   // namespace autodoc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
