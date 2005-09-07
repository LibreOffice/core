/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokintpr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:07:00 $
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

#ifndef ADC_LUIDL_TOKINTPR_HXX
#define ADC_LUIDL_TOKINTPR_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace uidl
{


class TokIdentifier;
class TokPunctuation;
class TokBuiltInType;
class TokTypeModifier;
class TokMetaType;
class TokStereotype;
class TokParameterHandling;
class TokAssignment;
class Tok_Documentation;


class TokenInterpreter
{
  public:
    virtual             ~TokenInterpreter() {}

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken ) = 0;
    virtual void        Process_NameSeparator() = 0;      // ::
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken ) = 0;
    virtual void        Process_BuiltInType(
                            const TokBuiltInType &
                                                i_rToken ) = 0;
    virtual void        Process_TypeModifier(
                            const TokTypeModifier &
                                                i_rToken ) = 0;
    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken ) = 0;
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken ) = 0;
    virtual void        Process_ParameterHandling(
                            const TokParameterHandling &
                                                i_rToken ) = 0;
    virtual void        Process_Raises() = 0;
    virtual void        Process_Needs() = 0;
    virtual void        Process_Observes() = 0;
    virtual void        Process_Assignment(
                            const TokAssignment &
                                                i_rToken ) = 0;
    virtual void        Process_EOL() = 0;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif
