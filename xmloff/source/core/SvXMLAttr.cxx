/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Nigel Hawkins  - n.hawkins@gmx.com
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <limits.h>    //USHRT_MAX
#include "SvXMLAttr.hxx"

SvXMLAttr::SvXMLAttr( const OUString& rLName,
                      const OUString& rValue ) :
        aPrefixPos(USHRT_MAX),
        aLName(rLName),
        aValue(rValue)
{
}

SvXMLAttr::SvXMLAttr( const sal_uInt16 nPos,
                      const OUString& rLName,
                      const OUString& rValue ) :
        aPrefixPos(nPos),
        aLName(rLName),
        aValue(rValue)
{
}

bool SvXMLAttr::operator== (const SvXMLAttr &rCmp) const
{
    return ( rCmp.aPrefixPos == aPrefixPos ) &&
           ( rCmp.aLName == aLName ) &&
           ( rCmp.aValue == aValue );
}

sal_uInt16 SvXMLAttr::getPrefixPos() const
{
    return aPrefixPos;
}

const OUString& SvXMLAttr::getLName() const
{
    return aLName;
}

const OUString& SvXMLAttr::getValue() const {
    return aValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
