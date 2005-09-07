/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_docu2.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:55:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();
  protected:
    // LIFECYCLE
                        Cx_Base(
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   rReceiver(o_rReceiver),
                                                    pFollowUpContext(&i_rFollowUpContext)
                                                    // pNewToken
                                                { }
  protected:
    void                SetToken(
                            DYN Token *         let_dpToken )
                                                { pNewToken = let_dpToken; }
    void                Handle_DocuSyntaxError(
                            CharacterSource &   io_rText );

  private:
    // DATA
    Token_Receiver &    rReceiver;
    TkpContext *        pFollowUpContext;
    Dyn<Token>          pNewToken;
};


class Cx_EoHtml : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_EoHtml(
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    void                SetIfIsStartOfParagraph(
                            bool                i_bNextTokenProperty )
                                                { bToken_IsStartOfParagraph = i_bNextTokenProperty; }

  private:
    bool                bToken_IsStartOfParagraph;
};

class Cx_EoXmlConst : public Cx_Base
{
  public:
    // LIFECYCLE
                        Cx_EoXmlConst(
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    void                SetTokenId(
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
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    void                SetTokenId(
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
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    void                SetTokenId(
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
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    void                SetTokenId(
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
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    void                SetTokenId(
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
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext),
                                                    bIsEnd(false), bEndTokenFound(false)
                                                 { }
    void                Set_End_FolloUpContext(
                            TkpContext &        i_rEnd_FollowUpContext )
                                                { pEnd_FollowUpContext = &i_rEnd_FollowUpContext; }

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    void                SetIsEnd(
                            bool                i_bIsEnd  )
                                                { bIsEnd = i_bIsEnd; }
    virtual TkpContext &
                        FollowUpContext();
  private:
    TkpContext *        pEnd_FollowUpContext;
    bool                bIsEnd;
    bool                bEndTokenFound;
};


}   // namespace dsapi
}   // namespace csi



#endif

