/*************************************************************************
 *
 *  $RCSfile: hfi_tag.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 11:34:15 $
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


