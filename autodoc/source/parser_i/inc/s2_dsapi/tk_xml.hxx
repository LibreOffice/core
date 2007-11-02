/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_xml.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:13:18 $
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
                            const String  &     i_sScope,
                            const String  &     i_sDim )
                                                :   eTag(i_eTag),
                                                    sScope(i_sScope),
                                                    sDim(i_sDim) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }
    const String  &     Scope() const           { return sScope; }
    const String  &     Dim() const             { return sDim; }

  private:
    // DATA
    EV_TokenId          eTag;
    String              sScope;
    String              sDim;
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
                            const String  &     i_sDim )
                                                :   eTag(i_eTag),
                                                    sDim(i_sDim) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }
    const String  &     Dim() const             { return sDim; }

  private:
    // DATA
    EV_TokenId          eTag;
    String              sDim;
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

