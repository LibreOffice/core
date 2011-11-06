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


