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



#ifndef ADC_ADC_CMDS_HXX
#define ADC_ADC_CMDS_HXX



// USED SERVICES
    // BASE CLASSES
#include "adc_cmd.hxx"
    // COMPONENTS
    // PARAMETERS

namespace autodoc
{
namespace command
{


/** A command that produces HTML output from the Autodoc Repository.
*/
class CreateHtml : public Command
{
  public:
                        CreateHtml();
                        ~CreateHtml();

    const String &      OutputDir() const;
    const String &      DevelopersManual_HtmlRoot() const
                                                { return sDevelopersManual_HtmlRoot; }

  private:
    // Interface Context:
    virtual void        do_Init(
                            opt_iter &          i_nCurArgsBegin,
                            opt_iter            i_nEndOfAllArgs );
    // Interface Command:
    virtual bool        do_Run() const;
    virtual int         inq_RunningRank() const;

    // Locals
    void                run_Cpp() const;
    void                run_Idl() const;

    // DATA
    String              sOutputRootDirectory;
    String              sDevelopersManual_HtmlRoot;
};

inline const String &
CreateHtml::OutputDir() const
    { return sOutputRootDirectory; }


extern const String C_opt_Verbose;

extern const String C_opt_Parse;
extern const String C_opt_Name;
extern const String C_opt_LangAll;
extern const String C_opt_ExtensionsAll;
extern const String C_opt_DevmanFile;
extern const String C_opt_SinceFile;

extern const String C_arg_Cplusplus;
extern const String C_arg_Idl;
extern const String C_arg_Java;

extern const String C_opt_Project;
//extern const String C_opt_Lang;
//extern const String C_opt_Extensions;
extern const String C_opt_SourceTree;
extern const String C_opt_SourceDir;
extern const String C_opt_SourceFile;

extern const String C_opt_CreateHtml;
extern const String C_opt_DevmanRoot;

//extern const String C_opt_CreateXml;
//extern const String C_opt_Load;
//extern const String C_opt_Save;

extern const String C_opt_IgnoreDefine;
extern const String C_opt_ExternNamespace;
extern const String C_opt_ExternRoot;


inline void
CHECKOPT( bool b, const char * miss, const String & opt )
{
    if ( NOT b )
    {
        StreamLock slMsg(100);
        throw X_CommandLine( slMsg() << "Missing " << miss <<" after " << opt << "." << c_str );
    }
}

}   // namespace command
}   // namespace autodoc


#endif
