/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pestate.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:04:37 $
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

#ifndef ADC_PESTATE_HXX
#define ADC_PESTATE_HXX



// USED SERVICES
    // BASE CLASSES
#include<s2_luidl/tokintpr.hxx>
#include<s2_luidl/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace uidl
{


class TokIdentifier;
class TokBuiltInType;
class TokPunctuation;
class Tok_Documentation;

class ParseEnvState : public    TokenInterpreter,
                      virtual protected TokenProcessing_Types
{
  public:
    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_NameSeparator();
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_BuiltInType(
                            const TokBuiltInType &
                                                i_rToken );
    virtual void        Process_TypeModifier(
                            const TokTypeModifier &
                                                i_rToken );
    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_ParameterHandling(
                            const TokParameterHandling &
                                                i_rToken );
    virtual void        Process_Raises();
    virtual void        Process_Needs();
    virtual void        Process_Observes();
    virtual void        Process_Assignment(
                            const TokAssignment &
                                                i_rToken );
    virtual void        Process_EOL();

    virtual void        On_SubPE_Left();

    virtual void        Process_Default();

  protected:
                        ParseEnvState()         :   bDefaultIsError(true) {}
    void                SetDefault2Ignore()     { bDefaultIsError = false; }

  private:
    virtual UnoIDL_PE & MyPE() = 0;
    bool                bDefaultIsError;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif

