/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: irendermodule.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_CANVAS_IRENDERMODULE_HXX
#define INCLUDED_CANVAS_IRENDERMODULE_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>


namespace basegfx
{
    class B2DRange;
    class B2IRange;
    class B2IVector;
    class B2IPoint;
}

namespace canvas
{
    struct ISurface;

    struct Vertex
    {
        float r,g,b,a;
        float u,v;
        float x,y,z;
    };

    /** Output module interface for backend render implementations.

        Implement this interface for each operating system- or
        library-specific rendering backend, which needs coupling with
        the canvas rendering framework (which can be shared between
        all backend implementations).
     */
    struct IRenderModule
    {
        /** Type of primitive passed to the render module via
            pushVertex()
         */
        enum PrimitiveType
        {
            PRIMITIVE_TYPE_UNKNOWN,
            PRIMITIVE_TYPE_TRIANGLE,
            PRIMITIVE_TYPE_QUAD
        };

        virtual ~IRenderModule() {}

        /// Lock rendermodule against concurrent access
        virtual void lock() const = 0;

        /// Unlock rendermodule for concurrent access
        virtual void unlock() const = 0;

        /** Maximal size of VRAM pages available

            This is typically the maximum texture size of the
            hardware, or some arbitrary limit if the backend is
            software.
         */
        virtual ::basegfx::B2IVector getPageSize() = 0;

        /** Create a (possibly hardware-accelerated) surface

            @return a pointer to a surface, which is an abstraction of
            a piece of (possibly hardware-accelerated) texture memory.
         */
        virtual ::boost::shared_ptr<ISurface> createSurface( const ::basegfx::B2IVector& surfaceSize ) = 0;

        /** Begin rendering the given primitive.

            Each beginPrimitive() call must be matched with an
            endPrimitive() call.
         */
        virtual void      beginPrimitive( PrimitiveType eType ) = 0;

        /** Finish rendering a primitive.

            Each beginPrimitive() call must be matched with an
            endPrimitive() call.
         */
        virtual void      endPrimitive() = 0;

        /** Add given vertex to current primitive

            After issuing a beginPrimitive(), each pushVertex() adds a
            vertex to the active primitive.
         */
        virtual void      pushVertex( const Vertex& vertex ) = 0;

        /** Query error status

            @returns true, if an error occured during primitive
            construction.
         */
        virtual bool      isError() = 0;
    };

    typedef ::boost::shared_ptr< IRenderModule > IRenderModuleSharedPtr;

    /// Little RAII wrapper for guarding access to IRenderModule interface
    class RenderModuleGuard : private ::boost::noncopyable
    {
    public:
        explicit RenderModuleGuard( const IRenderModuleSharedPtr& rRenderModule ) :
            mpRenderModule( rRenderModule )
        {
            mpRenderModule->lock();
        }

        ~RenderModuleGuard()
        {
            mpRenderModule->unlock();
        }

    private:
        const IRenderModuleSharedPtr mpRenderModule;
    };
}

#endif /* INCLUDED_CANVAS_IRENDERMODULE_HXX */
