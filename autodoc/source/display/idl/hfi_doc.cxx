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

    if (!aSeeAlsosWithoutText.empty())
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
