/*************************************************************************
 *
 *  $RCSfile: targethelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 14:36:41 $
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

#ifndef __FRAMEWORK_TARGETHELPER_HXX_
#define __FRAMEWORK_TARGETHELPER_HXX_

//_______________________________________________
// own includes

#include <sal/types.h>
#include <rtl/ustring.hxx>

//_______________________________________________
// namespace

namespace framework{


//_______________________________________________
// const

#define SPECIALTARGET_SELF          ::rtl::OUString::createFromAscii("_self"           )
#define SPECIALTARGET_PARENT        ::rtl::OUString::createFromAscii("_parent"         )
#define SPECIALTARGET_TOP           ::rtl::OUString::createFromAscii("_top"            )
#define SPECIALTARGET_BLANK         ::rtl::OUString::createFromAscii("_blank"          )
#define SPECIALTARGET_DEFAULT       ::rtl::OUString::createFromAscii("_default"        )
#define SPECIALTARGET_BEAMER        ::rtl::OUString::createFromAscii("_beamer"         )
#define SPECIALTARGET_MENUBAR       ::rtl::OUString::createFromAscii("_menubar"        )
#define SPECIALTARGET_HELPAGENT     ::rtl::OUString::createFromAscii("_helpagent"      )
#define SPECIALTARGET_HELPTASK      ::rtl::OUString::createFromAscii("OFFICE_HELP_TASK")

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

        /** @short  it checks, if the given target name is a well known special
                    one anyway and return a suitable enum value.

            @note   An empty target is similar to "_self"!

            @param  sTarget
                    the unknown target name, which should be checked.

            @return Returns a suitable enum value, which classify the
                    given target name.
         */
        static ESpecialTarget classifyTarget(const ::rtl::OUString& sTarget);

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
