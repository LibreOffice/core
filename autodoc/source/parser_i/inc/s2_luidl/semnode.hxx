/*************************************************************************
 *
 *  $RCSfile: semnode.hxx,v $
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

#ifndef ADC_SEMNODE_HXX
#define ADC_SEMNODE_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS
#include <csi/prl/quname2.hxx>
#include <udm/ref.hxx>

namespace ary
{
class Repository;
namespace uidl
{
class Gate;
class CeNamespace;
}   // namespace uidl
}   // namespace ary


namespace csi
{
namespace uidl
{


class Struct;
class QualifiedName;
class Token;


/** is an implementation class for UnoIDL_PE s
*/
class SemanticNode : private TokenProcessing_Types
{
  public:
                        SemanticNode();
    void                EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~SemanticNode();

    udm::IRef< Struct > GetStructRef(
                            const QuName &      i_rText,
                            ary::uidl::CeNamespace &
                                                i_rCurNamespace );
    void                SetTokenResult(
                            E_TokenDone         i_eDone,
                            E_EnvStackAction    i_eWhat2DoWithEnvStack,
                            UnoIDL_PE *         i_pParseEnv2Push = 0 );
    UnoIDL_PE *         Parent() const          { return pParentPE; }
    ary::uidl::Gate &   AryGate() const         { return *pAryGate; }

  private:
    // DATA
    UnoIDL_PE *         pParentPE;
    ary::uidl::Gate *   pAryGate;
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

