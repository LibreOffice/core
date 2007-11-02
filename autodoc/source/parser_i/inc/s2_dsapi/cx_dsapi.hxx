/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_dsapi.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:12:06 $
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

#ifndef ADC_CX_DSAPI_HXX
#define ADC_CX_DSAPI_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcont2.hxx>
    // COMPONENTS
#include <cosv/tpl/dyn.hxx>
#include <tokens/tkpstam2.hxx>
    // PARAMETERS


namespace csi
{
namespace dsapi
{

class Token_Receiver;
class Token;

class Cx_EoHtml;
class Cx_EoXmlConst;
class Cx_EoXmlLink_BeginTag;
class Cx_EoXmlLink_EndTag;
class Cx_EoXmlFormat_BeginTag;
class Cx_EoXmlFormat_EndTag;
class Cx_CheckStar;

/**
@descr
*/
class Context_Docu : public TkpDocuContext,
                     private StateMachineContext
{
  public:
    //  LIFECYCLE
                        Context_Docu(
                            Token_Receiver &    o_rReceiver );
    virtual void        SetParentContext(
                            TkpContext &        io_rParentContext,
                            const char *        i_sMultiLineEndToken );

                        ~Context_Docu();
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );

    virtual bool        PassNewToken();
    virtual void        SetMode_IsMultiLine(
                            bool                i_bTrue );

    //  INQUIRY
    virtual TkpContext &
                        FollowUpContext();
  private:
    //  SERVICE FUNCTIONS
    virtual void        PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            UINT16              i_nTokenId,
                            CharacterSource &   io_rText );

    void                SetupStateMachine();

    //  DATA
    StateMachin2        aStateMachine;
    Token_Receiver *    pReceiver;

        // Contexts
    TkpContext *        pParentContext;
    String              sMultiLineEndToken;

    Dyn<Cx_EoHtml>      pCx_EoHtml;
    Dyn<Cx_EoXmlConst>  pCx_EoXmlConst;
    Dyn<Cx_EoXmlLink_BeginTag>
                        pCx_EoXmlLink_BeginTag;
    Dyn<Cx_EoXmlLink_EndTag>
                        pCx_EoXmlLink_EndTag;
    Dyn<Cx_EoXmlFormat_BeginTag>
                        pCx_EoXmlFormat_BeginTag;
    Dyn<Cx_EoXmlFormat_EndTag>
                        pCx_EoXmlFormat_EndTag;
    Dyn<Cx_CheckStar>   pCx_CheckStar;

        // Temporary data, used during ReadCharChain()
    Dyn<Token>          pNewToken;
    ::TkpContext *      pFollowUpContext;
    bool                bIsMultiline;
};


}   // namespace dsapi
}   // namespace csi


#endif

