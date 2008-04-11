/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pagemanager.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_CANVAS_PAGEMANAGER_HXX
#define INCLUDED_CANVAS_PAGEMANAGER_HXX

#include <basegfx/vector/b2isize.hxx>
#include <canvas/rendering/irendermodule.hxx>
#include <canvas/rendering/isurface.hxx>

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
