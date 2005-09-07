/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ci_attag.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:20:26 $
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

