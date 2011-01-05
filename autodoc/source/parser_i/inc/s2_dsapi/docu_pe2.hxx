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
    void                SetCurSinceAtTagVersion_OOo(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                SetCurSinceAtTagVersion_Number(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2SinceAtTag(
                            DYN ary::inf::DocuToken &
                                                let_drNewToken );

    bool                CheckVersionSyntax_OOo(
                            const String &      i_versionPart1 );
    bool                CheckVersionSyntax_Number(
                            const String &      i_versionPart2 );
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

