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

#ifndef ADC_DISPLAY_HFI_METHOD_HXX
#define ADC_DISPLAY_HFI_METHOD_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_param.hxx>
#include <ary/doc/d_oldidldocu.hxx>
typedef ary::doc::OldIdlDocu CodeInfo;
#include "hfi_linklist.hxx"



namespace csi
{
    namespace idl
    {
        class Parameter;
    }
}

class HF_FunctionDeclaration;

class HF_IdlMethod : public HtmlFactory_Idl
{
  public:
    typedef ary::StdConstIterator<ary::idl::Parameter>  param_list;

                        HF_IdlMethod(
                            Environment &       io_rEnv,
                            Xml::Element &      o_cell );
    virtual             ~HF_IdlMethod();

    void                Produce_byData(
                            const String &      i_sName,
                            type_id             i_nReturnType,
                            param_list &        i_rParams,
                            type_list &         i_rExceptions,
                            bool                i_bOneway,
                            bool				i_bEllipse,
                            const client &      i_ce ) const;
  private:
    void                write_Declaration(
                            const String &      i_sName,
                            type_id             i_nReturnType,
                            param_list &        i_rParams,
                            type_list &         i_rExceptions,
                            bool                i_bOneway,
                            bool				i_bEllipse ) const;
//    void                write_Param(
//                            HF_IdlTypeText &    o_type,
//                            Xml::Element &      o_names,
//                            const ary::idl::Parameter &
//                                                i_param ) const;

    Xml::Element *      write_Param(
                            HF_FunctionDeclaration &
                                                o_decl,
                            const ary::idl::Parameter &
                                                i_param ) const;
    void                enter_ContentCell() const;
    void                leave_ContentCell() const;
};



// IMPLEMENTATION



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
