/*************************************************************************
 *
 *  $RCSfile: hfi_doc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:24:33 $
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


#include <precomp.h>
#include "hfi_doc.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary_i/codeinf2.hxx>
#include <toolkit/hf_docentry.hxx>
#include "hfi_tag.hxx"




HF_IdlDocu::HF_IdlDocu( Environment &       io_rEnv,
                        HF_DocEntryList &   o_rOut )
    :   HtmlFactory_Idl( io_rEnv, &o_rOut.CurOut() ),
        rOut(o_rOut)
{
}

HF_IdlDocu::~HF_IdlDocu()
{
}

// KORR
//   Should not be used any longer.
//   Use Produce_byCesOwnDocu() or Produce_byDocu4Reference()
//   instead.
void
HF_IdlDocu::Produce_byData( const client &  i_ce,
                            const ce_info * i_doc ) const
{
    const ce_info * i_pDocu = i_doc != 0
                                ?   i_doc
                                :   i_ce.Docu();
    if (i_pDocu == 0)
        return;

    bool bShort = NOT i_pDocu->Short().IsEmpty();
    bool bDescr = NOT i_pDocu->Description().IsEmpty();


    if ( i_pDocu->IsDeprecated()
         OR i_ce.SightLevel() == ary::idl::sl_File AND NOT i_pDocu->IsPublished()
         OR i_pDocu->IsOptional() )
    {   // any usage restriction
        rOut.Produce_Term("Usage Restrictions");

        if ( i_pDocu->IsDeprecated() )
            rOut.Produce_Definition() >> *new Html::Italic << "deprecated";
        if ( i_ce.SightLevel() == ary::idl::sl_File AND NOT i_pDocu->IsPublished() )
            rOut.Produce_Definition() >> *new Html::Italic << "not published";
        if ( i_pDocu->IsOptional() )
            rOut.Produce_Definition() >> *new Html::Italic << "optional";

        if ( i_pDocu->IsDeprecated() AND
             // KORR
             // Workaround, because DocuTex2::IsEmpty() does not
             //   calculate whitespace tokens only as empty.
             i_pDocu->DeprecatedText().Tokens().size() > 1 )
        {
            rOut.Produce_Term("Deprecation Info");

            HF_IdlDocuTextDisplay
                aDescription( Env(), 0, i_ce);
            aDescription.Out().Enter( rOut.Produce_Definition() );
            i_pDocu->DeprecatedText().DisplayAt( aDescription );
            aDescription.Out().Leave();
        }
    }   // end if (<any usage restriction>)

    if ( bShort OR bDescr )
    {
        rOut.Produce_Term("Description");
        HF_IdlDocuTextDisplay
                aDescription( Env(), 0, i_ce);
        if (bShort)
        {
            aDescription.Out().Enter( rOut.Produce_Definition() );
            i_pDocu->Short().DisplayAt( aDescription );
            aDescription.Out().Leave();
        }
        if (bDescr)
        {
            aDescription.Out().Enter( rOut.Produce_Definition() );
            i_pDocu->Description().DisplayAt( aDescription );
            aDescription.Out().Leave();
        }
    }

    for ( std::vector< ary::info::AtTag2* >::const_iterator
                iter = i_pDocu->Tags().begin();
          iter != i_pDocu->Tags().end();
          ++iter )
    {
        if ( strlen( (*iter)->Title() ) > 0 )
        {
            HF_IdlTag
                    aTag(Env(),  i_ce);
            Xml::Element &
                rTerm = rOut.Produce_Term();
            aTag.Produce_byData( rTerm,
                                 rOut.Produce_Definition(),
                                 *(*iter) );
        }
    }   // end for
}

void
HF_IdlDocu::Produce_byCesOwnDocu( const client & i_ce ) const
{
    const ce_info * i_pDocu = i_ce.Docu();
    if (i_pDocu != 0)
        Produce_byDocuAndScope(*i_pDocu, &i_ce, i_ce);
}

void
HF_IdlDocu::Produce_byDocu4Reference( const ce_info &     i_rDocuForReference,
                                      const client &      i_rScopeGivingCe ) const
{
    Produce_byDocuAndScope(i_rDocuForReference, 0, i_rScopeGivingCe);
}

void
HF_IdlDocu::Produce_byDocuAndScope( const ce_info &     i_rDocu,
                                    const client *      i_pClient,
                                    const client &      i_rScopeGivingCe ) const
{
    bool bShort = NOT i_rDocu.Short().IsEmpty();
    bool bDescr = NOT i_rDocu.Description().IsEmpty();

    if ( i_rDocu.IsDeprecated()
         OR (i_pClient != 0 ? i_pClient->SightLevel() == ary::idl::sl_File : false)
            AND NOT i_rDocu.IsPublished()
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
             // KORR
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

    for ( std::vector< ary::info::AtTag2* >::const_iterator
                iter = i_rDocu.Tags().begin();
          iter != i_rDocu.Tags().end();
          ++iter )
    {
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
}
