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



#ifndef CSV_PLOCROOT_HXX
#define CSV_PLOCROOT_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS
#include <cosv/csv_ostream.hxx>
#include <cosv/persist.hxx>


namespace csv
{

class bostream;


namespace ploc
{


class Root
{
  public:
    virtual             ~Root();

    static DYN Root *   Create_(
                            const char * &      o_sPathAfterRoot,
                            const char *        i_sPath,
                            const char *        i_sDelimiter = Delimiter() );

    virtual void        Get(                    /// Does not add a '\0' at the end,
                            ostream &           o_rPath ) const = 0;
    virtual void        Get(                    /// Does not add a '\0' at the end.
                            bostream &          so_rPath ) const = 0;
    virtual DYN Root *  CreateCopy() const = 0;
    virtual const char *
                        OwnDelimiter() const = 0;
};



}   // namespace ploc
}   // namespace csv



#endif



