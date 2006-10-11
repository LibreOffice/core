/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwRewriter.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-11 08:46:26 $
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

#ifndef _SW_REWRITER_HXX
#define _SW_REWRITER_HXX

#include <vector>
#include <tools/string.hxx>

typedef std::pair<String, String> SwRewriteRule;

class SwRewriter
{
    std::vector<SwRewriteRule> mRules;

public:
    SwRewriter();
    SwRewriter(const SwRewriter & rSrc);
    ~SwRewriter();

    void AddRule(const String & rWhat, const String & rWith);

    String Apply(const String & rStr) const;
    std::vector<String> Apply(const std::vector<String> & rStrs) const;

    String ToString() const;
};

#endif // _SW_REWRITER_HXX
