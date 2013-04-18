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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
