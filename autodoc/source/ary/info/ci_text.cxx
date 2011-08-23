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

#include <precomp.h>
#include <ary/info/ci_text.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/info/all_dts.hxx>


namespace ary
{
namespace info
{

DocuText::DocuText()
    :   bUsesHtml(false)
{
}

DocuText::~DocuText()
{
    for ( TokenList::iterator iter = aTokens.begin();
          iter != aTokens.end();
          ++iter )
    {
        delete (*iter);
    }
}

void
DocuText::StoreAt( DocuDisplay & o_rDisplay ) const
{
    ary::info::DocuText::TokenList::const_iterator itEnd = aTokens.end();
    for ( ary::info::DocuText::TokenList::const_iterator it = aTokens.begin();
          it != itEnd;
          ++it )
    {
        (*it)->StoreAt(o_rDisplay);
    }
}

}   // namespace info
}   // namespace ary


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
