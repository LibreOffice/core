/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: irendermodule.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:45:24 $
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

#ifndef INCLUDED_CANVAS_IRENDERMODULE_HXX
#define INCLUDED_CANVAS_IRENDERMODULE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
            PRIMITIVE_TYPE_UNKNONWN,
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
