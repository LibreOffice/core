/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: openclose.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:55:43 $
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

#ifndef CSV_OPENCLOSE_HXX
#define CSV_OPENCLOSE_HXX


namespace csv
{

// Open modes for storages:
enum E_RWMode
{
    rwDefault   = 0x0000,       //  Keep old settings. If there are none, set default.
    rwRead      = 0x0001,       //  Reads only
    rwWrite     = 0x0002,       //  Writes only
    rwReadWrite = 0x0003        //  Reads and writes.
};

enum E_OpenMode
{
    omCreateIfNecessary     = 0x0000,   // Creates a new file only, if file does not exist.
    omCreateNot             = 0x0010,   // Open fails, if file does not exist.
    omCreate                = 0x0020    // Existing file will be deleted.
};
enum E_ShareMode
{
    shmShareNot     = 0x0000,       // Allow others nothing
    shmShareRead    = 0x0004,       // Allow others to read
    shmShareAll     = 0x000C        // Allow others to read and write
};

/** Constants for filemode combinations
    These combinations are the only ones, guaranteed to be supported.
*/
const UINT32    CFM_RW      = rwReadWrite;
const UINT32    CFM_CREATE  = rwReadWrite | omCreate;
const UINT32    CFM_READ    = rwRead | omCreateNot | shmShareRead;



class OpenClose
{
  public:
    bool                open(
                            UINT32          in_nOpenModeInfo = 0 ); /// Combination of values of E_RWMode and E_ShareMode und E_OpenMode. 0 := Keep existing mode.
    void                close();

    bool                is_open() const;

  private:
    virtual bool        do_open(
                            UINT32          in_nOpenModeInfo ) = 0;
    virtual void        do_close() = 0;
    virtual bool        inq_is_open() const = 0;
};



class OpenCloseGuard
{
  public:
                        OpenCloseGuard(
                            OpenClose &         i_rOpenClose,
                            UINT32              i_nOpenModeInfo = 0 );
                        ~OpenCloseGuard();
                        operator bool() const;

  private:
    // Forbidden:
                        OpenCloseGuard(OpenCloseGuard&);
    OpenCloseGuard &    operator=(OpenCloseGuard&);

    // DATA
    OpenClose &         rOpenClose;
};


// IMPLEMENTATION

inline bool
OpenClose::open( UINT32 i_nOpenModeInfo )
    { return do_open(i_nOpenModeInfo); }
inline void
OpenClose::close()
    { do_close(); }
inline bool
OpenClose::is_open() const
    { return inq_is_open(); }

inline
OpenCloseGuard::OpenCloseGuard( OpenClose & i_rOpenClose,
                                UINT32      i_nOpenModeInfo )
    :   rOpenClose(i_rOpenClose)
    { rOpenClose.open(i_nOpenModeInfo); }
inline
OpenCloseGuard::~OpenCloseGuard()
    { if (rOpenClose.is_open()) rOpenClose.close(); }
inline
OpenCloseGuard::operator bool() const
    { return rOpenClose.is_open(); }




}   // namespace csv






#endif


