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

#include "htmlnum.hxx"
#include <ndtxt.hxx>
#include <doc.hxx>

void SwHTMLNumRuleInfo::Set( const SwTxtNode& rTxtNd )
{
    const SwNumRule* pTxtNdNumRule( rTxtNd.GetNumRule() );
    if ( pTxtNdNumRule &&
         pTxtNdNumRule != rTxtNd.GetDoc()->GetOutlineNumRule() )
    {
        pNumRule = const_cast<SwNumRule*>(pTxtNdNumRule);
        nDeep = static_cast< sal_uInt16 >(pNumRule ? rTxtNd.GetActualListLevel() + 1 : 0);
        bNumbered = rTxtNd.IsCountedInList();
        // #i57919# - correction of refactoring done by cws swnumtree:
        // <bRestart> has to be set to <true>, if numbering is restarted at this
        // text node and the start value equals <USHRT_MAX>.
        // Start value <USHRT_MAX> indicates, that at this text node the numbering
        // is restarted with the value given at the corresponding level.
        bRestart = rTxtNd.IsListRestart() && !rTxtNd.HasAttrListRestartValue();
    }
    else
    {
        pNumRule = 0;
        nDeep = 0;
        bNumbered = bRestart = sal_False;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
