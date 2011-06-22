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

#ifndef ADC_DOCU_PE_HXX
#define ADC_DOCU_PE_HXX



// USED SERVICES
    // BASE CLASSES
#include <adoc/tokintpr.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace doc
{
    class OldCppDocu;
}
namespace info
{
    class AtTag;
}
}



namespace adoc
{


class Adoc_PE : public TokenInterpreter
{
  public:
                        Adoc_PE();
                        ~Adoc_PE();

    virtual void        Hdl_at_std(
                            const Tok_at_std &  i_rTok );
    virtual void        Hdl_at_base(
                            const Tok_at_base & i_rTok );
    virtual void        Hdl_at_exception(
                            const Tok_at_exception &
                                                i_rTok );
    virtual void        Hdl_at_impl(
                            const Tok_at_impl & i_rTok );
    virtual void        Hdl_at_key(
                            const Tok_at_key &  i_rTok );
    virtual void        Hdl_at_param(
                            const Tok_at_param &
                                                i_rTok );
    virtual void        Hdl_at_see(
                            const Tok_at_see &  i_rTok );
    virtual void        Hdl_at_template(
                            const Tok_at_template &
                                                i_rTok );
    virtual void        Hdl_at_interface(
                            const Tok_at_interface &
                                                i_rTok );
    virtual void        Hdl_at_internal(
                            const Tok_at_internal &
                                                i_rTok );
    virtual void        Hdl_at_obsolete(
                            const Tok_at_obsolete &
                                                i_rTok );
    virtual void        Hdl_at_module(
                            const Tok_at_module &
                                                i_rTok );
    virtual void        Hdl_at_file(
                            const Tok_at_file & i_rTok );
    virtual void        Hdl_at_gloss(
                            const Tok_at_gloss &
                                                i_rTok );
    virtual void        Hdl_at_global(
                            const Tok_at_global &
                                                i_rTok );
    virtual void        Hdl_at_include(
                            const Tok_at_include &
                                                i_rTok );
    virtual void        Hdl_at_label(
                            const Tok_at_label &
                                                i_rTok );
    virtual void        Hdl_at_since(
                            const Tok_at_since &
                                                i_rTok );
    virtual void        Hdl_at_HTML(
                            const Tok_at_HTML &
                                                i_rTok );
    virtual void        Hdl_at_NOHTML(
                            const Tok_at_NOHTML &
                                                i_rTok );

    virtual void        Hdl_DocWord(
                            const Tok_DocWord & i_rTok );

    virtual void        Hdl_Whitespace(
                            const Tok_Whitespace &
                                                i_rTok );
    virtual void        Hdl_LineStart(
                            const Tok_LineStart &
                                                i_rTok );
    virtual void        Hdl_Eol(
                            const Tok_Eol &     i_rTok );

    virtual void        Hdl_EoDocu(
                            const Tok_EoDocu &  i_rTok );


    DYN ary::doc::OldCppDocu *
                        ReleaseJustParsedDocu();

    bool                IsComplete() const;

  private:
    void                InstallAtTag(
                            DYN ary::info::AtTag *
                                                let_dpTag,
                            bool                i_bImplicit = false );  /// True for implicit @short and @descr.
    ary::doc::OldCppDocu &
                        CurDocu();
    ary::info::AtTag &  CurAtTag();
    bool                UsesHtmlInDocuText();

    void                RenameCurShortTag();
    void                FinishCurShortTag();


    // DATA
    enum E_TagState
    {
        ts_new,
        ts_std
    };
    enum E_DocuState
    {
        ds_wait_for_short = 0,
        ds_in_short,
        ds_1newline_after_short,
        ds_in_descr,
        ds_std
    };

    Dyn<ary::doc::OldCppDocu>
                        pCurDocu;
    ary::info::AtTag *  pCurAtTag;
    uintt               nLineCountInDocu;
    UINT8               nCurSpecialMeaningTokens;
    UINT8               nCurSubtractFromLineStart;
    E_TagState          eCurTagState;
    E_DocuState         eDocuState;
    bool                bIsComplete;
    bool                bUsesHtmlInDocuText;
};


// IMPLEMENTATION
inline bool
Adoc_PE::IsComplete() const
{
    return bIsComplete;
}




}   // namespace adoc
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
