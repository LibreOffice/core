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

#ifndef CSV_DATETIME_HXX
#define CSV_DATETIME_HXX



namespace csv
{


class Date
{
  public:
                        Date();
                        Date(
                            unsigned 			i_nDay,
                            unsigned 			i_nMonth,
                            unsigned 			i_nYear );

    unsigned			Day() const 			{ return nData >> 24; }
    unsigned			Month() const           { return (nData & 0x00FF0000) >> 16; }
    unsigned			Year() const            { return nData & 0x0000FFFF; }

    static const Date &	Null_();

  private:
    UINT32				nData;
};

class Time
{
  public:
                        Time();
                        Time(
                            unsigned 			i_nHour,
                            unsigned 			i_nMinutes,
                            unsigned 			i_nSeconds = 0,
                            unsigned            i_nSeconds100 = 0 );

    unsigned			Hour() const 			{ return nData >> 24; }
    unsigned			Minutes() const 		{ return (nData & 0x00FF0000) >> 16; }
    unsigned			Seconds() const         { return (nData & 0x0000FF00) >> 8; }
    unsigned			Seconds100() const      { return nData & 0x000000FF; }

    static const Time &	Null_();

  private:
    UINT32				nData;
};


}   // namespace csv




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
