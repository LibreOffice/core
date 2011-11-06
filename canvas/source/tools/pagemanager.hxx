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
