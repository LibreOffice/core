/*************************************************************************
 *
 *  $RCSfile: targethelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 14:38:02 $
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
