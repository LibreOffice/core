/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "common.hxx"

//flags for handleArguments()
enum class State {
    NONE, Input, Output, MergeSrc, Languages
};

namespace common {

bool handleArguments(
    int argc, char * argv[], HandledArgs& o_aHandledArgs)
{
    o_aHandledArgs = HandledArgs();
    State nState = State::NONE;

    for( int i = 1; i < argc; i++ )
    {
        if ( OString( argv[ i ] ).toAsciiUpperCase() == "-I" )
        {
            nState = State::Input; // next token specifies source file
        }
        else if ( OString( argv[ i ] ).toAsciiUpperCase() == "-O" )
        {
            nState = State::Output; // next token specifies the dest file
        }
        else if ( OString( argv[ i ] ).toAsciiUpperCase() == "-M" )
        {
            nState = State::MergeSrc; // next token specifies the merge database
            o_aHandledArgs.m_bMergeMode = true;
        }
        else if ( OString( argv[ i ] ).toAsciiUpperCase() == "-L" )
        {
            nState = State::Languages;
        }
        else if ( OString( argv[ i ] ).toAsciiUpperCase() == "-B" )
        {
            o_aHandledArgs.m_bUTF8BOM = true;
        }
        else
        {
            switch ( nState )
            {
                case State::NONE:
                {
                    return false;    // no valid command line
                }
                case State::Input:
                {
                    o_aHandledArgs.m_sInputFile = OString( argv[i] );
                }
                break;
                case State::Output:
                {
                    o_aHandledArgs.m_sOutputFile = OString( argv[i] );
                }
                break;
                case State::MergeSrc:
                {
                    o_aHandledArgs.m_sMergeSrc = OString( argv[i] );
                }
                break;
                case State::Languages:
                {
                    o_aHandledArgs.m_sLanguage = OString( argv[i] );
                }
                break;
            }
        }
    }
    if( !o_aHandledArgs.m_sInputFile.isEmpty() &&
        !o_aHandledArgs.m_sOutputFile.isEmpty() )
    {
        return true;
    }
    else
    {
        o_aHandledArgs = HandledArgs();
        return false;
    }
}

void writeUsage(const OString& rName, const OString& rFileType)
{
    std::cout
        << " Syntax: " << rName.getStr()
        << " -i FileIn -o FileOut [-m DataBase] [-l Lang] [-b]\n"
           " FileIn:   Source files (" << rFileType.getStr() << ")\n"
           " FileOut:  Destination file (*.*)\n"
           " DataBase: Mergedata (*.po)\n"
           " Lang: Restrict the handled language; one element of\n"
           " (de, en-US, ...) or all\n"
           " -b:   Add UTF-8 Byte Order Mark to FileOut(use with -m option)\n";
}

void writePoEntry(
    const OString& rExecutable, PoOfstream& rPoStream, const OString& rSourceFile,
    const OString& rResType, const OString& rGroupId, const OString& rLocalId,
    const OString& rHelpText, const OString& rText, const PoEntry::TYPE eType )
{
    try
    {
        PoEntry aPO(rSourceFile, rResType, rGroupId, rLocalId, rHelpText, rText, eType);
        rPoStream.writeEntry( aPO );
    }
    catch( PoEntry::Exception& aException )
    {
        if(aException == PoEntry::NOSOURCFILE)
        {
            std::cerr << rExecutable << " warning: no sourcefile specified for po entry\n";
        }
        else
        {
            std::cerr << rExecutable << " warning: invalid po attributes extracted from " <<  rSourceFile << "\n";
            if(aException == PoEntry::NOGROUPID)
            {
                std::cerr << "No groupID specified!\n";
                std::cerr << "String: " << rText << "\n";
            }
            else if (aException == PoEntry::NOSTRING)
            {
                std::cerr << "No string specified!\n";
                std::cerr << "GroupID: " << rGroupId << "\n";
                if( !rLocalId.isEmpty() ) std::cerr << "LocalID: " << rLocalId << "\n";
            }
            else
            {
                if (aException == PoEntry::NORESTYPE)
                {
                    std::cerr << "No resource type specified!\n";
                }
                else if (aException == PoEntry::WRONGHELPTEXT)
                {
                    std::cerr << "x-comment length is 5 characters:" << rHelpText << "\n";
                }

                std::cerr << "GroupID: " << rGroupId << "\n";
                if( !rLocalId.isEmpty() ) std::cerr << "LocalID: " << rLocalId << "\n";
                std::cerr << "String: " << rText << "\n";
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
