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

#ifndef CSV_MBSTREAM_HXX
#define CSV_MBSTREAM_HXX

// USED SERVICES
    // BASE CLASSES
#include <cosv/bstream.hxx>
    // COMPONENTS
    // PARAMETERS


namespace csv
{

class mbstream : public bstream
{
  public:
    // LIFECYCLE
                        mbstream(
                            uintt               i_nSize);
                        ~mbstream();
    // OPERATIONS
    void                resize(
                            uintt               i_nSize );
    //  INQUIRY
    uintt               size() const;
    const void *        data() const;

  private:
    // Interface bistream:
    virtual uintt       do_read(
                            void *          out_pDest,
                            uintt           i_nNrofBytes);
    virtual bool        inq_eod() const;
    // Interface bostream:
    virtual uintt       do_write(
                            const void *    i_pSrc,
                            uintt           i_nNrofBytes);
    // Interface bstream:
    virtual uintt       do_seek(
                            intt            i_nDistance,
                            seek_dir        i_eStartPoint = ::csv::beg );
    virtual uintt       inq_position() const;

    // DYN
    DYN char *          dpOwnedMemorySpace;
    uintt               nSize;
    uintt               nCurPosition;
};


// IMPLEMENTATION

inline uintt
mbstream::size() const
    { return nSize; }
inline const void *
mbstream::data() const
    { return dpOwnedMemorySpace; }


}   // namespace csv


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
