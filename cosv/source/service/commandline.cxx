/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commandline.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:45:51 $
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

#include <precomp.h>
#include <cosv/commandline.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/file.hxx>


namespace csv
{

namespace
{

const intt C_nNoOption = -1;

const char * sIncludeOptionShort = "-A:";
const char * sIncludeOptionLong  = "--Arguments:";
const uintt nIncludeOptionShort_Length = strlen(sIncludeOptionShort);
const uintt nIncludeOptionLong_Length = strlen(sIncludeOptionLong);


/** Analyses, if an option is the one to include a file with
    further command line arguments.
*/
bool                IsIncludeOption(
                        const String &      i_option );

/** Gets the file name from an include-arguments-option.
*/
String              IncludeFile_fromIncludeOption(
                        const String &      i_option );


bool
IsIncludeOption(const String & i_option)
{
    return strncmp(i_option, sIncludeOptionShort, nIncludeOptionShort_Length) == 0
           OR
           strncmp(i_option, sIncludeOptionLong, nIncludeOptionLong_Length) == 0;
}

String
IncludeFile_fromIncludeOption(const String & i_option)
{
    if ( strncmp(i_option, sIncludeOptionShort, nIncludeOptionShort_Length)
         == 0 )
    {
        return String(i_option, nIncludeOptionShort_Length, str::maxsize);
    }
    else
    if ( strncmp(i_option, sIncludeOptionLong, nIncludeOptionLong_Length)
         == 0 )
    {
        return String(i_option, nIncludeOptionLong_Length, str::maxsize);
    }
    return String::Null_();
}


}   // end anonymous namespace




/** Local helper class for searching a possible option name in a vector of
    ->OptionDescription.
*/
struct CommandLine::
FindOptionByText
{
    bool                operator()(
                            const CommandLine::OptionDescription &
                                                i_option )
                        { return i_option.sText == sOption; }

    /// @param i_searchText [i_searchText != ""]
                        FindOptionByText(
                            const String &      i_option )
                        :   sOption(i_option)   { }
  private:
    const String        sOption;
};


typedef std::vector<StringVector::const_iterator>   StringCIteratorList;
typedef std::vector<intt>                           OptionIdList;

bool
CommandLine::Interpret( int    argc,
                        char * argv[] )
{
    Get_Arguments(argc,argv);
    csv_assert(aOptionPoints.size() == aOptionIds.size());

    StringVector::const_iterator
        itNext          = aCommandLine.begin();
        ++itNext;       // Move 1 forward from program name.
    StringVector::const_iterator
        itEnd           = aCommandLine.end();
    StringCIteratorList::const_iterator
        itOptPtsEnd     = aOptionPoints.end();

    OptionIdList::const_iterator
        itOptIds = aOptionIds.begin();
    for ( StringCIteratorList::const_iterator itOptPts = aOptionPoints.begin();
          itOptPts != itOptPtsEnd AND bIsOk;
          ++itOptPts, ++itOptIds )
    {
        // May be, there are arguments which do not belong to the last option:
        // itNext != *is
        Handle_FreeArguments(itNext, *itOptPts);

        itNext = do_HandleOption( *itOptIds,
                                  *itOptPts + 1,
                                  itOptPts+1 == itOptPtsEnd ? itEnd : *(itOptPts+1) );
        csv_assert(itNext <= itEnd);
    }   // end for (is)
    Handle_FreeArguments(itNext, itEnd);

    return bIsOk;
}

CommandLine::CommandLine()
    :   aOptions(),
        aCommandLine(),
        bIsOk(false)
{
}

void
CommandLine::Add_Option( intt                i_id,
                         String              i_text )
{
    aOptions.push_back(OptionDescription( i_id,
                                          i_text ));
}

void
CommandLine::Get_Arguments( int    argc,
                            char * argv[] )
{
    aCommandLine.erase(aCommandLine.begin(),aCommandLine.end());
    aCommandLine.reserve(argc);

    char ** pArgEnd = argv + argc;
    for ( char ** pArg = &argv[0];
          pArg != pArgEnd;
          ++pArg )
    {
        Store_Argument(*pArg);
    }   // end for
    Find_OptionPoints();
    bIsOk = true;
}

intt
CommandLine::Find_Option( const String & i_text ) const
{
    if (i_text.empty())
        return C_nNoOption;

    FindOptionByText aSearch(i_text);
    OptionList::const_iterator
        itFound = std::find_if( aOptions.begin(),
                                aOptions.end(),
                                aSearch );
    if (itFound != aOptions.end())
    {
        return (*itFound).nId;
    }
    return C_nNoOption;
}

bool
CommandLine::Store_Argument( const String & i_arg )
{
    if ( NOT IsIncludeOption(i_arg) )
    {
        aCommandLine.push_back(i_arg);
        return true;
    }

    return Try2Include_Options(i_arg);
}

void
CommandLine::Find_OptionPoints()
{
    StringVector::const_iterator    itEnd   = aCommandLine.end();
    for ( StringVector::const_iterator it = aCommandLine.begin() + 1;
          it != itEnd;
          ++it )
    {
        intt    nOption = Find_Option(*it);
        if (nOption != C_nNoOption)
        {
            aOptionPoints.push_back(it);
            aOptionIds.push_back(nOption);
        }
    }   // end for (i)
}

void
CommandLine::Handle_FreeArguments( StringVector::const_iterator i_begin,
                                   StringVector::const_iterator i_end )
{
    for ( StringVector::const_iterator it = i_begin;
          it != i_end AND bIsOk;
          ++it )
    {
        do_HandleFreeArgument(*it);
    }
}

bool
CommandLine::Try2Include_Options(const String & i_includeOption)
{
    static StringVector
        aIncludedOptionFiles_;

    const String
        aOptionFile(IncludeFile_fromIncludeOption(i_includeOption));

    // Avoid recursion deadlock 1
    if ( std::find( aIncludedOptionFiles_.begin(),
                    aIncludedOptionFiles_.end(),
                    aOptionFile )
         != aIncludedOptionFiles_.end() )
    {
        Cerr() << "\nError: Self inclusion of option file "
               << aOptionFile
               << ".\n"
               << Endl();
        return false;
    }

    // Avoid recursion deadlock 2
    aIncludedOptionFiles_.push_back(aOptionFile);

    bool ok = Include_Options(aOptionFile);

    // Avoid recursion deadlock 3
    aIncludedOptionFiles_.pop_back();

    return ok;
}

bool
CommandLine::Include_Options( const String & i_optionsFile )
{
    StreamStr
        aIncludedText(500);
    bool ok = Load_Options(aIncludedText, i_optionsFile);
    if (NOT ok)
        return false;

    StringVector
        aIncludedOptions;
    Split(aIncludedOptions, aIncludedText.c_str());

    StringVector::const_iterator itEnd = aIncludedOptions.end();
    for ( StringVector::const_iterator it = aIncludedOptions.begin();
          it != itEnd;
          ++it )
    {
        Store_Argument(*it);
    }   // end for

    return true;
}

bool
CommandLine::Load_Options( StreamStr &      o_text,
                           const String &   i_optionsFile )
{
    if (i_optionsFile.empty())
        return false;

    File
        aOptionsFile(i_optionsFile, CFM_READ);
    OpenCloseGuard
        aOFGuard(aOptionsFile);
    if (NOT aOFGuard)
    {
        Cerr() << "\nError: Options file "
               << i_optionsFile
               << " not found.\n"
               << Endl();
        return false;
    }

    StreamStr
        aLoad(aOptionsFile);
    o_text.swap(aLoad);
    return true;
}




/******************         OptionDescription       ***********************/


CommandLine::
OptionDescription::OptionDescription( intt          i_id,
                                      String        i_text )
    :   nId(i_id),
        sText(i_text)
{
}




}   // namespace csv
