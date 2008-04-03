/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdGlobalResourceContainer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:05:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_GLOBAL_RESOURCE_CONTAINER_HXX
#define SD_GLOBAL_RESOURCE_CONTAINER_HXX

#include "sdmod.hxx"
#include <memory>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/XInterface.hpp>

namespace css = ::com::sun::star;

namespace sd {

class SdGlobalResource
{
public:
    virtual ~SdGlobalResource (void) {};
};

/** The purpose of this container is to hold references to resources that
    are globally available to all interested objects and to destroy them
    when the sd module is destroyed.  Examples for resources can be
    containers of bitmaps or the container of master pages used by the
    MasterPagesSelector objects in the task panel.

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

        When in doubt, use the shared_ptr variant of this method.
    */
    void AddResource (::std::auto_ptr<SdGlobalResource> pResource);

    /** Add a resource to the container.  By using a shared_ptr and
        releasing it only when the SgGlobalResourceContainer is destroyed
        the given resource is kept alive at least that long.  When at the
        time of the destruction of SgGlobalResourceContainer no other
        references exist the resource is destroyed as well.
    */
    void AddResource (::boost::shared_ptr<SdGlobalResource> pResource);

    /** Add a resource that is implemented as UNO object.  Destruction
        (when the sd modules is unloaded) is done by a) calling dispose()
        when the XComponent is supported and by b) releasing the reference.
    */
    void AddResource (const ::css::uno::Reference<css::uno::XInterface>& rxResource);

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
