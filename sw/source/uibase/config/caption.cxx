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


#include <numrule.hxx>
#include <caption.hxx>

InsCaptionOpt::InsCaptionOpt(const SwCapObjType eType, const SvGlobalName* pOleId) :
    m_bUseCaption(false),
    m_eObjType(eType),
    m_nNumType(SVX_NUM_ARABIC),
    m_sNumberSeparator(". "),
    m_nPos(1),
    m_nLevel(0),
    m_sSeparator( OUString(": ") ),
    m_bIgnoreSeqOpts(false),
    m_bCopyAttributes(false)
{
    if (pOleId)
        m_aOleId = *pOleId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
