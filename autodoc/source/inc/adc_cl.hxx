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



#ifndef ADC_ADC_CL_HXX
#define ADC_ADC_CL_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/comdline.hxx>
    // COMPONENTS
    // PARAMETERS

#include <vector>

namespace ary
{
    class Repository;
}

namespace autodoc
{
namespace command
{
    class Command;
    class CreateHtml;
    class SinceTagTransformationData;
}


/** Reads and runs an Autodoc command line.
*/
class CommandLine : public csv::CommandLine_Ifc
{
  public:
    // LIFECYCLE
                        CommandLine();
                        ~CommandLine();
    // OPERATIONS
    int                 Run() const;

    // INQUIRY
        // debugging
    bool                DebugStyle_ShowText() const;
    bool                DebugStyle_ShowStoredObjects() const;
    bool                DebugStyle_ShowTokens() const;

        // @since tags
    bool                DoesTransform_SinceTag() const;

//    /// @see command::SinceTagTransformationData::StripSinceTagValue()
//    bool                Strip_SinceTagText(
//                            String &            io_sSinceTagValue ) const;

    /// @see command::SinceTagTransformationData::DisplayOf()
    const String &      DisplayOf_SinceTagValue(
                            const String &      i_sVersionNumber ) const;

    const ::std::vector<String> & IgnoreDefines() const
                                                { return ignoreDefines; }

        // extern IDL links
    const String &      ExternRoot() const      { return sExternRoot; }
    const String &      ExternNamespace() const { return sExternNamespace; }

    bool                CppUsed() const         { return bCpp; }
    bool                IdlUsed() const         { return bIdl; }

    // ACCESS
    void                AddIgnoreDefine(
                            const String& define)
                                                { ignoreDefines.push_back(define); }
    static CommandLine &
                        Get_();
    void                Set_ExternRoot(
                            const String &      i_s )
                                                { sExternRoot = i_s; }
    void                Set_ExternNamespace(
                            const String &      i_s )
                                                { sExternNamespace = i_s; }
    ary::Repository &   TheRepository() const   { csv_assert(pReposy != 0);
                                                  return *pReposy; }
    void                Set_CppUsed()           { bCpp = true; }
    void                Set_IdlUsed()           { bIdl = true; }

  private:
    // Interface cosv::CommandLine_Ifc:
    virtual void        do_Init(
                            int                 argc,
                            char *              argv[] );
    virtual void        do_PrintUse() const;
    virtual bool        inq_CheckParameters() const;

    // Locals
    typedef StringVector::const_iterator            opt_iter;
    typedef std::vector< DYN command::Command* >    CommandList;

    void                load_IncludedCommands(
                            StringVector &      out,
                            const char *        i_filePath );

    void                do_clVerbose(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clParse(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clCreateHtml(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clSinceFile(
                            opt_iter &          it,
                            opt_iter            itEnd );

//    void                do_clCreateXml(
//                            opt_iter &          it,
//                            opt_iter            itEnd );
//    void                do_clLoad(
//                            opt_iter &          it,
//                            opt_iter            itEnd );
//    void                do_clSave(
//                            opt_iter &          it,
//                            opt_iter            itEnd );

    void                sort_Commands();

    // DATA
    uintt               nDebugStyle;
    Dyn<command::SinceTagTransformationData>
                        pSinceTransformator;

    CommandList         aCommands;
    bool                bInitOk;
    command::CreateHtml *
                        pCommand_CreateHtml;

    ::std::vector<String> ignoreDefines;

    String              sExternRoot;
    String              sExternNamespace;

    mutable Dyn<ary::Repository>
                        pReposy;
    bool                bCpp;
    bool                bIdl;

    static CommandLine *
                        pTheInstance_;
};



// IMPLEMENTATION
inline bool
CommandLine::DebugStyle_ShowText() const
    { return (nDebugStyle & 2) != 0; }
inline bool
CommandLine::DebugStyle_ShowStoredObjects() const
    { return (nDebugStyle & 4) != 0; }
inline bool
CommandLine::DebugStyle_ShowTokens() const
    { return (nDebugStyle & 1) != 0; }

}   // namespace autodoc


inline bool
DEBUG_ShowText()
    { return autodoc::CommandLine::Get_().DebugStyle_ShowText(); }
inline bool
DEBUG_ShowStoring()
    { return autodoc::CommandLine::Get_().DebugStyle_ShowStoredObjects(); }
inline bool
DEBUG_ShowTokens()
    { return autodoc::CommandLine::Get_().DebugStyle_ShowTokens(); }

#endif

