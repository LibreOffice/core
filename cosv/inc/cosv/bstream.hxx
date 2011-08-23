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
    virtual				~bistream() {}

    //	OPERATIONS
        /// @return Number of actually read bytes.
    uintt 		        read(
                            void *	        out_pDest,
                            uintt           i_nNrofBytes);
    // INQUIRY
        /**  @return True, if already one try to read had failed.
             There is no guarantee, that it returns true, if end of data
             is just reached.
             Though it will return false, if there is still somemething
             to read.
        */
    bool		        eod() const;

  private:
    virtual uintt 		do_read(
                            void *	        out_pDest,
                            uintt           i_nNrofBytes) = 0;
    virtual bool		inq_eod() const = 0;
};


class bostream
{
  public:
    // LIFECYCLE
    virtual				~bostream() {}

    //	OPERATIONS
        /// @return Number of actually written bytes.
    uintt 		        write(
                            const void *   	i_pSrc,
                            uintt           i_nNrofBytes);
        /// @return Number of actually written bytes.
    uintt 				write(
                            const char *   	i_pSrc );
        /// @return Number of actually written bytes.
    uintt 				write(
                            const String &	i_pSrc );
  private:
    virtual uintt 		do_write(
                            const void *   	i_pSrc,
                            uintt           i_nNrofBytes) = 0;
};


class bstream : public bistream,
                public bostream
{
  public:
    uintt 		        seek(
                            intt 			i_nDistanceFromBegin,
                            seek_dir        i_eStartPoint = ::csv::beg );
    uintt 		        position() const;

  private:
    virtual uintt 		do_seek(
                            intt 			i_nDistance,
                            seek_dir        i_eStartPoint = ::csv::beg ) = 0;
    virtual uintt 		inq_position() const = 0;
};


// IMPLEMENTATION
inline uintt 
bistream::read( void *	       o_pDest,
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
bostream::write( const String &	i_sSrc )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
