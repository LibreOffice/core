/*************************************************************************
 *
 *  $RCSfile: docu_pe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:38:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    class Documentation;

    namespace info
    {
        class CodeInfo;
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


    DYN ary::Documentation *
                        ReleaseJustParsedDocu();

    bool                IsComplete() const;

  private:
    void                InstallAtTag(
                            DYN ary::info::AtTag *
                                                let_dpTag,
                            bool                i_bImplicit = false );  /// True for implicit @short and @descr.
    ary::info::CodeInfo &
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

    Dyn<ary::info::CodeInfo>
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


