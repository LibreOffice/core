/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: all_dts.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:14:38 $
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

#ifndef ARY_INFO_ALL_DTS_HXX
#define ARY_INFO_ALL_DTS_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace info
{

class DocuDisplay;

class DocuToken
{
  public:
    virtual             ~DocuToken() {}

    void                StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    bool                IsWhite() const;

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const = 0;
    virtual bool        inq_IsWhite() const = 0;
};

class DT_Text : public DocuToken
{
  public:
                        DT_Text(
                            const char *        i_sText )
                                                :   sText( i_sText ) {}

    const String  &     Text() const            { return sText; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    String              sText;
};

class DT_MaybeLink : public DocuToken
{
  public:
                        DT_MaybeLink(
                            const char *        i_sText,
                            bool                i_bIsGlobal,
                            bool                i_bIsFunction  )
                                                :   sText( i_sText ),
                                                    bIsGlobal(i_bIsGlobal),
                                                    bIsFunction(i_bIsFunction) { }

    const String  &     Text() const            { return sText; }
    bool                IsAbsolute() const      { return bIsGlobal; }
    bool                IsFunction() const      { return bIsFunction; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    String              sText;
    bool                bIsGlobal;
    bool                bIsFunction;
};

class DT_Whitespace : public DocuToken
{
  public:
                        DT_Whitespace(
                            UINT8               i_nLength )
                                                :   nLength( i_nLength ) {}
    UINT8               Length() const          { return nLength; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    UINT8               nLength;
};


class DT_Eol : public DocuToken
{
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;
};

class DT_Xml : public DocuToken
{
  public:
                        DT_Xml(
                            const char *        i_sText )
                                                :   sText( i_sText ) {}

    const String  &     Text() const            { return sText; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    String              sText;
};


// IMPLEMENTATION

inline void
DocuToken::StoreAt( DocuDisplay & o_rDisplay ) const
    { do_StoreAt(o_rDisplay); }
inline bool
DocuToken::IsWhite() const
    { return inq_IsWhite(); }



}
}

#endif

