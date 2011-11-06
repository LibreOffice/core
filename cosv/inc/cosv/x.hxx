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



#ifndef CSV_X_HXX
#define CSV_X_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS
#include <cosv/csv_ostream.hxx>


namespace csv
{

class Exception
{
  public:
    virtual             ~Exception() {}
    virtual void        GetInfo(
                            ostream &           o_rOutputMedium ) const = 0;
};


class X_Default : public Exception
{
  public:
                        X_Default(
                            const char *        i_sMessage )
                            :   sMessage(i_sMessage) {}
    virtual void        GetInfo(                // Implemented in comfunc.cxx
                            ostream &           o_rOutputMedium ) const;
  private:
    String              sMessage;
};


}   // namespace csv



#endif



