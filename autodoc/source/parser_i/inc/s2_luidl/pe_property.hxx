/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_property.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:16:34 $
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

#ifndef ADC_UIDL_PE_PROPERTY_HXX
#define ADC_UIDL_PE_PROPERTY_HXX



// USED SERVICES
    // BASE CLASSES

#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
#include <ary/idl/i_property.hxx>
    // PARAMETERS
#include <ary/idl/i_gate.hxx>


namespace csi
{
namespace uidl
{


class PE_Variable;

class PE_Property : public UnoIDL_PE,
                    public ParseEnvState
{
  public:
    typedef ary::idl::Ce_id                     Ce_id;
    typedef ary::idl::Type_id                   Type_id;
    typedef ary::idl::Property::Stereotypes     Stereotypes;


                        PE_Property(
                            const Ce_id &       i_rCurOwner );

    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &
                                                io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual             ~PE_Property();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_Default();

    void                PresetOptional()        { bIsOptional = true; }
    void                PresetStereotypes(
                            Stereotypes::E_Flags
                                                i_eFlag )
                                                { aStereotypes.Set_Flag(i_eFlag); }
  private:
    enum E_State
    {
        e_none,
        e_start,
        expect_variable,
        in_variable
    };

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    // DATA
    E_State             eState;
    const Ce_id *       pCurOwner;

    Dyn<PE_Variable>    pPE_Variable;

        // object-data
    Type_id             nCurParsedType;
    String              sCurParsedName;
    bool                bIsOptional;
    Stereotypes         aStereotypes;
};


}   // namespace uidl
}   // namespace csi


#endif

