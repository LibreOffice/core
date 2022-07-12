/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <limits.h>

#include <utility>
#include "SvXMLAttr.hxx"

SvXMLAttr::SvXMLAttr( OUString _aLName,
                      OUString _aValue ) :
        aPrefixPos(USHRT_MAX),
        aLName(std::move(_aLName)),
        aValue(std::move(_aValue))
{
}

SvXMLAttr::SvXMLAttr( const sal_uInt16 nPos,
                      OUString _aLName,
                      OUString _aValue ) :
        aPrefixPos(nPos),
        aLName(std::move(_aLName)),
        aValue(std::move(_aValue))
{
}

bool SvXMLAttr::operator== (const SvXMLAttr &rCmp) const
{
    return ( rCmp.aPrefixPos == aPrefixPos ) &&
           ( rCmp.aLName == aLName ) &&
           ( rCmp.aValue == aValue );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
