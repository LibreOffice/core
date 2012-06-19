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
#include <x_parse2.hxx>

// NOT FULLY DECLARED SERVICES

    enum E_Type
    {
        x_Any                       = 0,
        x_InvalidChar,
        x_UnexpectedEOF
    };
void
X_AutodocParser::GetInfo( std::ostream &        o_rOutputMedium ) const
{
    switch (eType)
    {
        case x_Any:
            o_rOutputMedium << "Unspecified parsing exception ." << Endl();
            break;
        case x_InvalidChar:
            o_rOutputMedium << "Unknown character during parsing." << Endl();
            break;
        case x_UnexpectedToken:
            o_rOutputMedium << "Unexpected token " << sName << " found." << Endl();
            break;
        case x_UnexpectedEOF:
            o_rOutputMedium << "Unexpected end of file found." << Endl();
            break;
        default:
            o_rOutputMedium << "Unknown exception during parsing." << Endl();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
