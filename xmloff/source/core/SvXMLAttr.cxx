/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
