/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mbstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:55:23 $
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


