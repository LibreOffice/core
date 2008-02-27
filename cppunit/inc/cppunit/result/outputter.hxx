/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outputter.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:15:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#include "cppunit/nocopy.hxx"

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

