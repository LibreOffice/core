/*************************************************************************
 *
 *  $RCSfile: d_token.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:13:30 $
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

#ifndef CSI_DSAPI_D_TOKEN_HXX
#define CSI_DSAPI_D_TOKEN_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary_i/ci_text2.hxx>
#include <ary_i/ci_atag2.hxx>
    // COMPONENTS
    // PARAMETERS
#include <display/uidldisp.hxx>




namespace csi
{
namespace dsapi
{

using ary::info::DocumentationDisplay;


class DT_Dsapi : public ary::info::DocuToken
{
  public:
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const  = 0;
};



class DT_TextToken : public DT_Dsapi
{
  public:
                        DT_TextToken(
                            const char *        i_sText )
                                                :   sText(i_sText) {}
                        DT_TextToken(
                            const String &      i_sText )
                                                :   sText(i_sText) {}
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
    const char *        GetText() const         { return sText; }
    const String &      GetTextStr() const      { return sText; }

    String &            Access_Text()           { return sText; }

  private:
    String              sText;
};

class DT_MLTag : public DT_Dsapi
{
  public:
    enum E_Kind
    {
        k_unknown = 0,
        k_begin,
        k_end,
        k_single
    };
};

class DT_MupType : public DT_MLTag
{
  public:
                        DT_MupType(             /// Constructor for End-Tag
                            bool                i_bEnd )    /// Must be there, but is not evaluated.
                                                :   bIsBegin(false) {}
                        DT_MupType(             /// Constructor for Begin-Tag
                            const udmstri &     i_sScope )
                                                :   sScope(i_sScope), bIsBegin(true) {}
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
    const udmstri &     Scope() const           { return sScope; }
    bool                IsBegin() const         { return bIsBegin; }

  private:
    udmstri             sScope;
    bool                bIsBegin;
};

class DT_MupMember : public DT_MLTag
{
  public:
                        DT_MupMember(           /// Constructor for End-Tag
                            bool                i_bEnd )    /// Must be there, but is not evaluated.
                                                :   bIsBegin(false) {}
                        DT_MupMember(           /// Constructor for Begin-Tag
                            const udmstri &     i_sScope )
                                                :   sScope(i_sScope), bIsBegin(true) {}
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
    const udmstri &     Scope() const           { return sScope; }
    bool                IsBegin() const         { return bIsBegin; }

  private:
    udmstri             sScope;
    bool                bIsBegin;
};

class DT_MupConst : public DT_Dsapi
{
  public:
                        DT_MupConst(
                            const char *        i_sConstText )
                                                :   sConstText(i_sConstText) {}
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
    const char *        GetText() const         { return sConstText; }

  private:
    udmstri             sConstText;             /// Without HTML.
};


class DT_Style : public DT_MLTag
{
  public:
                        DT_Style(
                            const char *        i_sPlainHtmlTag,
                            bool                i_bNewLine )
                                                : sText(i_sPlainHtmlTag), bNewLine(i_bNewLine) {}

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
    const char *        GetText() const         { return sText; }
    bool                IsStartOfNewLine() const
                                                { return bNewLine; }
  private:
    udmstri             sText;                  /// With HTML.
    E_Kind              eKind;
    bool                bNewLine;
};

class DT_EOL : public DT_Dsapi
{
  public:

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
  private:
    udmstri             sText;
};


class DT_AtTag : public ary::info::AtTag2
{
  public:
    void                AddToken(
                            DYN ary::info::DocuToken &
                                                let_drToken )
                                                {   aText.AddToken(let_drToken); }
    void                SetName(
                            const char *        i_sName )
                                                { sTitle = i_sName; }

  protected:
                        DT_AtTag(
                            const char *        i_sTitle )
                                                :   ary::info::AtTag2(i_sTitle) {}
};

class DT_StdAtTag : public DT_AtTag
{
  public:
                        DT_StdAtTag(
                            const char *        i_sTitle )
                                                :   DT_AtTag(i_sTitle) {}
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
};

class DT_SeeAlsoAtTag : public DT_AtTag
{
  public:
                        DT_SeeAlsoAtTag()       :   DT_AtTag("") {}

//  void                SetLinkText(
//                          const char *        i_sLinkText )
//                                              { sLinkText = i_sLinkText; }

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
    const udmstri &     LinkText() const        { return sTitle; }  // Missbrauch von sTitle

  private:
//  udmstri             sLinkText;
};

class DT_ParameterAtTag : public DT_AtTag
{
  public:
                        DT_ParameterAtTag()     :   DT_AtTag("") {}

    void                SetTitle(
                            const char *        i_sTitle );
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
};

class DT_SinceAtTag : public DT_AtTag
{
  public:
                        DT_SinceAtTag()     :   DT_AtTag("Since version") {}

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
};


}   // namespace dsapi
}   // namespace csi

#endif

