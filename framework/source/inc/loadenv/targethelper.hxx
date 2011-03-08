/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __FRAMEWORK_TARGETHELPER_HXX_
#define __FRAMEWORK_TARGETHELPER_HXX_

//_______________________________________________
// own includes

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <targets.h>
//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// definitions

/** @short  can be used to detect, if a target name (used e.g. for XFrame.findFrame())
            has a special meaning or can be used as normal frame name (e.g. for XFrame.setName()).

    @author as96863
 */
class TargetHelper
{
    //___________________________________________
    // const

    public:

        /** @short  its used at the following interfaces to classify
                    target names.
         */
        enum ESpecialTarget
        {
            E_NOT_SPECIAL   ,
            E_SELF          ,
            E_PARENT        ,
            E_TOP           ,
            E_BLANK         ,
            E_DEFAULT       ,
            E_BEAMER        ,
            E_MENUBAR       ,
            E_HELPAGENT     ,
            E_HELPTASK
        };

    //___________________________________________
    // interface

    public:

        //___________________________________________

        /** @short  it checks the given unknown target name,
                    if it's the expected special one.

            @note   An empty target is similar to "_self"!

            @param  sCheckTarget
                    must be the unknwon target name, which should be checked.

            @param  eSpecialTarget
                    represent the expected target.

            @return It returns <TRUE/> if <var>sCheckTarget</var> represent
                    the expected <var>eSpecialTarget</var> value; <FALSE/> otherwhise.
         */
        static sal_Bool matchSpecialTarget(const ::rtl::OUString& sCheckTarget  ,
                                                 ESpecialTarget   eSpecialTarget);

        //___________________________________________

        /** @short  it checks, if the given name can be used
                    to set it at a frame using XFrame.setName() method.

            @descr  Because we handle special targets in a hard coded way
                    (means we do not check the real name of a frame then)
                    such named frames will never be found!

                    And in case such special names can exists one times only
                    by definition inside the same frame tree (e.g. _beamer and
                    OFFICE_HELP_TASK) its not a good idea to allow anything here :-)

                    Of course we can't check unknwon names, which are not special ones.
                    But we decide, that it's not allowed to use "_" as first sign
                    (because we reserve this letter for our own purposes!)
                    and the value must not a well known special target.

            @param  sName
                    the new frame name, which sould be checked.
         */
        static sal_Bool isValidNameForFrame(const ::rtl::OUString& sName);
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_TARGETHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
