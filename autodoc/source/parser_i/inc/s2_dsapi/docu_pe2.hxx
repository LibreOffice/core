/*************************************************************************
 *
 *  $RCSfile: docu_pe2.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:43:37 $
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

#ifndef ADC_DSAPI_DOCU_PE2_HXX
#define ADC_DSAPI_DOCU_PE2_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_dsapi/tokintpr.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace info
{
class CodeInformation;
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
                        SapiDocu_PE();
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


    DYN ary::info::CodeInformation *
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

    typedef void ( SapiDocu_PE::*F_TokenAdder )( DYN ary::info::DocuToken & let_drNewToken );

    void                AddDocuToken2Void(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2Short(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2Description(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2Deprecated(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2CurAtTag(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );
    void                SetCurParameterAtTagName(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );
    void                SetCurSeeAlsoAtTagLinkText(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );
    void                SetCurSinceAtTagVersion(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );
    void                AddDocuToken2SinceAtTag(
                            DYN ary::info::DocuToken &
                                                let_drNewToken );

    // DATA
    Dyn<ary::info::CodeInformation>
                        pDocu;
    E_State             eState;
    F_TokenAdder        fCurTokenAddFunction;

    Dyn<DT_AtTag>       pCurAtTag;
    udmstri             sCurDimAttribute;
};

}   // namespace dsapi
}   // namespace csi


// IMPLEMENTATION


#endif

