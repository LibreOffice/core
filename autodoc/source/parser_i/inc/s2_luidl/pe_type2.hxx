/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_type2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:04:04 $
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

#ifndef ADC_PE_TYPE2_HXX
#define ADC_PE_TYPE2_HXX



// USED SERVICES
    // BASE CLASSES
#include<s2_luidl/parsenv2.hxx>
#include<s2_luidl/pestate.hxx>
    // COMPONENTS
#include<ary/qualiname.hxx>
    // PARAMETERS


namespace csi
{
namespace uidl
{


class PE_Type : public UnoIDL_PE,
                public ParseEnvState
{
  public:
                         PE_Type(
                            ary::idl::Type_id & o_rResult );
    virtual             ~PE_Type();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

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
    virtual void        Process_Default();

  private:
    enum E_State
    {
        e_none = 0,
        expect_type,
        expect_quname_part,
        expect_quname_separator,
        in_template_type
    };

    void                Finish();
    PE_Type &           MyTemplateType();

    virtual void        InitData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    // DATA
    ary::idl::Type_id * pResult;

    uintt               nIsSequenceCounter;
    uintt               nSequenceDownCounter;
    bool                bIsUnsigned;
    ary::QualifiedName  sFullType;

    E_State             eState;
    String              sLastPart;

    Dyn<PE_Type>        pPE_TemplateType;   /// @attention Recursion, only initiate, if needed!
    ary::idl::Type_id   nTemplateType;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif
