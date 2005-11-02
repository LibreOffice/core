/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagemanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:54:32 $
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

#ifndef INCLUDED_CANVAS_PAGEMANAGER_HXX
#define INCLUDED_CANVAS_PAGEMANAGER_HXX

#ifndef _BGFX_VECTOR_B2ISIZE_HXX
#include <basegfx/vector/b2isize.hxx>
#endif
#ifndef INCLUDED_CANVAS_IRENDERMODULE_HXX
#include <canvas/rendering/irendermodule.hxx>
#endif
#ifndef INCLUDED_CANVAS_ISURFACE_HXX
#include <canvas/rendering/isurface.hxx>
#endif

#include "page.hxx"

namespace canvas
{

    //////////////////////////////////////////////////////////////////////////////////
    // PageManager
    //////////////////////////////////////////////////////////////////////////////////

    class PageManager
    {
    public:
        PageManager( const canvas::IRenderModuleSharedPtr pRenderModule ) :
            mpRenderModule(pRenderModule)
        {
        }

        // returns the maximum size of a hardware
        // accelerated page, e.g. OpenGL texture.
        ::basegfx::B2ISize getPageSize() const;

        canvas::IRenderModuleSharedPtr getRenderModule() const;

        FragmentSharedPtr allocateSpace( const ::basegfx::B2ISize& rSize );
        void              free( const FragmentSharedPtr& pFragment );

        void              nakedFragment( const FragmentSharedPtr& pFragment );

        void              validatePages();

    private:
        // the pagemanager needs access to the rendermodule
        // since we query for system resources from it.
        canvas::IRenderModuleSharedPtr mpRenderModule;

        // here we collect all fragments that will be created
        // since we need them for relocation purposes.
        typedef std::list<FragmentSharedPtr> FragmentContainer_t;
        FragmentContainer_t maFragments;

        // this is the container holding all created pages,
        // behind the scenes these are real hardware surfaces.
        typedef std::list<PageSharedPtr> PageContainer_t;
        PageContainer_t maPages;

        bool relocate( const FragmentSharedPtr& pFragment );
    };

    //////////////////////////////////////////////////////////////////////////////////
    // PageManagerSharedPtr
    //////////////////////////////////////////////////////////////////////////////////

    typedef ::boost::shared_ptr< PageManager > PageManagerSharedPtr;

    //////////////////////////////////////////////////////////////////////////////////
    // End of file
    //////////////////////////////////////////////////////////////////////////////////
}

#endif
