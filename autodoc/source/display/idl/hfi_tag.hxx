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

#ifndef ADC_DISPLAY_HFI_TAG_HXX
#define ADC_DISPLAY_HFI_TAG_HXX

// BASE CLASSES
#include "hi_factory.hxx"
#include <ary_i/disdocum.hxx>
// USED SERVICES
#include <ary/idl/i_types4idl.hxx>
#include <ary_i/ci_atag2.hxx>
#include <ary_i/ci_text2.hxx>
#include <ary/doc/d_oldidldocu.hxx>

#include <toolkit/hf_docentry.hxx>

/** This class is an implementation of ary::inf::DocuText_Display
    and will be used by that interface.
*/
class HF_IdlDocuTextDisplay : public HtmlFactory_Idl,
                              public ary::inf::DocuText_Display
{
  public:
                        HF_IdlDocuTextDisplay(
                            Environment  &      io_rEnv,
                            Xml::Element *      o_pOut,
                            const ary::idl::CodeEntity &
                                                i_rScopeGivingCe );
    virtual             ~HF_IdlDocuTextDisplay();

    const ary::idl::CodeEntity &
                        ScopeGivingCe() const { return *pScopeGivingCe; }
  private:
    virtual void        Display_TextToken(
                            const csi::dsapi::DT_TextToken &
                                                i_rToken );
    virtual void        Display_White();

    virtual void        Display_MupType(
                            const csi::dsapi::DT_MupType &
                                                i_rToken );
    virtual void        Display_MupMember(
                            const csi::dsapi::DT_MupMember &
                                                i_rToken );
    virtual void        Display_MupConst(
                            const csi::dsapi::DT_MupConst &
                                                i_rToken );
    virtual void        Display_Style(
                            const csi::dsapi::DT_Style &    i_rToken );
    virtual void        Display_EOL();

    // Local
    void                StartLinkGathering(
                            const String &      i_sScope )
                                                { sLinkToken = ""; sScope = i_sScope; bGatherLink = true; }
    void                StopLinkGathering()     { bGatherLink = false; }
    /** @precond
        The scope is in sScope, the name is in sLinkToken.
    */
    void                CreateTypeLink();
    /** @precond
        The scope is in sScope, the qualified member-name is in sLinkToken.
    */
    void                CreateMemberLink();

    // DATA
    String              sScope;
    String              sLinkToken;
    bool                bGatherLink;
    const ary::idl::CodeEntity *
                        pScopeGivingCe;
};



class HF_IdlShortDocu : public HtmlFactory_Idl
{
  public:
                        HF_IdlShortDocu(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlShortDocu();

    void                Produce_byData(
                            const ary::idl::CodeEntity &
                                                i_rCe );
};



class HF_IdlTag : public HtmlFactory_Idl,
                  public ary::inf::DocuTag_Display
{
  public:
                        HF_IdlTag(
                            Environment &       io_rEnv,
                            const ary::idl::CodeEntity &
                                                i_rScopeGivingCe );
    virtual             ~HF_IdlTag();

    void                Produce_byData(
                            Xml::Element &      o_rTitle,
                            Xml::Element &      o_rText,
                            const ary::inf::AtTag2 &
                                                i_rTag ) const;
    void                Produce_byData(
                            Xml::Element &      o_rTitle,
                            Xml::Element &      o_rText,
                            const std::vector< csi::dsapi::DT_SeeAlsoAtTag* > &
                                                i_seeAlsoVector ) const;
  private:
    virtual void        Display_StdAtTag(
                            const csi::dsapi::DT_StdAtTag &
                                                i_rToken );
    virtual void        Display_SeeAlsoAtTag(
                            const csi::dsapi::DT_SeeAlsoAtTag &
                                                i_rToken );
    virtual void        Display_ParameterAtTag(
                            const csi::dsapi::DT_ParameterAtTag &
                                                i_rToken );
    virtual void        Display_SinceAtTag(
                            const csi::dsapi::DT_SinceAtTag &
                                                i_rToken );

    void                Enter_TextOut(
                            Xml::Element &      o_rText ) const;
    void                Leave_TextOut() const;
    void                PutText_Out(
                            const ary::inf::DocuTex2 &
                                                i_rText ) const;
    // DATA
    mutable Xml::Element *
                        pTitleOut;
    mutable HF_IdlDocuTextDisplay
                        aTextOut;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
