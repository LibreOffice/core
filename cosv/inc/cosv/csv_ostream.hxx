/*************************************************************************
 *
 *  $RCSfile: csv_ostream.hxx,v $
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

#include <cosv/template/dyn.hxx>

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

