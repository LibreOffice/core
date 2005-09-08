/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datetime.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:54:30 $
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

#ifndef CSV_DATETIME_HXX
#define CSV_DATETIME_HXX



namespace csv
{


class Date
{
  public:
                        Date();
                        Date(
                            unsigned            i_nDay,
                            unsigned            i_nMonth,
                            unsigned            i_nYear );

    unsigned            Day() const             { return nData >> 24; }
    unsigned            Month() const           { return (nData & 0x00FF0000) >> 16; }
    unsigned            Year() const            { return nData & 0x0000FFFF; }

    static const Date & Null_();

  private:
    UINT32              nData;
};

class Time
{
  public:
                        Time();
                        Time(
                            unsigned            i_nHour,
                            unsigned            i_nMinutes,
                            unsigned            i_nSeconds = 0,
                            unsigned            i_nSeconds100 = 0 );

    unsigned            Hour() const            { return nData >> 24; }
    unsigned            Minutes() const         { return (nData & 0x00FF0000) >> 16; }
    unsigned            Seconds() const         { return (nData & 0x0000FF00) >> 8; }
    unsigned            Seconds100() const      { return nData & 0x000000FF; }

    static const Time & Null_();

  private:
    UINT32              nData;
};


}   // namespace csv




#endif

