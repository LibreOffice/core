/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: targethelper.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_LOADENV_TARGETHELPER_HXX_
#include <loadenv/targethelper.hxx>
#endif

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// declarations

/*-----------------------------------------------
    05.08.2003 09:12
-----------------------------------------------*/
TargetHelper::ESpecialTarget TargetHelper::classifyTarget(const ::rtl::OUString& sTarget)
{
    if (
        (!sTarget.getLength()              ) ||
        (sTarget.equals(SPECIALTARGET_SELF))
       )
        return E_SELF;

    if (sTarget.equals(SPECIALTARGET_PARENT))
        return E_PARENT;

    if (sTarget.equals(SPECIALTARGET_TOP))
        return E_TOP;

    if (sTarget.equals(SPECIALTARGET_BLANK))
        return E_BLANK;

    if (sTarget.equals(SPECIALTARGET_DEFAULT))
        return E_DEFAULT;

    if (sTarget.equals(SPECIALTARGET_BEAMER))
        return E_BEAMER;

    if (sTarget.equals(SPECIALTARGET_MENUBAR))
        return E_MENUBAR;

    if (sTarget.equals(SPECIALTARGET_HELPAGENT))
        return E_HELPAGENT;

    if (sTarget.equals(SPECIALTARGET_HELPTASK))
        return E_HELPTASK;

    return E_NOT_SPECIAL;
}

/*-----------------------------------------------
    05.08.2003 09:08
-----------------------------------------------*/
sal_Bool TargetHelper::matchSpecialTarget(const ::rtl::OUString& sCheckTarget  ,
                                                ESpecialTarget   eSpecialTarget)
{
    switch(eSpecialTarget)
    {
        case E_SELF :
            return (
                    (!sCheckTarget.getLength()              ) ||
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

        case E_HELPAGENT :
            return (sCheckTarget.equals(SPECIALTARGET_HELPAGENT));

        case E_HELPTASK :
            return (sCheckTarget.equals(SPECIALTARGET_HELPTASK));
        default:
            return sal_False;
    }

    return sal_False;
}

/*-----------------------------------------------
    05.08.2003 09:17
-----------------------------------------------*/
sal_Bool TargetHelper::isValidNameForFrame(const ::rtl::OUString& sName)
{
    // some special targets are realy special ones :-)
    // E.g. the are realy used to locate one frame inside the frame tree.
    if (
        (!sName.getLength()                                 ) ||
        (TargetHelper::matchSpecialTarget(sName, E_HELPTASK)) ||
        (TargetHelper::matchSpecialTarget(sName, E_BEAMER)  )
       )
        return sal_True;

    // all other names must be checked more general
    // special targets starts with a "_".
    return (sName.indexOf('_') != 0);
}

} // namespace framework
