/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
