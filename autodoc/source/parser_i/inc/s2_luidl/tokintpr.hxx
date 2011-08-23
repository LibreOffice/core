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

#ifndef ADC_LUIDL_TOKINTPR_HXX
#define ADC_LUIDL_TOKINTPR_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace uidl
{


class TokIdentifier;
class TokPunctuation;
class TokBuiltInType;
class TokTypeModifier;
class TokMetaType;
class TokStereotype;
class TokParameterHandling;
class TokAssignment;
class Tok_Documentation;


class TokenInterpreter
{
  public:
    virtual				~TokenInterpreter() {}

    virtual void		Process_Identifier(
                            const TokIdentifier &
                                                i_rToken ) = 0;
    virtual void		Process_NameSeparator() = 0;      // ::
    virtual void		Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken ) = 0;
    virtual void		Process_BuiltInType(
                            const TokBuiltInType &
                                                i_rToken ) = 0;
    virtual void		Process_TypeModifier(
                            const TokTypeModifier &
                                                i_rToken ) = 0;
    virtual void		Process_MetaType(
                            const TokMetaType &	i_rToken ) = 0;
    virtual void		Process_Stereotype(
                            const TokStereotype &
                                                i_rToken ) = 0;
    virtual void		Process_ParameterHandling(
                            const TokParameterHandling &
                                                i_rToken ) = 0;
    virtual void		Process_Raises() = 0;
    virtual void		Process_Needs() = 0;
    virtual void		Process_Observes() = 0;
    virtual void		Process_Assignment(
                            const TokAssignment &
                                                i_rToken ) = 0;
    virtual void		Process_EOL() = 0;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
