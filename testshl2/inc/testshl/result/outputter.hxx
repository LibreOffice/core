/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

//

#ifndef outputter_hxx
#define outputter_hxx

#include <string>
#include <ostream>
#include <memory>
#include "testshl/log.hxx"
#include <sal/types.h>
#include "testshl/nocopy.hxx"

// #include <fstream>

class Outputter : NOCOPY
{
    std::auto_ptr<Log> m_pLog;
    std::ostream* m_pStream;

    void writeToAll(const sal_Char* _pStr);
public:
    class endl
    {
        char c;
    public:
        endl():c('\0'){}
    };
public:
    Outputter(Log * _pLog )
            :m_pLog(_pLog),
             m_pStream(NULL) {}

    Outputter(std::ostream& _aStream)
            :m_pLog(NULL),
             m_pStream(&_aStream) {}

    ~Outputter();

    void write(const sal_Char*);
    void write(std::string const&);
    void write(sal_Int32);
    // void write(double);
};

Outputter& operator <<( Outputter &stream, const sal_Char* );
Outputter& operator <<( Outputter &stream, std::string const& );
Outputter& operator <<( Outputter &stream, sal_Int32 );
// Outputter& operator <<( Outputter &stream, double );

Outputter& operator <<( Outputter &stream, Outputter::endl const&);

// Outputter& operator <<( Outputter &stream, const char* );

#endif

