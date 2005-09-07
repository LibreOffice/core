/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_tag.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:48:12 $
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

#ifndef ADC_DISPLAY_HFI_TAG_HXX
#define ADC_DISPLAY_HFI_TAG_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
#include <ary_i/disdocum.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_language.hxx>
#include <ary_i/ci_atag2.hxx>
#include <ary_i/ci_text2.hxx>
#include <ary_i/codeinf2.hxx>

#include <toolkit/hf_docentry.hxx>

namespace ary
{
namespace idl
{
    class Module;
}
}


/** This class is an implementation of ary::info::DocuText_Display
    and will be used by that interface.
*/
class HF_IdlDocuTextDisplay : public HtmlFactory_Idl,
                              public ary::info::DocuText_Display
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
                  public ary::info::DocuTag_Display
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
                            const ary::info::AtTag2 &
                                                i_rTag ) const;
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
                            const ary::info::DocuTex2 &
                                                i_rText ) const;
    // DATA
    mutable Xml::Element *
                        pTitleOut;
    mutable HF_IdlDocuTextDisplay
                        aTextOut;
};




// IMPLEMENTATION


#endif


