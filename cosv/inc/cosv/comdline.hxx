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



#ifndef CSV_COMDLINE_HXX
#define CSV_COMDLINE_HXX
//  KORR_DEPRECATED_3.0
//      Replace by cosv/commandline.hxx.


namespace csv
{

class CommandLine_Ifc
{
  public:
    virtual             ~CommandLine_Ifc() {}

    void                Init(
                            int                 argc,
                            char *              argv[] );
    void                PrintUse() const;
    bool                CheckParameters() const;

  private:
    virtual void        do_Init(
                            int                 argc,
                            char *              argv[] ) = 0;

    virtual void        do_PrintUse() const = 0;
    virtual bool        inq_CheckParameters() const = 0;
};

inline void
CommandLine_Ifc::Init( int                 argc,
                       char *              argv[] )
    { do_Init( argc, argv ); }
inline void
CommandLine_Ifc::PrintUse() const
    { do_PrintUse(); }

}   // namespace csv



#endif

