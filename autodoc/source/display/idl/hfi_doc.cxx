/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hfi_doc.cxx,v $
 * $Revision: 1.10 $
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

#include <precomp.h>
#include "hfi_doc.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <ary_i/d_token.hxx>
#include <toolkit/hf_docentry.hxx>
#include "hfi_tag.hxx"
#include "hi_ary.hxx"




HF_IdlDocu::HF_IdlDocu( Environment &       io_rEnv,
                        HF_DocEntryList &   o_rOut )
    :   HtmlFactory_Idl( io_rEnv, &o_rOut.CurOut() ),
        rOut(o_rOut)
{
}

HF_IdlDocu::~HF_IdlDocu()
{
}

void
HF_IdlDocu::Produce_fromCodeEntity( const client & i_ce ) const
{
    const ce_info *
        i_pDocu = Get_IdlDocu(i_ce.Docu());
    if (i_pDocu != 0)
        Produce_byDocuAndScope(*i_pDocu, &i_ce, i_ce);
}

void
HF_IdlDocu::Produce_fromReference( const ce_info &   i_rDocuForReference,
                                   const client &    i_rScopeGivingCe ) const
{
    Produce_byDocuAndScope(i_rDocuForReference, 0, i_rScopeGivingCe );
}

void
HF_IdlDocu::Produce_byDocuAndScope( const ce_info & i_rDocu,
                                    const client *  i_pClient,
                                    const client &  i_rScopeGivingCe ) const
{
    bool bShort = NOT i_rDocu.Short().IsEmpty();
    bool bDescr = NOT i_rDocu.Description().IsEmpty();

    if ( i_rDocu.IsDeprecated()
         OR (
             (i_pClient != 0 ? i_pClient->SightLevel() == ary::idl::sl_File : false)
             AND NOT i_rDocu.IsPublished()
            )
         OR i_rDocu.IsOptional() )
    {   // any usage restriction
        rOut.Produce_Term("Usage Restrictions");

        if ( i_rDocu.IsDeprecated() )
            rOut.Produce_Definition() >> *new Html::Italic << "deprecated";
        if ( (i_pClient != 0 ? i_pClient->SightLevel() == ary::idl::sl_File : false)
             AND NOT i_rDocu.IsPublished() )
            rOut.Produce_Definition() >> *new Html::Italic << "not published";
        if ( i_rDocu.IsOptional() )
            rOut.Produce_Definition() >> *new Html::Italic << "optional";

        if ( i_rDocu.IsDeprecated() AND
             // KORR_FUTURE
             // Workaround, because DocuTex2::IsEmpty() does not
             //   calculate whitespace tokens only as empty.
             i_rDocu.DeprecatedText().Tokens().size() > 1 )
        {
            rOut.Produce_Term("Deprecation Info");

            HF_IdlDocuTextDisplay
                aDescription( Env(), 0, i_rScopeGivingCe);
            aDescription.Out().Enter( rOut.Produce_Definition() );
            i_rDocu.DeprecatedText().DisplayAt( aDescription );
            aDescription.Out().Leave();
        }
    }   // end if (<any usage restriction>)

    if ( bShort OR bDescr )
    {
        rOut.Produce_Term("Description");
        HF_IdlDocuTextDisplay
                aDescription( Env(), 0, i_rScopeGivingCe);
        if (bShort)
        {
            aDescription.Out().Enter( rOut.Produce_Definition() );
            i_rDocu.Short().DisplayAt( aDescription );
            aDescription.Out().Leave();
        }
        if (bDescr)
        {
            aDescription.Out().Enter( rOut.Produce_Definition() );
            i_rDocu.Description().DisplayAt( aDescription );
            aDescription.Out().Leave();
        }
    }

    std::vector< csi::dsapi::DT_SeeAlsoAtTag* >
        aSeeAlsosWithoutText;
    std::vector< csi::dsapi::DT_SeeAlsoAtTag* >
        aSeeAlsosWithText;

    for ( std::vector< ary::inf::AtTag2* >::const_iterator
                iter = i_rDocu.Tags().begin();
          iter != i_rDocu.Tags().end();
          ++iter )
    {
        csi::dsapi::DT_SeeAlsoAtTag*
            pSeeAlso = dynamic_cast< csi::dsapi::DT_SeeAlsoAtTag * >(*iter);
        if (pSeeAlso != 0 )
        {
            if ( pSeeAlso->Text().IsEmpty() )
            {
                aSeeAlsosWithoutText.push_back(pSeeAlso);
            }
            else
            {
                aSeeAlsosWithText.push_back(pSeeAlso);
            }
            continue;
        }

        if ( strlen( (*iter)->Title() ) > 0 )
        {
            HF_IdlTag
                    aTag(Env(), i_rScopeGivingCe);
            Xml::Element &
                rTerm = rOut.Produce_Term();
            aTag.Produce_byData( rTerm,
                                 rOut.Produce_Definition(),
                                 *(*iter) );
        }
    }   // end for

    if (aSeeAlsosWithoutText.size() > 0)
    {
        HF_IdlTag
            aSeeAlsoTag(Env(),  i_rScopeGivingCe);
        Xml::Element &
            rTerm = rOut.Produce_Term();
        aSeeAlsoTag.Produce_byData( rTerm,
                                    rOut.Produce_Definition(),
                                    aSeeAlsosWithoutText );
    }

    for ( std::vector< csi::dsapi::DT_SeeAlsoAtTag* >::const_iterator
                itSee2 = aSeeAlsosWithText.begin();
          itSee2 != aSeeAlsosWithText.end();
          ++itSee2 )
    {
        HF_IdlTag
            aTag(Env(),  i_rScopeGivingCe);
        Xml::Element &
            rTerm = rOut.Produce_Term();
            aTag.Produce_byData( rTerm,
                                 rOut.Produce_Definition(),
                                 *(*itSee2) );
    }   // end for
}
