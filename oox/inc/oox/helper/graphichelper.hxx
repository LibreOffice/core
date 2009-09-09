/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: graphichelper.hxx,v $
 * $Revision: 1.1 $
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

#ifndef OOX_HELPER_GRAPHICHELPER_HXX
#define OOX_HELPER_GRAPHICHELPER_HXX

#include <deque>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
    namespace lang { class XMultiServiceFactory; }
    namespace io { class XInputStream; }
    namespace graphic { class XGraphic; }
    namespace graphic { class XGraphicObject; }
    namespace graphic { class XGraphicProvider; }
} } }

namespace oox {

// ============================================================================

/** Provides helper functions for graphics and graphic objects handling.

    All createGraphicObject() and importGraphicObject() functions create
    persistent graphic objects internally and store them in an internal
    container to prevent their early destruction. This makes it possible to use
    the returned URL of the graphic object in any way (e.g. insert it into a
    property map) without needing to store it immediatly at an object that
    resolves the graphic object from the passed URL and thus prevents it from
    being destroyed.
 */
class GraphicHelper
{
public:
    explicit            GraphicHelper(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );
                        ~GraphicHelper();

    /** Imports a graphic from the passed input stream. */
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                        importGraphic(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm );

    /** Imports a graphic from the passed binary memory block. */
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                        importGraphic( const StreamDataSequence& rGraphicData );

    /** Creates a persistent graphic object from the passed graphic.
        @return  The URL of the created and internally cached graphic object. */
    ::rtl::OUString     createGraphicObject(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rxGraphic );

    /** Creates a persistent graphic object from the passed input stream.
        @return  The URL of the created and internally cached graphic object. */
    ::rtl::OUString     importGraphicObject(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm );

    /** Creates a persistent graphic object from the passed binary memory block.
        @return  The URL of the created and internally cached graphic object. */
    ::rtl::OUString     importGraphicObject( const StreamDataSequence& rGraphicData );

private:
    typedef ::std::deque< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicObject > > GraphicObjectDeque;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxCompContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicProvider > mxGraphicProvider;
    GraphicObjectDeque  maGraphicObjects;
    const ::rtl::OUString maGraphicObjScheme;
};

// ============================================================================

} // namespace oox

#endif

