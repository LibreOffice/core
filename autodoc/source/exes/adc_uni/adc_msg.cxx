/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <precomp.h>
#include <adc_msg.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/file.hxx>
#include <cosv/tpl/tpltools.hxx>


namespace autodoc
{


Messages::Messages()
    :   aMissingDocs(),
        aParseErrors(),
        aInvalidConstSymbols(),
        aUnresolvedLinks(),
        aTypeVsMemberMisuses()
{
}

Messages::~Messages()
{
}

void
Messages::WriteFile(const String & i_sOutputFilePath)
{
    csv::File
        aOut(i_sOutputFilePath, csv::CFM_CREATE);
    aOut.open();

    // KORR_FUTURE Enable this when appropriate:
    WriteParagraph( aOut,
                    aParseErrors,
                    "Incompletely Parsed Files" );

    WriteParagraph( aOut,
                    aMissingDocs,
                    "Entities Without Documentation" );

    WriteParagraph( aOut,
                    aInvalidConstSymbols,
                    "Incorrectly Written Const Symbols" );

    WriteParagraph( aOut,
                    aUnresolvedLinks,
                    "Unresolved Links" );

    WriteParagraph( aOut,
                    aTypeVsMemberMisuses,
                    "Confusion or Misuse of <Type> vs. <Member>" );
    aOut.close();
}

void
Messages::Out_MissingDoc(   const String &      i_sEntity,
                            const String &      i_sFile,
                            uintt               i_nLine)
{
    AddValue( aMissingDocs,
              i_sEntity,
              i_sFile,
              i_nLine );
}

void
Messages::Out_ParseError(   const String &      i_sFile,
                            uintt               i_nLine)
{
    aParseErrors[Location(i_sFile,i_nLine)] = String::Null_();
}

void
Messages::Out_InvalidConstSymbol( const String &      i_sText,
                                  const String &      i_sFile,
                                  uintt               i_nLine)
{
    AddValue( aInvalidConstSymbols,
              i_sText,
              i_sFile,
              i_nLine );
}

void
Messages::Out_UnresolvedLink( const String &      i_sLinkText,
                              const String &      i_sFile,
                              uintt               i_nLine)
{
    AddValue( aUnresolvedLinks,
              i_sLinkText,
              i_sFile,
              i_nLine );
}

void
Messages::Out_TypeVsMemberMisuse( const String &      i_sLinkText,
                                  const String &      i_sFile,
                                  uintt               i_nLine)
{
    AddValue( aTypeVsMemberMisuses,
              i_sLinkText,
              i_sFile,
              i_nLine );
}

Messages &
Messages::The_()
{
    static Messages TheMessages_;
    return TheMessages_;
}

void
Messages::AddValue( MessageMap &        o_dest,
                    const String &      i_sText,
                    const String &      i_sFile,
                    uintt               i_nLine )
{
    String &
        rDest = o_dest[Location(i_sFile,i_nLine)];
    StreamLock
        slDest(2000);
    if (NOT rDest.empty())
        slDest() << rDest;
    slDest() << "\n    " << i_sText;
    rDest = slDest().c_str();
}

void
Messages::WriteParagraph( csv::File &         o_out,
                          const MessageMap &  i_source,
                          const String &      i_title )
{
    StreamStr   aLine(2000);

    // Write title of paragraph:
    aLine << i_title
          << "\n";
    o_out.write(aLine.c_str());

    aLine.seekp(0);
    for (uintt i = i_title.size(); i > 0; --i)
    {
        aLine << '-';
    }
    aLine << "\n\n";
    o_out.write(aLine.c_str());

    // Write Content
    MessageMap::const_iterator it = i_source.begin();
    MessageMap::const_iterator itEnd = i_source.end();
    for ( ; it != itEnd; ++it )
    {
        aLine.seekp(0);
        aLine << (*it).first.sFile;
        // Nobody wants to see this, if we don't know the line:
        if ((*it).first.nLine != 0)
        {
            aLine << ", line "
                  << (*it).first.nLine;
        }
        if (NOT (*it).second.empty())
        {
            aLine << ':'
                  << (*it).second
                  << "\n";
        }
        o_out.write(aLine.c_str());
    }
    o_out.write("\n\n\n");
}

}   // namespace autodoc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
