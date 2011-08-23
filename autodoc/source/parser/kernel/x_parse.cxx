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
#include <x_parse.hxx>

// NOT FULLY DECLARED SERVICES



X_Parser::X_Parser( E_Event				i_eEvent,
                    const char *		i_sObject,
                    const String &      i_sCausingFile_FullPath,
                    uintt				i_nCausingLineNr  )
    :	eEvent(i_eEvent),
        sObject(i_sObject),
        sCausingFile_FullPath(i_sCausingFile_FullPath),
        nCausingLineNr(i_nCausingLineNr)
{
}

X_Parser::~X_Parser()
{
}

X_Parser::E_Event
X_Parser::GetEvent() const
{
     return eEvent;
}

void
X_Parser::GetInfo( std::ostream & o_rOutputMedium ) const
{
    o_rOutputMedium << "Error in file "
                    << sCausingFile_FullPath
                    << " in line "
                    << nCausingLineNr
                    << ": ";


    switch (eEvent)
    {
        case x_InvalidChar:
            o_rOutputMedium << "Unknown character '"
                            << sObject
                            << "'";
            break;
        case x_UnexpectedToken:
            o_rOutputMedium << "Unexpected token \""
                            << sObject
                            << "\"";
            break;
        case x_UnexpectedEOF:
            o_rOutputMedium << "Unexpected end of file.";
            break;
        case x_UnspecifiedSyntaxError:
            o_rOutputMedium << "Unspecified syntax problem in file.";
            break;
        case x_Any:
        default:
            o_rOutputMedium << "Unspecified parsing exception.";
    }	// end switch
    o_rOutputMedium << Endl();
}


std::ostream &
operator<<( std::ostream &                  o_rOut,
            const autodoc::X_Parser_Ifc &   i_rException )
{
    i_rException.GetInfo(o_rOut);
     return o_rOut;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
