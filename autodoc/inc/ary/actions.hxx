/*************************************************************************
 *
 *  $RCSfile: actions.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:10:09 $
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

#ifndef ARY_ACTIONS_HXX
#define ARY_ACTIONS_HXX

//  VERSION:            Autodoc 2.2


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace n22
{
    class RepositoryCenter;
}


/*
enum E_Action
{
    action_Parse,
    action_SecondaryProductions,
    action_Save,
    action_Load,
    action_ReadyForRead
};
*/


/** @resp
    Performs such commands on the repository, which refer to
    large parts of it.

    @collab ::ary::Repository
            and its components and derivates.
    @descr
    This class works in kind of double dispatch way:

    // Client code:
    Command_Xy  aMyCommand;
    ary::Repository::The_().PerformCommand( aMyCommand );

    // Repository_Implementation::PerformCommand() code:
    aMyCommand.Run(*this);

    // Command_Xy::Run(Repository_Implementation & rRepository) code:
    rRepository.Run_Command_Xy(*this);
*/
class Command
{
  public:
    virtual             ~Command() {}

    void                Run(
                            n22::RepositoryCenter &
                                                io_rReposy );
  private:
    virtual void        do_Run(
                            n22::RepositoryCenter &
                                                io_rReposy ) = 0;
};


// IMPLEMENTATION

inline void
Command::Run(n22::RepositoryCenter & io_rReposy)
    { do_Run(io_rReposy); }


} // namespace ary


#endif



