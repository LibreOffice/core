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

#ifndef DSAPI_CX_DOCU2_HXX
#define DSAPI_CX_DOCU2_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcont2.hxx>
    // COMPONENTS
    // PARAMETERS
#include <s2_dsapi/tk_xml.hxx>

namespace csi
{
namespace dsapi
{

class Token_Receiver;


/**
@descr
*/

class Cx_Base : public ::TkpContext
{
  public:
    virtual bool		PassNewToken();
    virtual TkpContext &
                        FollowUpContext();
  protected:
    // LIFECYCLE
                        Cx_Base(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	rReceiver(o_rReceiver),
                                                    pFollowUpContext(&i_rFollowUpContext)
                                                    // pNewToken
                                                { }
  protected:
    void				SetToken(
                            DYN Token *			let_dpToken )
                                                { pNewToken = let_dpToken; }
    void                Handle_DocuSyntaxError(
                            CharacterSource &   io_rText );

  private:
    // DATA
    Token_Receiver &	rReceiver;
    TkpContext *		pFollowUpContext;
    Dyn<Token>			pNewToken;
};


class Cx_EoHtml : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_EoHtml(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //	OPERATIONS
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    void				SetIfIsStartOfParagraph(
                            bool				i_bNextTokenProperty )
                                                { bToken_IsStartOfParagraph = i_bNextTokenProperty; }

  private:
    bool				bToken_IsStartOfParagraph;
};

class Cx_EoXmlConst : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_EoXmlConst(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //	OPERATIONS
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    void				SetTokenId(
                            lux::Enum< Tok_XmlConst::E_TokenId >
                                                i_eTokenId )
                                                { eTokenId = i_eTokenId; }
  private:
    Tok_XmlConst::EV_TokenId 
                        eTokenId;
};

class Cx_EoXmlLink_BeginTag : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_EoXmlLink_BeginTag(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //	OPERATIONS
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    void				SetTokenId(
                            Tok_XmlLink_BeginTag::EV_TokenId
                                                i_eTokenId )
                                                { eTokenId = i_eTokenId; }
  private:
    Tok_XmlLink_BeginTag::EV_TokenId
                        eTokenId;
};

class Cx_EoXmlLink_EndTag : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_EoXmlLink_EndTag(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //	OPERATIONS
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    void				SetTokenId(
                            Tok_XmlLink_EndTag::EV_TokenId
                                                i_eTokenId )
                                                { eTokenId = i_eTokenId; }
  private:
    Tok_XmlLink_EndTag::E_TokenId
                        eTokenId;
};

class Cx_EoXmlFormat_BeginTag : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_EoXmlFormat_BeginTag(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //	OPERATIONS
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    void				SetTokenId(
                            lux::Enum< Tok_XmlFormat_BeginTag::E_TokenId >
                                                i_eTokenId )
                                                { eTokenId = i_eTokenId; }
  private:
    lux::Enum< Tok_XmlFormat_BeginTag::E_TokenId >
                        eTokenId;
};

class Cx_EoXmlFormat_EndTag : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_EoXmlFormat_EndTag(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //	OPERATIONS
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    void				SetTokenId(
                            lux::Enum< Tok_XmlFormat_EndTag::E_TokenId >
                                                i_eTokenId )
                                                { eTokenId = i_eTokenId; }
  private:
    lux::Enum< Tok_XmlFormat_EndTag::E_TokenId >
                        eTokenId;
};

class Cx_CheckStar : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_CheckStar(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext),
                                                    bIsEnd(false), bEndTokenFound(false)
                                                 { }
    void				Set_End_FolloUpContext(
                            TkpContext &		i_rEnd_FollowUpContext )
                                                { pEnd_FollowUpContext = &i_rEnd_FollowUpContext; }

    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    void				SetIsEnd(
                            bool 				i_bIsEnd  )
                                                { bIsEnd = i_bIsEnd; }
    virtual TkpContext &
                        FollowUpContext();
  private:
    TkpContext *		pEnd_FollowUpContext;
    bool				bIsEnd;
    bool				bEndTokenFound;
};


}   // namespace dsapi
}   // namespace csi



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
