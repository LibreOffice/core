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

