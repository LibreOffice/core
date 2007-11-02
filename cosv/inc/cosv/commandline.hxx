/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commandline.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:38:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
