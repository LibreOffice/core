/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: semnode.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:18:34 $
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

#ifndef ADC_SEMNODE_HXX
#define ADC_SEMNODE_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/qualiname.hxx>
// #include <udm/ref.hxx>


namespace ary
{
    class QualifiedName;
    class Repository;

namespace idl
{
    class Gate;
    class Module;
}   // namespace idl
}   // namespace ary


namespace csi
{
namespace uidl
{


class Struct;
class Token;


/** is an implementation class for UnoIDL_PE s
*/
class SemanticNode : private TokenProcessing_Types
{
  public:
                        SemanticNode();
    void                EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::idl::Gate &    io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~SemanticNode();

    void                SetTokenResult(
                            E_TokenDone         i_eDone,
                            E_EnvStackAction    i_eWhat2DoWithEnvStack,
                            UnoIDL_PE *         i_pParseEnv2Push = 0 );
    UnoIDL_PE *         Parent() const          { return pParentPE; }
    ary::idl::Gate &    AryGate() const         { return *pAryGate; }
    TokenProcessing_Result &
                        TokenResult() const     { return *pTokenResult; }

  private:
    // DATA
    UnoIDL_PE *         pParentPE;
    ary::idl::Gate *    pAryGate;
    TokenProcessing_Result *
                        pTokenResult;
};


/*
class Trying_PE
{
  public:
   virtual              ~Trying_PE() {}

  protected:
                        Trying_PE();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    void                StartTry(
                            UnoIDL_PE &         i_rFirstTry );
    void                Add2Try(
                            UnoIDL_PE &         i_rTry );
    bool                AmITrying() const;
    UnoIDL_PE *         NextTry() const;
    void                FinishTry();

  private:
    std::vector<UnoIDL_PE*>
                        aTryableSubEnvironments;
    uintt               nTryCounter;
};

*/


// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif

