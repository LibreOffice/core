/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef ADC_DISPLAY_HFI_METHOD_HXX
#define ADC_DISPLAY_HFI_METHOD_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_param.hxx>
#include <ary/doc/d_oldidldocu.hxx>

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
                            bool                i_bEllipse,
                            const client &      i_ce ) const;
  private:
    void                write_Declaration(
                            const String &      i_sName,
                            type_id             i_nReturnType,
                            param_list &        i_rParams,
                            type_list &         i_rExceptions,
                            bool                i_bOneway,
                            bool                i_bEllipse ) const;
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
