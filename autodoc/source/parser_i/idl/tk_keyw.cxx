/*************************************************************************
 *
 *  $RCSfile: tk_keyw.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:42:51 $
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

#include <precomp.h>
#include <s2_luidl/tk_keyw.hxx>


// NOT FULLY DECLARED SERVICES
#include <s2_luidl/tokintpr.hxx>


using csi::uidl::TokBuiltInType;
using csi::uidl::TokTypeModifier;
using csi::uidl::TokMetaType;
using csi::uidl::TokStereotype;
using csi::uidl::TokParameterHandling;


lux::EnumValueMap           G_aTokBuiltInType_EV_TokenId_Values;
TokBuiltInType::EV_TokenId  ev_bty_none(TokBuiltInType::e_none,"");
TokBuiltInType::EV_TokenId  ev_bty_any(TokBuiltInType::bty_any,"any");
TokBuiltInType::EV_TokenId  ev_bty_boolean(TokBuiltInType::bty_boolean,"boolean");
TokBuiltInType::EV_TokenId  ev_bty_byte(TokBuiltInType::bty_byte,"byte");
TokBuiltInType::EV_TokenId  ev_bty_char(TokBuiltInType::bty_char,"char");
TokBuiltInType::EV_TokenId  ev_bty_double(TokBuiltInType::bty_double,"double");
TokBuiltInType::EV_TokenId  ev_bty_hyper(TokBuiltInType::bty_hyper,"hyper");
TokBuiltInType::EV_TokenId  ev_bty_long(TokBuiltInType::bty_long,"long");
TokBuiltInType::EV_TokenId  ev_bty_short(TokBuiltInType::bty_short,"short");
TokBuiltInType::EV_TokenId  ev_bty_string(TokBuiltInType::bty_string,"string");
TokBuiltInType::EV_TokenId  ev_bty_void(TokBuiltInType::bty_void,"void");
TokBuiltInType::EV_TokenId  ev_bty_ellipse(TokBuiltInType::bty_ellipse,"...");


lux::EnumValueMap           G_aTokTypeModifier_EV_TokenId_Values;
TokTypeModifier::EV_TokenId ev_tmod_none(TokTypeModifier::e_none,"");
TokTypeModifier::EV_TokenId ev_tmod_unsigned(TokTypeModifier::tmod_unsigned,"unsigned");
TokTypeModifier::EV_TokenId ev_tmod_sequence(TokTypeModifier::tmod_sequence,"sequence");


lux::EnumValueMap           G_aTokMetaType_EV_TokenId_Values;
TokMetaType::EV_TokenId     ev_mt_none(TokMetaType::e_none,"");
TokMetaType::EV_TokenId     ev_mt_attribute(TokMetaType::mt_attribute,"attribute");
TokMetaType::EV_TokenId     ev_mt_constants(TokMetaType::mt_constants,"constants");
TokMetaType::EV_TokenId     ev_mt_enum(TokMetaType::mt_enum,"enum");
TokMetaType::EV_TokenId     ev_mt_exception(TokMetaType::mt_exception,"exception");
TokMetaType::EV_TokenId     ev_mt_ident(TokMetaType::mt_ident,"ident");
TokMetaType::EV_TokenId     ev_mt_interface(TokMetaType::mt_interface,"interface");
TokMetaType::EV_TokenId     ev_mt_module(TokMetaType::mt_module,"module");
TokMetaType::EV_TokenId     ev_mt_property(TokMetaType::mt_property,"property");
TokMetaType::EV_TokenId     ev_mt_service(TokMetaType::mt_service,"service");
TokMetaType::EV_TokenId     ev_mt_singleton(TokMetaType::mt_singleton,"singleton");
TokMetaType::EV_TokenId     ev_mt_struct(TokMetaType::mt_struct,"struct");
TokMetaType::EV_TokenId     ev_mt_typedef(TokMetaType::mt_typedef,"typedef");
TokMetaType::EV_TokenId     ev_mt_uik(TokMetaType::mt_uik,"uik");


lux::EnumValueMap           G_aTokStereotype_EV_TokenId_Values;
TokStereotype::EV_TokenId   ev_ste_none(TokStereotype::e_none,"");
TokStereotype::EV_TokenId   ev_ste_bound(TokStereotype::ste_bound,"bound");
TokStereotype::EV_TokenId   ev_ste_const(TokStereotype::ste_const,"const");
TokStereotype::EV_TokenId   ev_ste_constrained(TokStereotype::ste_constrained,"constrained");
TokStereotype::EV_TokenId   ev_ste_maybeambiguous(TokStereotype::ste_maybeambiguous,"maybeambiguous");
TokStereotype::EV_TokenId   ev_ste_maybedefault(TokStereotype::ste_maybedefault,"maybedefault");
TokStereotype::EV_TokenId   ev_ste_maybevoid(TokStereotype::ste_maybevoid,"maybevoid");
TokStereotype::EV_TokenId   ev_ste_oneway(TokStereotype::ste_oneway,"oneway");
TokStereotype::EV_TokenId   ev_ste_optional(TokStereotype::ste_optional,"optional");
TokStereotype::EV_TokenId   ev_ste_readonly(TokStereotype::ste_readonly,"readonly");
TokStereotype::EV_TokenId   ev_ste_removable(TokStereotype::ste_removable,"removable");
TokStereotype::EV_TokenId   ev_ste_virtual(TokStereotype::ste_virtual,"virtual");
TokStereotype::EV_TokenId   ev_ste_transient(TokStereotype::ste_transient,"transient");
TokStereotype::EV_TokenId   ev_ste_published(TokStereotype::ste_published,"published");


lux::EnumValueMap                   G_aTokParameterHandling_EV_TokenId_Values;
TokParameterHandling::EV_TokenId    ev_ph_none(TokParameterHandling::e_none,"");
TokParameterHandling::EV_TokenId    ev_ph_in(TokParameterHandling::ph_in,"in");
TokParameterHandling::EV_TokenId    ev_ph_out(TokParameterHandling::ph_out,"out");
TokParameterHandling::EV_TokenId    ev_ph_inout(TokParameterHandling::ph_inout,"inout");


namespace lux
{

EnumValueMap &
TokBuiltInType::EV_TokenId::Values_()       { return G_aTokBuiltInType_EV_TokenId_Values; }
EnumValueMap &
TokTypeModifier::EV_TokenId::Values_()      { return G_aTokTypeModifier_EV_TokenId_Values; }
EnumValueMap &
TokMetaType::EV_TokenId::Values_()          { return G_aTokMetaType_EV_TokenId_Values; }
EnumValueMap &
TokStereotype::EV_TokenId::Values_()        { return G_aTokStereotype_EV_TokenId_Values; }
EnumValueMap &
TokParameterHandling::EV_TokenId::Values_() { return G_aTokParameterHandling_EV_TokenId_Values; }

}   // namespace lux



namespace csi
{
namespace uidl
{

void
TokBuiltInType::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Process_BuiltInType(*this);
}

const char *
TokBuiltInType::Text() const
{
    return eTag.Text();
}

void
TokTypeModifier::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    io_rInterpreter.Process_TypeModifier(*this);
}

const char *
TokTypeModifier::Text() const
{
    return eTag.Text();
}

void
TokMetaType::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    io_rInterpreter.Process_MetaType(*this);
}

const char *
TokMetaType::Text() const
{
    return eTag.Text();
}

void
TokStereotype::Trigger( TokenInterpreter &  io_rInterpreter ) const
{
    io_rInterpreter.Process_Stereotype(*this);
}

const char *
TokStereotype::Text() const
{
    return eTag.Text();
}

void
TokParameterHandling::Trigger( TokenInterpreter &   io_rInterpreter ) const
{
    io_rInterpreter.Process_ParameterHandling(*this);
}

const char *
TokParameterHandling::Text() const
{
    return eTag.Text();
}

void
TokRaises::Trigger( TokenInterpreter &  io_rInterpreter ) const
{
    io_rInterpreter.Process_Raises();
}

const char *
TokRaises::Text() const
{
    return "raises";
}

void
TokNeeds::Trigger( TokenInterpreter &   io_rInterpreter ) const
{
    io_rInterpreter.Process_Needs();
}

const char *
TokNeeds::Text() const
{
    return "needs";
}
void
TokObserves::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    io_rInterpreter.Process_Observes();
}

const char *
TokObserves::Text() const
{
    return "observes";
}

}   // namespace uidl
}   // namespace csi

