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



#ifndef CSV_COMMANDLINE_HXX
#define CSV_COMMANDLINE_HXX

#include <cosv/string.hxx>




namespace csv
{

/** Does the standards in handling command line parameters.

    This class provides a default behaviour this way:

    *   Each option can have several forms

            like: "-f" and "--file"

        which are identified by having the same id.
        The option id is used when calling ->do_HandleOption().

    *   For each recognized option together with its parameters
        ->do_HandleOption() is called.

    *   For the first unrecognized argument ->do_HandleFreeArgument() is
        called.
        After the first unrecognized argument, ->do_HandleFreeArgument()
        is called for all remaining arguments.

    @howtoderive
    -   Overwrite ->do_HandleOption() to act on all known options.
        Overwrite ->do_HandleFreeArgument() to act on additional
        arguments not connected to an option.
*/
class CommandLine
{
  public:
  // LIFECYCLE
    virtual             ~CommandLine() {}

  // OPERATIONS
    bool                Interpret(
                            int                 argc,
                            char *              argv[] );
  // INQUIRY
    const StringVector &
                        Arguments() const;
    bool                IsOk() const;

  protected:
                        CommandLine();
    void                Add_Option(
                            intt                i_id,
                            String              i_text );
    void                Set_Error();

  private:
    // public for use by struct commandline.cxx-anonymous::FindOptionByText;
    struct OptionDescription
    {
        intt                nId;
        String              sText;

                            OptionDescription(
                                intt                i_id,
                                String              i_text );
    }; private:

    struct FindOptionByText;

    typedef std::vector<OptionDescription>              OptionList;
    typedef std::vector<StringVector::const_iterator>   StringCIteratorList;
    typedef std::vector<intt>                           OptionIdList;

    // Locals
    void                Get_Arguments(
                            int                 argc,
                            char *              argv[] );
    intt                Find_Option(
                            const String &      i_text ) const;
    bool                Store_Argument(
                            const String &      i_arg );
    void                Find_OptionPoints();
    void                Handle_FreeArguments(
                            StringVector::const_iterator
                                                i_begin,
                            StringVector::const_iterator
                                                i_end );

    // Helpers for options included via file
    bool                Try2Include_Options(
                            const String &      i_optionsFile );
    bool                Include_Options(
                            const String &      i_optionsFile );
    bool                Load_Options(
                            StreamStr &         o_text,
                            const String &      i_optionsFile );

    /** Handles an option found in the command line.
        Needs to be overwritten.

        @return
        The first argument within the range
        i_next_argument .. i_comandLine_end that does not belong as a
        parameter to the handled option.
    */
    virtual StringVector::const_iterator
                        do_HandleOption(
                            intt                i_id,
                            StringVector::const_iterator
                                                i_paramsBegin,
                            StringVector::const_iterator
                                                i_paramsEnd ) = 0;
    /** Handles arguments on the command line that do not belong to
        an option.
    */
    virtual void        do_HandleFreeArgument(
                            const String &      i_argument ) = 0;
    // DATA
    OptionList          aOptions;

    /// Used during and after ->GetArguments()
    StringVector        aCommandLine;
    StringCIteratorList aOptionPoints;
    OptionIdList        aOptionIds;
    bool                bIsOk;
};


inline const StringVector &
CommandLine::Arguments() const
{
    return aCommandLine;
}

inline bool
CommandLine::IsOk() const
{
    return bIsOk;
}

inline void
CommandLine::Set_Error()
{
    bIsOk = false;
}




}   // namespace csv
#endif
