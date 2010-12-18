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

#ifndef ADC_CPP_FEVNTHDL_HXX
#define ADC_CPP_FEVNTHDL_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace loc
{
    class File;
}
}




namespace cpp
{


/** This is an interface, which accepts the file scope events that may
    be important for parsing. It is implementation-dependant, where to
    put or what to do with them.
*/
class FileScope_EventHandler
{
  public:
    // LIFECYCLE
    virtual             ~FileScope_EventHandler() {}

    // OPERATIONS
    void                SetCurFile(
                            ary::loc::File &    io_rCurFile );
    void                Event_IncrLineCount();
    void                Event_SwBracketOpen();
    void                Event_SwBracketClose();
    void                Event_Semicolon();

  private:
    virtual void        do_SetCurFile(
                            ary::loc::File &    io_rCurFile ) = 0;
    virtual void        do_Event_IncrLineCount() = 0;
    virtual void        do_Event_SwBracketOpen() = 0;
    virtual void        do_Event_SwBracketClose() = 0;
    virtual void        do_Event_Semicolon() = 0;
};


// IMPLEMENTATION

inline void
FileScope_EventHandler::SetCurFile( ary::loc::File & io_rCurFile )
    { do_SetCurFile(io_rCurFile); }
inline void
FileScope_EventHandler::Event_IncrLineCount()
    { do_Event_IncrLineCount(); }
inline void
FileScope_EventHandler::Event_SwBracketOpen()
    { do_Event_SwBracketOpen(); }
inline void
FileScope_EventHandler::Event_SwBracketClose()
    { do_Event_SwBracketClose(); }
inline void
FileScope_EventHandler::Event_Semicolon()
    { do_Event_Semicolon(); }




}   // namespace cpp
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
