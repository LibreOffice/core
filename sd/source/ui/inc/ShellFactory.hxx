/*************************************************************************
 *
 *  $RCSfile: ShellFactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: af $ $Date: 2004-07-20 08:22:14 $
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

#ifndef SD_VIEW_SHELL_FACTORY_HXX
#define SD_VIEW_SHELL_FACTORY_HXX

#include <sal/types.h>
#include <memory>

class Window;

namespace sd {

class FrameView;
class ViewShell;

typedef sal_Int32 ShellId;
// This shell id is a reserved value and does not specify any valid shell.
static const sal_Int32 snInvalidShellId = -1;

template<class ShellType>
class ShellFactory
{
public:
    /** This abstract virtual class needs a destructor so that the
        destructors of derived classes are called.
    */
    virtual ~ShellFactory (void) {};

    /** Create a new instance of a view shell for the given id that will
        be stacked onto the given view shell base.
        @return
            Return the new view shell or NULL when a creation is not
            possible.
        */
    virtual ShellType* CreateShell (
        ShellId nId,
        ::Window* pParentWindow,
        FrameView* pFrameView = NULL) = 0;

    /** Tell the factory that a shell is no longer in use.  It may destroy
        it or put it for future use in a cache.
    */
    virtual void ReleaseShell (ShellType* pShell) = 0;
};

} // end of namespace sd

#endif
