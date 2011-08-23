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

#ifndef ADC_DSAPI_TOKINTPR_HXX
#define ADC_DSAPI_TOKINTPR_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace dsapi
{


class Tok_AtTag;
class Tok_XmlConst;
class Tok_XmlLink_BeginTag;
class Tok_XmlLink_EndTag;
class Tok_XmlFormat_BeginTag;
class Tok_XmlFormat_EndTag;
class Tok_Word;
class Tok_HtmlTag;

class TokenInterpreter
{
  public:
    virtual				~TokenInterpreter() {}

    virtual void		Process_AtTag(
                            const Tok_AtTag &	i_rToken ) = 0;
    virtual void		Process_HtmlTag(
                            const Tok_HtmlTag &	i_rToken ) = 0;
    virtual void		Process_XmlConst(
                            const Tok_XmlConst &
                                                i_rToken ) = 0;
    virtual void		Process_XmlLink_BeginTag(
                            const Tok_XmlLink_BeginTag &
                                                i_rToken ) = 0;
    virtual void		Process_XmlLink_EndTag(
                            const Tok_XmlLink_EndTag &
                                                i_rToken ) = 0;
    virtual void		Process_XmlFormat_BeginTag(
                            const Tok_XmlFormat_BeginTag &
                                                i_rToken ) = 0;
    virtual void		Process_XmlFormat_EndTag(
                            const Tok_XmlFormat_EndTag &
                                                i_rToken ) = 0;
    virtual void		Process_Word(
                            const Tok_Word &	i_rToken ) = 0;
    virtual void		Process_Comma() = 0;
    virtual void		Process_DocuEnd() = 0;
    virtual void		Process_EOL() = 0;
    virtual void		Process_White() = 0;
};



// IMPLEMENTATION


}   // namespace dsapi
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
