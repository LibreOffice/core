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

#ifndef ARY_INFO_CI_ATTAG_HXX
#define ARY_INFO_CI_ATTAG_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS



namespace ary
{
namespace info
{

class DocuText;
class DocuDisplay;

class AtTag
{
  public:
    virtual             ~AtTag() {}

    virtual bool        Add_SpecialMeaningToken(    /// @return false, if token was not spüecial.
                            const char *        i_sText,
                            intt                i_nNr ) = 0;
    virtual void        Add_Token(
                            const char *        i_sText );
    virtual void        Add_PotentialLink(
                            const char *        i_sText,
                            bool                i_bIsGlobal,
                            bool                i_bIsFunction );
    virtual void        Add_Whitespace(
                            UINT8               i_nLength );
    virtual void        Add_Eol();

    virtual UINT8       NrOfSpecialMeaningTokens() const = 0;
    virtual AtTag *     GetFollower() = 0;

    void                StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    const DocuText &    CText() const;

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;   // later becoming abstract

    virtual DocuText *  Text() = 0;
};



// IMPLEMENTATION

inline void
AtTag::StoreAt( DocuDisplay & o_rDisplay ) const
    { do_StoreAt(o_rDisplay); }
inline const DocuText &
AtTag::CText() const
    { DocuText * ret = const_cast< AtTag* >(this)->Text();
      csv_assert( ret != 0 );
      return *ret;
    }


}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
