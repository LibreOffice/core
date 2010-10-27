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

#ifndef ADC_PESTATE_HXX
#define ADC_PESTATE_HXX



// USED SERVICES
    // BASE CLASSES
#include<s2_luidl/tokintpr.hxx>
#include<s2_luidl/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace uidl
{


class TokIdentifier;
class TokBuiltInType;
class TokPunctuation;
class Tok_Documentation;

class ParseEnvState : public    TokenInterpreter,
                      virtual protected TokenProcessing_Types
{
  public:
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
    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_ParameterHandling(
                            const TokParameterHandling &
                                                i_rToken );
    virtual void        Process_Raises();
    virtual void        Process_Needs();
    virtual void        Process_Observes();
    virtual void        Process_Assignment(
                            const TokAssignment &
                                                i_rToken );
    virtual void        Process_EOL();

    virtual void        On_SubPE_Left();

    virtual void        Process_Default();

  protected:
                        ParseEnvState()         :   bDefaultIsError(true) {}
    void                SetDefault2Ignore()     { bDefaultIsError = false; }

  private:
    virtual UnoIDL_PE & MyPE() = 0;
    bool                bDefaultIsError;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
