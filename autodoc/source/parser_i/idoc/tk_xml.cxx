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

#include <precomp.h>
#include <s2_dsapi/tk_xml.hxx>


// NOT FULLY DEFINED SERVICES
#include <s2_dsapi/tokintpr.hxx>

using csi::dsapi::Tok_XmlConst;
using csi::dsapi::Tok_XmlLink_Tag;
using csi::dsapi::Tok_XmlFormat_Tag;


lux::EnumValueMap			G_aTok_XmlConst_EV_TokenId_Values;
Tok_XmlConst::EV_TokenId    ev_consts_none(Tok_XmlConst::e_none,"");
Tok_XmlConst::EV_TokenId    ev_e_true(Tok_XmlConst::e_true,"true");
Tok_XmlConst::EV_TokenId    ev_e_false(Tok_XmlConst::e_false,"false");
Tok_XmlConst::EV_TokenId    ev_e_null(Tok_XmlConst::e_null,"NULL");
Tok_XmlConst::EV_TokenId    ev_e_void(Tok_XmlConst::e_void,"void");

lux::EnumValueMap	        G_aTok_XmlLink_Tag_EV_TokenId_Values;
Tok_XmlLink_Tag::EV_TokenId ev_linktags_none(Tok_XmlLink_Tag::e_none,"");
Tok_XmlLink_Tag::EV_TokenId ev_e_const(Tok_XmlLink_Tag::e_const,"const");
Tok_XmlLink_Tag::EV_TokenId ev_member(Tok_XmlLink_Tag::member,"member");
Tok_XmlLink_Tag::EV_TokenId ev_type(Tok_XmlLink_Tag::type,"type");

lux::EnumValueMap	        	G_aTok_XmlFormat_Tag_EV_TokenId_Values;
Tok_XmlFormat_Tag::EV_TokenId   ev_formattags_none(Tok_XmlFormat_Tag::e_none,"");
Tok_XmlFormat_Tag::EV_TokenId   ev_code(Tok_XmlFormat_Tag::code,"code");
Tok_XmlFormat_Tag::EV_TokenId   ev_listing(Tok_XmlFormat_Tag::listing,"listing");
Tok_XmlFormat_Tag::EV_TokenId   ev_atom(Tok_XmlFormat_Tag::atom,"code");

                                   
namespace lux
{

template<> EnumValueMap &
Tok_XmlConst::EV_TokenId::Values_()	        { return G_aTok_XmlConst_EV_TokenId_Values; }
template<> EnumValueMap &
Tok_XmlLink_Tag::EV_TokenId::Values_()	    { return G_aTok_XmlLink_Tag_EV_TokenId_Values; }
template<> EnumValueMap &
Tok_XmlFormat_Tag::EV_TokenId::Values_()	{ return G_aTok_XmlFormat_Tag_EV_TokenId_Values; }

}   // namespace lux



namespace csi
{
namespace dsapi
{

void
Tok_XmlConst::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_XmlConst(*this);
}

const char *
Tok_XmlConst::Text() const
{
    return eTag.Text();
}

void
Tok_XmlLink_BeginTag::Trigger( TokenInterpreter &	io_rInterpreter ) const
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
Tok_XmlLink_EndTag::Trigger( TokenInterpreter &	io_rInterpreter ) const
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
Tok_XmlFormat_BeginTag::Trigger( TokenInterpreter &	io_rInterpreter ) const
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
Tok_XmlFormat_EndTag::Trigger( TokenInterpreter &	io_rInterpreter ) const
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
