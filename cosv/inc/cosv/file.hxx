/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: file.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:55:05 $
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


