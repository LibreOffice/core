/*************************************************************************
 *
 *  $RCSfile: targets.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: as $ $Date: 2002-08-12 11:40:36 $
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

#define SPECIALTARGET_SELF                  DECLARE_ASCII("_self"           )   // The frame himself is searched.
#define SPECIALTARGET_PARENT                DECLARE_ASCII("_parent"         )   // The direct parent frame is searched.
#define SPECIALTARGET_TOP                   DECLARE_ASCII("_top"            )   // Search at ouer parents for the first task (if any exist) or a frame without a parent.
#define SPECIALTARGET_BLANK                 DECLARE_ASCII("_blank"          )   // Create a new task.
#define SPECIALTARGET_DEFAULT               DECLARE_ASCII("_default"        )   // Create a new task or recycle an existing one
#define SPECIALTARGET_BEAMER                DECLARE_ASCII("_beamer"         )   // special frame in hierarchy
#define SPECIALTARGET_MENUBAR               DECLARE_ASCII("_menubar"        )   // special target for menubars
#define SPECIALTARGET_HELPAGENT             DECLARE_ASCII("_helpagent"      )   // special target for the help agent window

class TargetCheck
{
    public:

    //_______________________________________________________________________
    /**
        it checks, if the given target name is a well known special one.
        The it returns <TRUE/> - otherwhise <FALSE/>.
        Special targets are defined above ...
     */
    static sal_Bool isSpecialTarget( const ::rtl::OUString& sTarget )
    {
        return  (
                    sTarget == SPECIALTARGET_SELF       ||
                    sTarget == SPECIALTARGET_PARENT     ||
                    sTarget == SPECIALTARGET_TOP        ||
                    sTarget == SPECIALTARGET_BLANK      ||
                    sTarget == SPECIALTARGET_DEFAULT    ||
                    sTarget == SPECIALTARGET_BEAMER     ||
                    sTarget == SPECIALTARGET_MENUBAR    ||
                    sTarget == SPECIALTARGET_HELPAGENT
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
    static sal_Bool isValidTarget( const ::rtl::OUString& sTarget )
    {
        return (sTarget.indexOf('_') == 0 && TargetCheck::isSpecialTarget(sTarget));
    }
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_TARGETS_H_
