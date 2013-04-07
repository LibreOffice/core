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

#include <loadenv/targethelper.hxx>

namespace framework{

sal_Bool TargetHelper::matchSpecialTarget(const OUString& sCheckTarget  ,
                                                ESpecialTarget   eSpecialTarget)
{
    switch(eSpecialTarget)
    {
        case E_SELF :
            return (
                    (sCheckTarget.isEmpty()                 ) ||
                    (sCheckTarget.equals(SPECIALTARGET_SELF))
                   );

        case E_PARENT :
            return (sCheckTarget.equals(SPECIALTARGET_PARENT));

        case E_TOP :
            return (sCheckTarget.equals(SPECIALTARGET_TOP));

        case E_BLANK :
            return (sCheckTarget.equals(SPECIALTARGET_BLANK));

        case E_DEFAULT :
            return (sCheckTarget.equals(SPECIALTARGET_DEFAULT));

        case E_BEAMER :
            return (sCheckTarget.equals(SPECIALTARGET_BEAMER));

        case E_MENUBAR :
            return (sCheckTarget.equals(SPECIALTARGET_MENUBAR));

        case E_HELPTASK :
            return (sCheckTarget.equals(SPECIALTARGET_HELPTASK));
        default:
            return sal_False;
    }
}

sal_Bool TargetHelper::isValidNameForFrame(const OUString& sName)
{
    // some special targets are realy special ones :-)
    // E.g. the are realy used to locate one frame inside the frame tree.
    if (
        (sName.isEmpty()                                    ) ||
        (TargetHelper::matchSpecialTarget(sName, E_HELPTASK)) ||
        (TargetHelper::matchSpecialTarget(sName, E_BEAMER)  )
       )
        return sal_True;

    // all other names must be checked more general
    // special targets starts with a "_".
    return (sName.indexOf('_') != 0);
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
