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

#ifndef ARY_INFO_CI_TEXT_HXX
#define ARY_INFO_CI_TEXT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace info
{

class DocuToken;


class DocuText
{
  public:
    typedef std::vector< DocuToken * >  TokenList;

                        DocuText();
                        ~DocuText();

    void                Set_HtmlUse(
                            bool                i_bUseIt )
                                                { bUsesHtml = i_bUseIt; }
    void                Add_Token(
                            DYN DocuToken &     let_drToken )
                                                { aTokens.push_back(&let_drToken); }
    const TokenList &   Tokens() const          { return aTokens; }
    bool                IsNoHtml() const        { return NOT bUsesHtml; }
    bool                IsEmpty() const         { return aTokens.empty(); }

  private:
    TokenList           aTokens;
    bool                bUsesHtml;
};






// IMPLEMENTATION


}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
