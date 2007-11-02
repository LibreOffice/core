/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_servi.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:17:07 $
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

#ifndef ADC_UIDL_PE_SERVI_HXX
#define ADC_UIDL_PE_SERVI_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
     namespace idl
    {
         class Service;
         class SglIfcService;
    }
}

namespace csi
{
namespace uidl
{

class PE_Property;
class PE_Type;
class PE_Function;


class PE_Service : public UnoIDL_PE,
                   public ParseEnvState
{
  public:
                        PE_Service();
    virtual             ~PE_Service();

    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_Needs();
    virtual void        Process_Observes();
    virtual void        Process_Default();

  private:
    void                On_Default();

    enum E_State
    {
        e_none = 0,
        need_name,
        need_curlbr_open,
        e_std,
        in_property,
        in_ifc_type,
        in_service_type,
        expect_ifc_separator,
        expect_service_separator,
        at_ignore,
        need_finish,
        need_base_interface,    /// After ":".
        need_curlbr_open_sib,   /// After base interface in single interface based service.
        e_std_sib,              /// Standard in single interface based service.
        e_STATES_MAX
    };

    virtual void        InitData();
    virtual void        TransferData();
    virtual void        ReceiveData();
    virtual UnoIDL_PE & MyPE();

    void                StartProperty();


    // DATA
    E_State             eState;
    String              sData_Name;
    bool                bIsPreDeclaration;
    ary::idl::Service * pCurService;
    ary::idl::SglIfcService *
                        pCurSiService;
    ary::idl::Ce_id     nCurService;  // Needed for PE_Attribute.

    Dyn<PE_Property>    pPE_Property;
    ary::idl::Ce_id     nCurParsed_Property;

    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nCurParsed_Type;

    Dyn<PE_Function>    pPE_Constructor;

    bool                bOptionalMember;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi


#endif

