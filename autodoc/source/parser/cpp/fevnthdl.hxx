/*************************************************************************
 *
 *  $RCSfile: fevnthdl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    namespace cpp
    {
        class ProjectGroup;
        class FileGroup;
    }
    namespace loc
    {
        class LocationRoot;
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
    void                SetCurProject(
                            ary::cpp::ProjectGroup &
                                                io_rCurProject );
    void                SetCurFile(
                            ary::cpp::FileGroup &
                                                io_rCurFile,
                            const udmstri &     i_sFileName );
    void                Event_IncrLineCount();
    void                Event_SwBracketOpen();
    void                Event_SwBracketClose();
    void                Event_Semicolon();

    // INQUIRY
    ary::cpp::ProjectGroup &
                        CurProject() const;
  private:
    virtual void        do_SetCurProject(
                            ary::cpp::ProjectGroup &
                                                io_rCurProject ) = 0;
    virtual void        do_SetCurFile(
                            ary::cpp::FileGroup &
                                                io_rCurFile,
                            const udmstri &     i_sFileName ) = 0;
    virtual void        do_Event_IncrLineCount() = 0;
    virtual void        do_Event_SwBracketOpen() = 0;
    virtual void        do_Event_SwBracketClose() = 0;
    virtual void        do_Event_Semicolon() = 0;

    virtual ary::cpp::ProjectGroup &
                        inq_CurProject() const = 0;
};


// IMPLEMENTATION

inline void
FileScope_EventHandler::SetCurProject( ary::cpp::ProjectGroup & io_rCurProject )
    { do_SetCurProject(io_rCurProject); }
inline void
FileScope_EventHandler::SetCurFile( ary::cpp::FileGroup & io_rCurFile,
                                    const udmstri &       i_sFileName )
    { do_SetCurFile(io_rCurFile,i_sFileName); }
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
inline ary::cpp::ProjectGroup &
FileScope_EventHandler::CurProject() const
    { return inq_CurProject(); }



}   // namespace cpp


#endif

