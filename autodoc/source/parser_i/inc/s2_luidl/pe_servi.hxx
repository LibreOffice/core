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
    virtual				~PE_Service();

    virtual void	 	EstablishContacts(
                            UnoIDL_PE *			io_pParentPE,
                            ary::Repository &	io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual void	  	ProcessToken(
                            const Token &		i_rToken );

    virtual void		Process_MetaType(
                            const TokMetaType &	i_rToken );
    virtual void		Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void		Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void		Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void		Process_Needs();
    virtual void		Process_Observes();
    virtual void		Process_Default();

  private:
    void				On_Default();

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

    virtual void		InitData();
    virtual void		TransferData();
    virtual void		ReceiveData();
    virtual UnoIDL_PE &	MyPE();

    void				StartProperty();


    // DATA
    E_State				eState;
    String              sData_Name;
    bool				bIsPreDeclaration;
    ary::idl::Service * pCurService;
    ary::idl::SglIfcService *
                        pCurSiService;
    ary::idl::Ce_id     nCurService;  // Needed for PE_Attribute.

    Dyn<PE_Property>	pPE_Property;
    ary::idl::Ce_id		nCurParsed_Property;

    Dyn<PE_Type>		pPE_Type;
    ary::idl::Type_id	nCurParsed_Type;

    Dyn<PE_Function>	pPE_Constructor;

    bool				bOptionalMember;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
