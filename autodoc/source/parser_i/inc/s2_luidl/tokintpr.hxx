/*************************************************************************
 *
 *  $RCSfile: tokintpr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:36 $
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

