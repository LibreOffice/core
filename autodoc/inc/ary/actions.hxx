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



