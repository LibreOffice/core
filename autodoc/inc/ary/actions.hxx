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

#ifndef ARY_ACTIONS_HXX
#define ARY_ACTIONS_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
    class RepositoryCenter;


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



