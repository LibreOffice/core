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

#ifndef ADC_ADC_MSG_HXX
#define ADC_ADC_MSG_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
namespace csv
{
    class File;
}


namespace autodoc
{


/** Gathers, sorts and displays (mainly diagnostic) messages to the
    user of Autodoc.
*/
class Messages
{
  public:
    // LIFECYCLE
                        Messages();
                        ~Messages();
    // OPERATIONS
    void                WriteFile(
                            const String &      i_sOutputFilePath);
    // INQUIRY

    // ACCESS
    void                Out_MissingDoc(
                            const String &      i_sEntity,
                            const String &      i_sFile,
                            uintt               i_nLine);
    void                Out_ParseError(
                            const String &      i_sFile,
                            uintt               i_nLine);
    void                Out_InvalidConstSymbol(
                            const String &      i_sText,
                            const String &      i_sFile,
                            uintt               i_nLine);
    void                Out_UnresolvedLink(
                            const String &      i_sLinkText,
                            const String &      i_sFile,
                            uintt               i_nLine);
    void                Out_TypeVsMemberMisuse(
                            const String &      i_sLinkText,
                            const String &      i_sFile,
                            uintt               i_nLine);

    static Messages &   The_();

  private:
    struct Location
    {
        String              sFile;
        uintt               nLine;

                            Location(
                                const String &      i_file,
                                uintt               i_line)
                                :   sFile(i_file),
                                    nLine(i_line)   {}
        bool                operator<(
                                const Location &    i_other) const
                                { int cmp = csv::compare(sFile,i_other.sFile);
                                  return cmp < 0
                                            ?   true
                                            :   cmp > 0
                                                    ?   false
                                                    :   nLine < i_other.nLine;
                                }
    };

    typedef std::map<Location,String>   MessageMap;

    // Locals
    void                AddValue(
                            MessageMap &        o_dest,
                            const String &      i_sText,
                            const String &      i_sFile,
                            uintt               i_nLine );
    void                WriteParagraph(
                            csv::File &         o_out,
                            const MessageMap &  i_source,
                            const String &      i_title );

    // DATA
    MessageMap          aMissingDocs;
    MessageMap          aParseErrors;
    MessageMap          aInvalidConstSymbols;
    MessageMap          aUnresolvedLinks;
    MessageMap          aTypeVsMemberMisuses;
};



// IMPLEMENTATION


}   // namespace autodoc

inline autodoc::Messages &
TheMessages()
{
    return autodoc::Messages::The_();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
