/*************************************************************************
 *
 *  $RCSfile: all_dts.hxx,v $
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

    const udmstri &     Text() const            { return sText; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    udmstri             sText;
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

    const udmstri &     Text() const            { return sText; }
    bool                IsAbsolute() const      { return bIsGlobal; }
    bool                IsFunction() const      { return bIsFunction; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    udmstri             sText;
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

    const udmstri &     Text() const            { return sText; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    udmstri             sText;
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

