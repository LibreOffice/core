/*************************************************************************
 *
 *  $RCSfile: tk_xml.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2002-05-14 09:02:21 $
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

#ifndef DSAPI_TK_XML_HXX
#define DSAPI_TK_XML_HXX

// USED SERVICES
    // BASE CLASSES
#include <s2_dsapi/dsapitok.hxx>
    // COMPONENTS
    // PARAMETERS
#include <luxenum.hxx>


namespace csi
{
namespace dsapi
{


class Tok_XmlTag : public Token
{
  public:
};

class Tok_XmlConst : public Tok_XmlTag
{
  public:
    // TYPE
    enum  E_TokenId
    {
        e_none = 0,
        e_true = 1,
        e_false = 2,
        e_null = 3,
        e_void = 4
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;

    // Spring and Fall
                        Tok_XmlConst(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};

class Tok_XmlLink_Tag : public Tok_XmlTag
{
  public:
    // TYPE
    enum E_TokenId
    {
        e_none = 0,
        e_const = 1,
        member = 2,
        type = 3
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;
};

class Tok_XmlLink_BeginTag : public Tok_XmlLink_Tag
{
  public:
    // Spring and Fall
                        Tok_XmlLink_BeginTag(
                            EV_TokenId          i_eTag,
                            const udmstri &     i_sScope,
                            const udmstri &     i_sDim )
                                                :   eTag(i_eTag),
                                                    sScope(i_sScope),
                                                    sDim(i_sDim) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }
    const udmstri &     Scope() const           { return sScope; }
    const udmstri &     Dim() const             { return sDim; }

  private:
    // DATA
    EV_TokenId          eTag;
    udmstri             sScope;
    udmstri             sDim;
};

class Tok_XmlLink_EndTag : public Tok_XmlLink_Tag
{
  public:
    // Spring and Fall
                        Tok_XmlLink_EndTag(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};

class Tok_XmlFormat_Tag : public Tok_XmlTag
{
  public:
    // TYPE
    enum E_TokenId
    {
        e_none = 0,
        code = 1,
        listing = 2,
        atom = 3
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;
};

class Tok_XmlFormat_BeginTag : public Tok_XmlFormat_Tag
{
  public:
    // Spring and Fall
                        Tok_XmlFormat_BeginTag(
                            EV_TokenId          i_eTag,
                            const udmstri &     i_sDim )
                                                :   eTag(i_eTag),
                                                    sDim(i_sDim) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }
    const udmstri &     Dim() const             { return sDim; }

  private:
    // DATA
    EV_TokenId          eTag;
    udmstri             sDim;
};

class Tok_XmlFormat_EndTag : public Tok_XmlFormat_Tag
{
  public:
    // Spring and Fall
                        Tok_XmlFormat_EndTag(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};


}   // namespace dsapi
}   // namespace csi

#endif

