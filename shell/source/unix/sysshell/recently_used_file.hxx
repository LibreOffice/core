/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recently_used_file.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:56:10 $
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

#ifndef INCLUDED_RECENTLY_USED_FILE_HXX
#define INCLUDED_RECENTLY_USED_FILE_HXX

#include <stdio.h>
#include <unistd.h>

//##################################
// simple wrapper around the recently_used_file

class recently_used_file
{
public:
    //----------------------------
    recently_used_file();

    //----------------------------
    ~recently_used_file();

    //----------------------------
    // set file pointer to the start of file
    void reset() const;

    //----------------------------
    void truncate(off_t length = 0);

    //----------------------------
    void flush();

    //----------------------------
    size_t read(
        char* buffer,
        size_t size) const;

    //----------------------------
    void write(const char* buffer, size_t size) const;

    //----------------------------
    bool eof() const;

private:
    FILE* file_;
};

#endif // INCLUDED_RECENTLY_USED_FILE_HXX
