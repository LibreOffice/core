/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: csv_ostream.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:38:54 $
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

