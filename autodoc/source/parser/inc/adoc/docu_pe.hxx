/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docu_pe.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:02:58 $
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
