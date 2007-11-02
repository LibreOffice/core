/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ci_text2.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:20:24 $
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
    bool                IsEmpty() const;
    const String &      TextOfFirstToken() const;

    String &            Access_TextOfFirstToken();

  private:
    TokenList           aTokens;
};



// IMPLEMENTATION

}   // namespace inf
}   // namespace ary


#endif

