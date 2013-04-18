/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
const UINT32    CFM_CREATE  =
    static_cast< UINT32 >(rwReadWrite) | static_cast< UINT32 >(omCreate);
const UINT32    CFM_READ    =
    static_cast< UINT32 >(rwRead) | static_cast< UINT32 >(omCreateNot) |
    static_cast< UINT32 >(shmShareRead);



class OpenClose
{
  public:
    virtual ~OpenClose() {}

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
