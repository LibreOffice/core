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

#ifndef ADC_CPP_CX_C_SUB_HXX
#define ADC_CPP_CX_C_SUB_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
#include "cx_base.hxx"
    // COMPONENTS
    // PARAMETERS


namespace cpp {


class Context_Comment : public Cx_Base
{
  public:
                        Context_Comment(
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(&i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    void	   			SetMode_IsMultiLine(
                            bool				i_bTrue )
                                                { bCurrentModeIsMultiline = i_bTrue; }
  private:
    bool				bCurrentModeIsMultiline;
};

class Context_ConstString : public Cx_Base
{
  public:
                        Context_ConstString(
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(&i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
};

class Context_ConstChar : public Cx_Base
{
  public:
                        Context_ConstChar(
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(&i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
};

class Context_ConstNumeric : public Cx_Base
{
  public:
                        Context_ConstNumeric(
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(&i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
};

class Context_UnblockMacro : public Cx_Base
{
  public:
                        Context_UnblockMacro(
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(&i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
};



}   // namespace cpp


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
