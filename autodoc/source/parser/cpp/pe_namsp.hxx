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

#ifndef ADC_CPP_PE_NAMSP_HXX
#define ADC_CPP_PE_NAMSP_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_pe.hxx>
    // PARAMETERS

namespace ary
{
namespace cpp
{
class Namespace;
}
}


namespace cpp
{


class PE_Namespace : public Cpp_PE
{
  public:
    enum E_State
    {
        start,
        gotName,
        expectSemicolon,        /// after namespace assignment
        size_of_states
    };
                        PE_Namespace(
                            Cpp_PE *            i_pParent );
                        ~PE_Namespace();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

    ary::cpp::Namespace *
                        Result_OpenedNamespace() const;
  private:
    void                Setup_StatusFunctions();

    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                On_start_Identifier(const char * i_sText);
    void                On_start_SwBracket_Left(const char * i_sText);
    void                On_gotName_SwBracket_Left(const char * i_sText);
    void                On_gotName_Assign(const char * i_sText);
    void                On_expectSemicolon_Semicolon(const char * i_sText);

    // DATA
    Dyn< PeStatusArray<PE_Namespace> >
                        pStati;

    String              sLocalName;
    bool                bPush;
};




}   // namespace cpp
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
