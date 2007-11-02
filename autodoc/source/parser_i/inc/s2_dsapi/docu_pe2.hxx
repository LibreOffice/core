/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docu_pe2.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:12:26 $
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

#ifndef ADC_DSAPI_DOCU_PE2_HXX
#define ADC_DSAPI_DOCU_PE2_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_dsapi/tokintpr.hxx>
    // COMPONENTS
    // PARAMETERS

class ParserInfo;

namespace ary
{
namespace doc
{
    class OldIdlDocu;
}

namespace inf
{
    class DocuToken;
}   // namespace info
}   // namespace ary



namespace csi
{
namespace dsapi
{


class Token;
class DT_AtTag;

class SapiDocu_PE : public TokenInterpreter
{
  public:
                        SapiDocu_PE(
                            ParserInfo &        io_rPositionInfo );
                        ~SapiDocu_PE();

    void                ProcessToken(
                            DYN csi::dsapi::Token &
                                                let_drToken );

    virtual void        Process_AtTag(
                            const Tok_AtTag &   i_rToken );
    virtual void        Process_HtmlTag(
                            const Tok_HtmlTag & i_rToken );
    virtual void        Process_XmlConst(
                            const Tok_XmlConst &
                                                i_rToken );
    virtual void        Process_XmlLink_BeginTag(
                            const Tok_XmlLink_BeginTag &
                                                i_rToken );
    virtual void        Process_XmlLink_EndTag(
                            const Tok_XmlLink_EndTag &
                                                i_rToken );
    virtual void        Process_XmlFormat_BeginTag(
                            const Tok_XmlFormat_BeginTag &
                                                i_rToken );
    virtual void        Process_XmlFormat_EndTag(
                            const Tok_XmlFormat_EndTag &
                                                i_rToken );
    virtual void        Process_Word(
                            const Tok_Word &    i_rToken );
    virtual void        Process_Comma();
    virtual void        Process_DocuEnd();
    virtual void        Process_EOL();
    virtual void        Process_White();


    DYN ary::doc::OldIdlDocu *
                            ReleaseJustParsedDocu();

    bool                IsComplete() const;

  private:
    enum E_State
    {
        e_none = 0,
        st_short,
        st_description,
        st_attags,
        st_complete
    };

    typedef void ( SapiDocu_PE::*F_TokenAdder )( DYN ary::inf::DocuToken & let_drNewToken );

    void                AddDocuToken2Void(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2Short(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2Description(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2Deprecated(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2CurAtTag(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                SetCurParameterAtTagName(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                SetCurSeeAlsoAtTagLinkText(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                SetCurSeeAlsoAtTagLinkText_2(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                SetCurSeeAlsoAtTagLinkText_3(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                SetCurSinceAtTagVersion(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2SinceAtTag(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );

    // DATA
    Dyn<ary::doc::OldIdlDocu>
                        pDocu;
    E_State             eState;
    ParserInfo *        pPositionInfo;
    F_TokenAdder        fCurTokenAddFunction;

    Dyn<DT_AtTag>       pCurAtTag;
    String              sCurDimAttribute;
    StreamStr           sCurAtSeeType_byXML;
};

}   // namespace dsapi
}   // namespace csi


// IMPLEMENTATION


#endif

