/*************************************************************************
 *
 *  $RCSfile: targets.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:28 $
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

#ifndef __FRAMEWORK_TARGETS_H_
#define __FRAMEWORK_TARGETS_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  Values for special frame search ... sTargetFrameName of findFrame() or queryDispatch() or loadComponentFromURL()
//_________________________________________________________________________________________________________________

#define SPECIALTARGET_ASCII_SELF            "_self"                             // The frame himself is searched.
#define SPECIALTARGET_ASCII_PARENT          "_parent"                           // The direct parent frame is searched.
#define SPECIALTARGET_ASCII_TOP             "_top"                              // Search at ouer parents for the first task (if any exist) or a frame without a parent.
#define SPECIALTARGET_ASCII_BLANK           "_blank"                            // Create a new task.
#define SPECIALTARGET_ASCII_DEFAULT         "_default"                          // Create a new task or recycle an existing one
#define SPECIALTARGET_ASCII_BEAMER          "_beamer"                           // special frame in hierarchy
#define SPECIALTARGET_ASCII_MENUBAR         "_menubar"                          // special target for menubars
#define SPECIALTARGET_ASCII_HELPAGENT       "_helpagent"                        // special target for the help agent window
#define SPECIALTARGET_ASCII_HELPTASK        "OFFICE_HELP_TASK"                  // special name for our help task

#define SPECIALTARGET_SELF                  DECLARE_ASCII(SPECIALTARGET_ASCII_SELF       )
#define SPECIALTARGET_PARENT                DECLARE_ASCII(SPECIALTARGET_ASCII_PARENT     )
#define SPECIALTARGET_TOP                   DECLARE_ASCII(SPECIALTARGET_ASCII_TOP        )
#define SPECIALTARGET_BLANK                 DECLARE_ASCII(SPECIALTARGET_ASCII_BLANK      )
#define SPECIALTARGET_DEFAULT               DECLARE_ASCII(SPECIALTARGET_ASCII_DEFAULT    )
#define SPECIALTARGET_BEAMER                DECLARE_ASCII(SPECIALTARGET_ASCII_BEAMER     )
#define SPECIALTARGET_MENUBAR               DECLARE_ASCII(SPECIALTARGET_ASCII_MENUBAR    )
#define SPECIALTARGET_HELPAGENT             DECLARE_ASCII(SPECIALTARGET_ASCII_HELPAGENT  )
#define SPECIALTARGET_HELPTASK              DECLARE_ASCII(SPECIALTARGET_ASCII_HELPTASK   )

class TargetCheck
{
    public:

    //_______________________________________________________________________
    /**
        it checks the given unknown target name, if it's the expected special one.

        @param  sCheckTarget
                    must be the unknwon target name, which should be checked

        @param  sSpecialTarget
                    represent the expected target name

        @return It returns <TRUE/> if <var>sCheckTarget</var> represent the expected
                <var>sSpecialTarget</var> value; <FALSE/> otherwhise.
                Compare will be done by ignoring case.
     */
    static sal_Bool matchSpecialTarget( /*IN*/ const ::rtl::OUString& sCheckTarget   ,
                                        /*IN*/ const ::rtl::OUString& sSpecialTarget )
    {
        return sCheckTarget.equalsIgnoreAsciiCase(sSpecialTarget);
    }

    //_______________________________________________________________________
    /**
        reject not allowed target names for frames.

        Some special targets are allowed for searching only ...
        Some other ones must be set as a frame name realy to locate it.
        This method filter such target names.

        @param  sName
                    the target name, which the outside code will set as a frame name.

        @return It returns <TRUE/> if <var>sName</var> is allowed as frame name; <FALSE/> otherwhise.
                Compare will be done by ignoring case.
     */
    static sal_Bool isValidFrameName( /*IN*/ const ::rtl::OUString& sName )
    {
        return  (
                    !TargetCheck::matchSpecialTarget(sName,SPECIALTARGET_SELF   ) ||
                    !TargetCheck::matchSpecialTarget(sName,SPECIALTARGET_PARENT ) ||
                    !TargetCheck::matchSpecialTarget(sName,SPECIALTARGET_TOP    ) ||
                    !TargetCheck::matchSpecialTarget(sName,SPECIALTARGET_BLANK  ) ||
                    !TargetCheck::matchSpecialTarget(sName,SPECIALTARGET_DEFAULT)
                );
    }

    //_______________________________________________________________________
    /**
        it checks, if the given target name is a well known special one anyway.
        Special targets are defined above ...

        @param  sTarget
                    must be the unknwon target name, which should be checked

        @return It returns <TRUE/> if <var>sTarget</var> represent a special one;
                <FALSE/> otherwhise.
     */
    static sal_Bool isSpecialTarget( /*IN*/ const ::rtl::OUString& sTarget )
    {
        return  (
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_SELF     ) ||
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_PARENT   ) ||
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_TOP      ) ||
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_BLANK    ) ||
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_DEFAULT  ) ||
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_BEAMER   ) ||
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_MENUBAR  ) ||
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_HELPAGENT) ||
                    TargetCheck::matchSpecialTarget(sTarget,SPECIALTARGET_HELPTASK )
                );
    }

    //_______________________________________________________________________
    /**
        it checks, if the given target name can be valid
        Of course we can't check unknwon names, which are not special ones.
        But we decide, that it's not allowed to use "_" as first sign
        and the value doesn't represent such special target.
        We reserve this letter for our own purposes.
     */
    static sal_Bool isValidTarget( /*IN*/ const ::rtl::OUString& sTarget )
    {
        sal_Int32 nPosOf_  = sTarget.indexOf('_');
        sal_Bool  bSpecial = TargetCheck::isSpecialTarget(sTarget);

        return (( nPosOf_ == 0 && bSpecial ) || ( nPosOf_ != 0 && ! bSpecial ));
    }
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_TARGETS_H_
