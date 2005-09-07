/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hstream.cpp,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:37:38 $
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

#include <string.h>
#include <stdlib.h>
#include "hstream.h"

HStream::HStream() : size(0), pos(0)
{
    seq = 0;
}


HStream::~HStream()
{
    if( seq )
        free( seq );
}


void HStream::addData( const byte *buf, int aToAdd)
{
    seq = (byte *)realloc( seq, size + aToAdd );
    memcpy( seq + size, buf, aToAdd );
    size += aToAdd;
}


int HStream::readBytes(byte * buf, int aToRead)
{
    if (aToRead >= (size - pos))
        aToRead = size - pos;
    for (int i = 0; i < aToRead; i++)
        buf[i] = seq[pos++];
    return aToRead;
}


int HStream::skipBytes(int aToSkip)
{
    if (aToSkip >= (size - pos))
        aToSkip = size - pos;
    pos += aToSkip;
    return aToSkip;
}


int HStream::available()
{
    return size - pos;
}


void HStream::closeInput()
{
}
