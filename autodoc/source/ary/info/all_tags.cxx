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
#include <ary/info/all_tags.hxx>


// NOT FULLY DEFINED SERVICES
#include <limits>
#include <ary/info/infodisp.hxx>
#include <adc_cl.hxx>


namespace ary
{
namespace info
{



//*****************************     StdTag      ***********************//


StdTag::StdTag( E_AtTagId i_eId )
    :   eId(i_eId),
        // aText,
        pNext(0)
{
}

bool
StdTag::Add_SpecialMeaningToken( const char *   ,
                                 intt           )
{
    // Does nothing

    // KORR_FUTURE
    //   Should be a logical exception:
    // csv_assert(false);
    return false;
}

UINT8
StdTag::NrOfSpecialMeaningTokens() const
{
    return 0;
}

AtTag *
StdTag::GetFollower()
{
    if (pNext != 0)
        return pNext->GetFollower();
    pNext = new StdTag(eId);
    return pNext;
}

void
StdTag::do_StoreAt( DocuDisplay &  o_rDisplay ) const
{
    o_rDisplay.Display_StdTag( *this );
}

DocuText *
StdTag::Text()
{
    return &aText;
}



//*****************************     BaseTag     ***********************//

BaseTag::BaseTag()
    :   // sBase
        // aText
        pNext(0)
{
}

bool
BaseTag::Add_SpecialMeaningToken( const char *      i_sText,
                                  intt              i_nNr )
{
    if ( i_nNr == 1 )
    {
        sBase.AssignText(i_sText,"::");
        return true;
    }
    return false;
}

const char *
BaseTag::Title() const
{
    return "Base Classes";
}

UINT8
BaseTag::NrOfSpecialMeaningTokens() const
{
    return 1;
}

AtTag *
BaseTag::GetFollower()
{
    if (pNext != 0)
        return pNext->GetFollower();
    pNext = new BaseTag;
    return pNext;
}

DocuText *
BaseTag::Text()
{
    return &aText;
}



//*****************************     ExceptionTag    ***********************//

ExceptionTag::ExceptionTag()
    :   // sException,
        // aText
        pNext(0)
{
}

bool
ExceptionTag::Add_SpecialMeaningToken( const char *     i_sText,
                                       intt             i_nNr )
{
    if ( i_nNr == 1 )
    {
        sException.AssignText(i_sText,"::");
        return true;
    }
    return false;
}

const char *
ExceptionTag::Title() const
{
    return "Thrown Exceptions";
}

UINT8
ExceptionTag::NrOfSpecialMeaningTokens() const
{
    return 1;
}

AtTag *
ExceptionTag::GetFollower()
{
    if (pNext != 0)
        return pNext->GetFollower();
    pNext = new ExceptionTag;
    return pNext;
}

DocuText *
ExceptionTag::Text()
{
    return &aText;
}


//*****************************     ImplementsTag   ***********************//

ImplementsTag::ImplementsTag()
    :   // sBase
        // aText
        pNext(0)
{
}

bool
ImplementsTag::Add_SpecialMeaningToken( const char *        i_sText,
                                        intt                i_nNr )
{
    if ( i_nNr == 1 )
    {
        sName.AssignText(i_sText,"::");
    }
    else
    {
        GetFollower()->Add_SpecialMeaningToken(i_sText,1);
    }
    return true;
}

const char *
ImplementsTag::Title() const
{
    return "Implements";
}

UINT8
ImplementsTag::NrOfSpecialMeaningTokens() const
{
    return std::numeric_limits<UINT8>::max();
}

AtTag *
ImplementsTag::GetFollower()
{
    if (pNext != 0)
        return pNext->GetFollower();
    pNext = new ImplementsTag;
    return pNext;
}

DocuText *
ImplementsTag::Text()
{
    return 0;
}

//*****************************     ParameterTag    ***********************//


ParameterTag::ParameterTag()
    :   // sName
        // aText
        pNext(0)
{
}

bool
ParameterTag::Add_SpecialMeaningToken( const char *     i_sText,
                                       intt             i_nNr )
{
    if ( i_nNr == 1 )
    {
        sName = i_sText;
        return true;
    }
    else if (i_nNr == 2)
    {
        uintt nLen = strlen(i_sText);
        if (*i_sText == '[' AND i_sText[nLen-1] == ']')
        {
            sValidRange = String(i_sText+1, nLen-2);
            return true;
        }
    }
    return false;
}

UINT8
ParameterTag::NrOfSpecialMeaningTokens() const
{
    return 2;
}

AtTag *
ParameterTag::GetFollower()
{
    if (pNext != 0)
        return pNext->GetFollower();
    return  pNext = new ParameterTag;
}

DocuText *
ParameterTag::Text()
{
    return &aText;
}

void
ParameterTag::do_StoreAt( DocuDisplay &  o_rDisplay ) const
{
    o_rDisplay.Display_ParameterTag( *this );
}

//*****************************     TemplateTag     ***********************//


TemplateTag::TemplateTag()
    :   // sName
        // aText
        pNext(0)
{
}

bool
TemplateTag::Add_SpecialMeaningToken( const char *      i_sText,
                                       intt             i_nNr )
{
    if ( i_nNr == 1 )
    {
        sName = i_sText;
        return true;
    }
    return false;
}

const char *
TemplateTag::Title() const
{
    return "Template Parameters";
}

UINT8
TemplateTag::NrOfSpecialMeaningTokens() const
{
    return 1;
}

AtTag *
TemplateTag::GetFollower()
{
    if (pNext != 0)
        return pNext->GetFollower();
    return  pNext = new TemplateTag;
}

void
TemplateTag::do_StoreAt( DocuDisplay &  o_rDisplay ) const
{
    o_rDisplay.Display_TemplateTag( *this );
}


DocuText *
TemplateTag::Text()
{
    return &aText;
}

}   // namespace info
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
