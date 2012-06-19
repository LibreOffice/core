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

#ifndef ARY_CI_ATAG2_HXX
#define ARY_CI_ATAG2_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary_i/ci_text2.hxx>
    // PARAMETERS



namespace ary
{
namespace inf
{


class DocumentationDisplay;

class AtTag2
{
  public:
    virtual             ~AtTag2() {}

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const = 0;

    const char *        Title() const           { return sTitle; }
    const DocuTex2 &    Text() const            { return aText; }
    DocuTex2 &          Access_Text()           { return aText; }

  protected:
                        AtTag2(
                            const char *        i_sTitle)
                                                :   sTitle(i_sTitle) {}
    String              sTitle;
    DocuTex2            aText;
};



// IMPLEMENTATION

}   // namespace inf
}   // namespace ary


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
