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



#ifndef CSV_PERSIST_HXX
#define CSV_PERSIST_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS


namespace csv
{
namespace ploc
{

class Path;


inline const char *
Delimiter()
{
#ifdef WNT
    return "\\";
#elif defined(UNX)
    return "/";
#else
#error  For using csv::ploc there has to be defined: WNT or UNX.
#endif
}



class Persistent
{
  public:
    virtual             ~Persistent() {}

    const Path &        MyPath() const;
    /// @return all paths without completing delimiter, even directories.
    const char *        StrPath() const;
    bool                Exists() const;

  protected:
                        Persistent();
    void                InvalidatePath();

  private:
    virtual const Path &
                        inq_MyPath() const = 0;
    // DATA
    mutable StreamStr   sPath;
};



// IMPLEMENTATION

inline
Persistent::Persistent()
    :   sPath(30) { }
inline const Path &
Persistent::MyPath() const
    { return inq_MyPath(); }
inline void
Persistent::InvalidatePath()
    { sPath.clear(); }



}   // namespace csv
}   // namespace ploc


#endif



