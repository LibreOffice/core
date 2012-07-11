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

#ifndef ARY_CINFO_CI_TEXT2_HXX
#define ARY_CINFO_CI_TEXT2_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace inf
{


class DocumentationDisplay;


class DocuToken
{
  public:
    virtual             ~DocuToken() {}
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const = 0;
    virtual bool        IsWhiteOnly() const = 0;
};


class DocuTex2
{
  public:
    typedef std::vector< DocuToken * >  TokenList;

                        DocuTex2();
    virtual             ~DocuTex2();

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
    void                AddToken(
                            DYN DocuToken &     let_drToken );

    const TokenList &   Tokens() const          { return aTokens; }
    bool                IsEmpty() const         { return aTokens.empty(); }
    const String &      TextOfFirstToken() const;

    String &            Access_TextOfFirstToken();

  private:
    TokenList           aTokens;
};



// IMPLEMENTATION

}   // namespace inf
}   // namespace ary


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
