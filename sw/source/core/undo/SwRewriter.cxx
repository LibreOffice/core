/*************************************************************************
 *
 *  $RCSfile: SwRewriter.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-09-08 15:21:33 $
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

#include <algorithm>
#include <SwRewriter.hxx>

using namespace std;

bool operator == (const SwRewriteRule & a, const SwRewriteRule & b)
{
    return a.first == b.first;
}

SwRewriter::SwRewriter()
{
}

SwRewriter::SwRewriter(const SwRewriter & rSrc)
    : mRules(rSrc.mRules)
{
}

SwRewriter::~SwRewriter()
{
}

void SwRewriter::AddRule(const String & rWhat, const String & rWith)
{
    SwRewriteRule aRule(rWhat, rWith);

    vector<SwRewriteRule>::iterator aIt;

    aIt = find(mRules.begin(), mRules.end(), aRule);

    if (aIt != mRules.end())
        *aIt = aRule;
    else
        mRules.push_back(aRule);
}

String SwRewriter::Apply(const String & rStr) const
{
    String aResult = rStr;
    vector<SwRewriteRule>::const_iterator aIt;

    for (aIt = mRules.begin(); aIt != mRules.end(); aIt++)
        aResult.SearchAndReplaceAll(aIt->first, aIt->second);

    return aResult;
}

vector<String> SwRewriter::Apply(const vector<String> & rStrs) const
{
    vector<String> aResult;
    vector<String>::const_iterator aIt;

    for (aIt = rStrs.begin(); aIt != rStrs.end(); aIt++)
        aResult.push_back(Apply(*aIt));

    return aResult;
}

String SwRewriter::ToString() const
{
    String aResult("[ \n", RTL_TEXTENCODING_ASCII_US);

    vector<SwRewriteRule>::const_iterator aIt;

    for (aIt = mRules.begin(); aIt != mRules.end(); aIt++)
    {
        aResult += String("  \"", RTL_TEXTENCODING_ASCII_US);
        aResult += aIt->first;
        aResult += String("\" -> \"", RTL_TEXTENCODING_ASCII_US);
        aResult += aIt->second;
        aResult += String("\"\n", RTL_TEXTENCODING_ASCII_US);
    }

    aResult += String("]", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

