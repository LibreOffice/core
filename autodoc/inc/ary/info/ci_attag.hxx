/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    virtual				~AtTag() {}

    void                Set_HtmlUseInDocuText(
                            bool                i_bUseIt );
    virtual bool	  	Add_SpecialMeaningToken(    /// @return false, if token was not spüecial.
                            const char *		i_sText,
                            intt				i_nNr ) = 0;
    virtual void		Add_Token(
                            const char *		i_sText );
    virtual void		Add_PotentialLink(
                            const char *		i_sText,
                            bool                i_bIsGlobal,
                            bool                i_bIsFunction );
    virtual void		Add_Whitespace(
                            UINT8               i_nLength );
    virtual void		Add_Eol();

    virtual UINT8		NrOfSpecialMeaningTokens() const = 0;
    virtual AtTag *		GetFollower() = 0;

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
