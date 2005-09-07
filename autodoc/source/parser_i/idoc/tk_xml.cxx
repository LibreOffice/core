/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_xml.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:55:19 $
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

#include <precomp.h>
#include <s2_dsapi/tk_xml.hxx>


// NOT FULLY DEFINED SERVICES
#include <s2_dsapi/tokintpr.hxx>

using csi::dsapi::Tok_XmlConst;
using csi::dsapi::Tok_XmlLink_Tag;
using csi::dsapi::Tok_XmlFormat_Tag;


lux::EnumValueMap           G_aTok_XmlConst_EV_TokenId_Values;
Tok_XmlConst::EV_TokenId    ev_consts_none(Tok_XmlConst::e_none,"");
Tok_XmlConst::EV_TokenId    ev_e_true(Tok_XmlConst::e_true,"true");
Tok_XmlConst::EV_TokenId    ev_e_false(Tok_XmlConst::e_false,"false");
Tok_XmlConst::EV_TokenId    ev_e_null(Tok_XmlConst::e_null,"NULL");
Tok_XmlConst::EV_TokenId    ev_e_void(Tok_XmlConst::e_void,"void");

lux::EnumValueMap           G_aTok_XmlLink_Tag_EV_TokenId_Values;
Tok_XmlLink_Tag::EV_TokenId ev_linktags_none(Tok_XmlLink_Tag::e_none,"");
Tok_XmlLink_Tag::EV_TokenId ev_e_const(Tok_XmlLink_Tag::e_const,"const");
Tok_XmlLink_Tag::EV_TokenId ev_member(Tok_XmlLink_Tag::member,"member");
Tok_XmlLink_Tag::EV_TokenId ev_type(Tok_XmlLink_Tag::type,"type");

lux::EnumValueMap               G_aTok_XmlFormat_Tag_EV_TokenId_Values;
Tok_XmlFormat_Tag::EV_TokenId   ev_formattags_none(Tok_XmlFormat_Tag::e_none,"");
Tok_XmlFormat_Tag::EV_TokenId   ev_code(Tok_XmlFormat_Tag::code,"code");
Tok_XmlFormat_Tag::EV_TokenId   ev_listing(Tok_XmlFormat_Tag::listing,"listing");
Tok_XmlFormat_Tag::EV_TokenId   ev_atom(Tok_XmlFormat_Tag::atom,"code");


namespace lux
{

template<> EnumValueMap &
Tok_XmlConst::EV_TokenId::Values_()         { return G_aTok_XmlConst_EV_TokenId_Values; }
template<> EnumValueMap &
Tok_XmlLink_Tag::EV_TokenId::Values_()      { return G_aTok_XmlLink_Tag_EV_TokenId_Values; }
template<> EnumValueMap &
Tok_XmlFormat_Tag::EV_TokenId::Values_()    { return G_aTok_XmlFormat_Tag_EV_TokenId_Values; }

}   // namespace lux



namespace csi
{
namespace dsapi
{

void
Tok_XmlConst::Trigger( TokenInterpreter &   io_rInterpreter ) const
{
    io_rInterpreter.Process_XmlConst(*this);
}

const char *
Tok_XmlConst::Text() const
{
    return eTag.Text();
}

void
Tok_XmlLink_BeginTag::Trigger( TokenInterpreter &   io_rInterpreter ) const
{
    io_rInterpreter.Process_XmlLink_BeginTag(*this);
}

const char *
Tok_XmlLink_BeginTag::Text() const
{
    static StreamStr ret(120);
    ret.seekp(0);
    if (sScope.length() > 0)
    {
        ret << "<"
            << eTag.Text()
            << " scope=\""
            << sScope
            << "\">";
    }
    else
    {
        ret << "<"
            << eTag.Text()
            << ">";
    }
    return ret.c_str();
}

void
Tok_XmlLink_EndTag::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Process_XmlLink_EndTag(*this);
}

const char *
Tok_XmlLink_EndTag::Text() const
{
    static StreamStr ret(120);
    ret.seekp(0);
    ret << "</"
        << eTag.Text()
        << ">";
    return ret.c_str();
}

void
Tok_XmlFormat_BeginTag::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Process_XmlFormat_BeginTag(*this);
}

const char *
Tok_XmlFormat_BeginTag::Text() const
{
    static StreamStr ret(120);
    ret.seekp(0);
    ret << "<"
        << eTag.Text()
        << ">";
    return ret.c_str();
}

void
Tok_XmlFormat_EndTag::Trigger( TokenInterpreter &   io_rInterpreter ) const
{
    io_rInterpreter.Process_XmlFormat_EndTag(*this);
}

const char *
Tok_XmlFormat_EndTag::Text() const
{
    static StreamStr ret(120);
    ret.seekp(0);
    ret << "</"
        << eTag.Text()
        << ">";
    return ret.c_str();
}


}   // namespace dsapi
}   // namespace csi


