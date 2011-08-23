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

#ifndef CSV_CSV_OSTREAM_HXX
#define CSV_CSV_OSTREAM_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS



#ifndef CSV_NO_IOSTREAMS

#include <iostream>


namespace csv
{

typedef std::ios        ios;
typedef std::ostream    ostream;

}   // namespace csv


#else

#include <cosv/tpl/dyn.hxx>

namespace csv
{

class StreamStr;

class ios
{
  public:
    enum seek_dir
    {
        beg=0,
        cur=1,
        end=2
    };
};

class ostream : public ios
{
  public:
    typedef ostream     self;

    virtual             ~ostream();

    self &              operator<<(
                            const char *        i_s );
    self &              operator<<(
                            char                i_c );
    self &              operator<<(
                            unsigned char       i_c );
    self &              operator<<(
                            signed char         i_c );

    self &              operator<<(
                            short               i_n );
    self &              operator<<(
                            unsigned short      i_n );
    self &              operator<<(
                            int                 i_n );
    self &              operator<<(
                            unsigned int        i_n );
    self &              operator<<(
                            long                i_n );
    self &              operator<<(
                            unsigned long       i_n );

    self &              operator<<(
                            float               i_n );
    self &              operator<<(
                            double              i_n );

    self &              seekp(
                            intt                i_nOffset,
                            seek_dir            i_eStart = ios::beg );
  protected:
                        ostream(
                            uintt               i_nStartSize );
    const StreamStr &   Data() const;

  private:
    Dyn<StreamStr>      pData;
};



inline const StreamStr &
ostream::Data() const
    { return *pData; }


}   // namespace csv


#endif




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
