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

