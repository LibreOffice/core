/*************************************************************************
 *
 *  $RCSfile: SdGlobalResourceContainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 08:56:07 $
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

#ifndef SD_GLOBAL_RESOURCE_CONTAINER_HXX
#define SD_GLOBAL_RESOURCE_CONTAINER_HXX

#include "sdmod.hxx"
#include <memory>

namespace sd {

class SdGlobalResource
{
public:
    virtual ~SdGlobalResource (void) {};
};

/** The purpose of this container is to hold references to resources that
    are globally available to all interested objects and to destroy them
    when the sd module is destroyed.  Resources can be containers of bitmaps
    or the container of master pages used by the MasterPagesSelector objects
    in the task panel.

    It works like a singleton in that there is one instance per sd module.
    Resources can be added (by themselves or their owners) to the
    container.  The main task of the container is the destruction of all
    resources that have been added to it.

    As you may note, there is no method to get a resource from the
    container.  It is the task of the resource to provide other means of
    access to it.

    The reason for this design is not to have to change the SdModule
    destructor every time when there is a new resource.  This is done by
    reversing the dependency between module and resource: the resource knows
    about the module--this container class to be more precisely--and tells
    it to destroy the resource when the sd module is at the end of its
    lifetime.
*/
class SdGlobalResourceContainer
{
public:
    static SdGlobalResourceContainer& Instance (void);

    /** Add a resource to the container.  The ownership of the resource is
        transferred to the container.  The resource is destroyed when the
        container is destroyed, i.e. when the sd module is destroyed.
    */
    void AddResource (::std::auto_ptr<SdGlobalResource> pResource);

    /** Tell the container that it is not any longer responsible for the
        specified resource.
        @return
            When the specified resource has previously added to the
            container the resource is returned (which is, of course, the
            same pointer as the given one.)  Otherwise a NULL pointer is
            returned.
    */
    ::std::auto_ptr<SdGlobalResource> ReleaseResource (
        SdGlobalResource* pResource);

protected:
    friend class ::SdModule;
    friend class ::std::auto_ptr<SdGlobalResourceContainer>;

    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;

    SdGlobalResourceContainer (void);
    ~SdGlobalResourceContainer (void);
};

} // end of namespace sd

#endif
