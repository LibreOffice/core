/*************************************************************************
 *
 *  $RCSfile: bstream.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:25:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CSV_BSTREAM_HXX
#define CSV_BSTREAM_HXX

#include <string.h>
#include <cosv/string.hxx>


namespace csv
{


enum seek_dir
{
    beg = 0,
    cur = 1,
    end = 2
};


class bistream
{
  public:
    // LIFECYCLE
    virtual             ~bistream() {}

    //  OPERATIONS
        /// @return Number of actually read bytes.
    uintt               read(
                            void *          out_pDest,
                            uintt           i_nNrofBytes);
    // INQUIRY
        /**  @return True, if already one try to read had failed.
             There is no guarantee, that it returns true, if end of data
             is just reached.
             Though it will return false, if there is still somemething
             to read.
        */
    bool                eod() const;

  private:
    virtual uintt       do_read(
                            void *          out_pDest,
                            uintt           i_nNrofBytes) = 0;
    virtual bool        inq_eod() const = 0;
};


class bostream
{
  public:
    // LIFECYCLE
    virtual             ~bostream() {}

    //  OPERATIONS
        /// @return Number of actually written bytes.
    uintt               write(
                            const void *    i_pSrc,
                            uintt           i_nNrofBytes);
        /// @return Number of actually written bytes.
    uintt               write(
                            const char *    i_pSrc );
        /// @return Number of actually written bytes.
    uintt               write(
                            const String &  i_pSrc );
  private:
    virtual uintt       do_write(
                            const void *    i_pSrc,
                            uintt           i_nNrofBytes) = 0;
};


class bstream : public bistream,
                public bostream
{
  public:
    uintt               seek(
                            intt            i_nDistanceFromBegin,
                            seek_dir        i_eStartPoint = ::csv::beg );
    uintt               position() const;

  private:
    virtual uintt       do_seek(
                            intt            i_nDistance,
                            seek_dir        i_eStartPoint = ::csv::beg ) = 0;
    virtual uintt       inq_position() const = 0;
};


// IMPLEMENTATION
inline uintt
bistream::read( void *         o_pDest,
               uintt           i_nNrofBytes)
    { return do_read(o_pDest, i_nNrofBytes); }
inline bool
bistream::eod() const
    { return inq_eod(); }

inline uintt
bostream::write( const void *   i_pSrc,
                 uintt          i_nNrofBytes)
    { return do_write( i_pSrc, i_nNrofBytes ); }
inline uintt
bostream::write( const char * i_sSrc )
    { return write( i_sSrc, strlen(i_sSrc) ); }
inline uintt
bostream::write( const String & i_sSrc )
    { return write( i_sSrc.c_str(), i_sSrc.length() ); }

inline uintt
bstream::seek( intt     i_nDistance,
               seek_dir i_eStartPoint )
    { return do_seek( i_nDistance, i_eStartPoint ); }
inline uintt
bstream::position() const
    { return inq_position(); }



}   // namespace csv


#endif

