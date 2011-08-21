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

#ifndef ADC_CPP_CPP_PE_HXX
#define ADC_CPP_CPP_PE_HXX



// USED SERVICES
    // BASE CLASSES
#include <semantic/parseenv.hxx>
#include "tokintpr.hxx"
    // COMPONENTS
#include "pev.hxx"
    // PARAMETERS
#include <ary/cpp/c_types4cpp.hxx>


namespace cpp {

class Cpp_PE : public ::ParseEnvironment,
               public TokenInterpreter
{
  public:
    typedef cpp::PeEnvironment  EnvData;

    void                SetTokenResult(
                            E_TokenDone         i_eDone,
                            E_EnvStackAction    i_eWhat2DoWithEnvStack,
                            ParseEnvironment *  i_pParseEnv2Push = 0 );

    virtual Cpp_PE *    Handle_ChildFailure();  // Defaulted to 0.

  protected:
                        Cpp_PE(
                            Cpp_PE *            io_pParent );
                        Cpp_PE(
                            EnvData &           i_rEnv );

    EnvData &           Env() const;

    void                StdHandlingOfSyntaxError(
                            const char *        i_sText );

  private:
    // DATA
    EnvData &           rMyEnv;
};

inline  Cpp_PE::EnvData &
Cpp_PE::Env() const
    { return rMyEnv; }

}   // namespace cpp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
