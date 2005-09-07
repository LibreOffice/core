/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hstream.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:37:57 $
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

#ifndef _HSTREAM_H_
#define _HSTREAM_H_

typedef unsigned char byte;
/**
 * Stream class
 */
class HStream
{
    public:
        HStream();
        virtual ~HStream();

/**
 *
 */
        void addData( const byte *buf, int aToAdd);
/**
 * Read some byte to buf as given size
 */
        int readBytes( byte *buf, int aToRead);
/**
 * Skip some byte from stream as given size
 */
        int skipBytes( int aToSkip );
/**
 * @returns Size of remained stream
 */
        int available();
/**
 * remove the stream from this object.
 */
        void closeInput();
    private:
        int size;
        byte *seq;
        int pos;
};
#endif
