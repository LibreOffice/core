/*************************************************************************
 *
 *  $RCSfile: ci_attag.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    virtual             ~AtTag() {}

    void                Set_HtmlUseInDocuText(
                            bool                i_bUseIt );
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

