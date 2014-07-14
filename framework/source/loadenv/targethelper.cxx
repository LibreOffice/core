/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    // some special targets are really special ones :-)
    // E.g. the are really used to locate one frame inside the frame tree.
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
