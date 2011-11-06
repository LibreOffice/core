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



#ifndef CSV_FILE_HXX
#define CSV_FILE_HXX

// USED SERVICES
    // BASE CLASSES
#include <cosv/bstream.hxx>
#include <cosv/openclose.hxx>
    // COMPONENTS
#include <stdio.h>
#include <cosv/string.hxx>
    // PARAMETERS
#include <cosv/persist.hxx>
#include <cosv/ploc.hxx>


class FileStrategy;


namespace csv
{


/** @task
    File is a class representing a file.
*/
class File : public bstream,
             public OpenClose,
             public ploc::Persistent
{
  public:
    // LIFECYCLE
                        File(
                            uintt           i_nMode = CFM_RW );
                        File(
                            const ::csv::ploc::Path &
                                            i_rLocation,
                            uintt           i_nMode = CFM_RW );
                        File(
                            const char *    i_sLocation,
                            uintt           in_nMode = CFM_RW );
                        File(
                            const String &  i_sLocation,
                            uintt           in_nMode = CFM_RW );
    virtual             ~File();

    // OPERATIONS
    bool                Assign(
                            ploc::Path      i_rLocation );
    bool                Assign(
                            const char *    i_sLocation );
    bool                Assign(
                            const String &  i_sLocation );
    //  INQUIRY
    uintt               Mode() const;

  private:
    enum E_LastIO
    {
        io_none = 0,
        io_read,
        io_write
    };

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
    // Interface OpenClose:
    virtual bool        do_open(
                            uintt           in_nOpenModeInfo );
    virtual void        do_close();
    virtual bool        inq_is_open() const;
    // Interface Persistent:
    virtual const ploc::Path &
                        inq_MyPath() const;
    // DATA
    ploc::Path          aPath;
    FILE *              pStream;

    uintt               nMode;              /// RWMode, OpenMode and ShareMode.
    E_LastIO            eLastIO;
};



// IMPLEMENTATION

inline uintt
File::Mode() const
    { return nMode; }


}   // namespace csv




#endif


