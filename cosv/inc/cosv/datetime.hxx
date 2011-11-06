/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

