/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SVTOOLS_EMBEDHLP_HXX
#define _SVTOOLS_EMBEDHLP_HXX

#include "svtools/svtdllapi.h"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <vcl/graph.hxx>
#include <tools/mapunit.hxx>
#include <rtl/ustring.hxx>

#define NS_UNO ::com::sun::star::uno
#define NS_EMBED ::com::sun::star::embed
#define NS_IO ::com::sun::star::io

namespace comphelper
{
    class EmbeddedObjectContainer;
}

class Rectangle;
class OutputDevice;
class String;
namespace svt
{
    struct EmbeddedObjectRef_Impl;
    class SVT_DLLPUBLIC EmbeddedObjectRef
    {
        EmbeddedObjectRef_Impl*  mpImp;
        NS_UNO::Reference < NS_EMBED::XEmbeddedObject > mxObj;

        SVT_DLLPRIVATE SvStream*   GetGraphicStream( sal_Bool bUpdate ) const;
        /* SVT_DLLPRIVATE */ void        GetReplacement( sal_Bool bUpdate );
        SVT_DLLPRIVATE void        Construct_Impl();

        EmbeddedObjectRef& operator = ( const EmbeddedObjectRef& );

    public:
        const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >& operator ->() const { return mxObj; }
        const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >& GetObject() const { return mxObj; }

        static void DrawPaintReplacement( const Rectangle &rRect, const String &rText, OutputDevice *pOut );
        static void DrawShading( const Rectangle &rRect, OutputDevice *pOut );
        static sal_Bool TryRunningState( const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >& );
        static void SetGraphicToContainer( const Graphic& rGraphic,
                                            comphelper::EmbeddedObjectContainer& aContainer,
                                            const ::rtl::OUString& aName,
                                            const ::rtl::OUString& aMediaType );

        static sal_Bool ObjectIsModified( const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >& )
                                throw( NS_UNO::Exception );
        static NS_UNO::Reference< NS_IO::XInputStream > GetGraphicReplacementStream(
                                            sal_Int64 nViewAspect,
                                            const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >&,
                                            ::rtl::OUString* pMediaType )
                                throw();

        // default constructed object; needs further assignment before it can be used
        EmbeddedObjectRef();

        // assign a previously default constructed object
        void Assign( const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >& xObj, sal_Int64 nAspect );

        // create object for a certain view aspect
        EmbeddedObjectRef( const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >& xObj, sal_Int64 nAspect );

        ~EmbeddedObjectRef();
        EmbeddedObjectRef( const EmbeddedObjectRef& );
        sal_Bool TryRunningState();

        // assigning to a container enables the object to exchange graphical representations with a storage
        void            AssignToContainer( comphelper::EmbeddedObjectContainer* pContainer, const ::rtl::OUString& rPersistName );
        comphelper::EmbeddedObjectContainer* GetContainer() const;

        ::rtl::OUString GetPersistName() const;
        sal_Int64       GetViewAspect() const;
        void            SetViewAspect( sal_Int64 nAspect );
        Graphic*        GetGraphic( ::rtl::OUString* pMediaType=0 ) const;

        // the original size of the object ( size of the icon for iconified object )
        // no conversion is done if no target mode is provided
        Size            GetSize( MapMode* pTargetMapMode = NULL ) const;

        // the following method tries to get the HC graphic if it is possible, otherwise returns NULL
        Graphic*        GetHCGraphic() const;

        void            SetGraphic( const Graphic& rGraphic, const ::rtl::OUString& rMediaType );
        void            SetGraphicStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInGrStream,
                            const ::rtl::OUString& rMediaType );

        void            UpdateReplacement() { GetReplacement( sal_True ); }
        void            UpdateReplacementOnDemand();
        MapUnit         GetMapUnit() const;
        void            Lock( sal_Bool bLock = sal_True );
        sal_Bool            IsLocked() const;
        void            Clear();
        sal_Bool            is() const { return mxObj.is(); }

        sal_Bool            IsChart() const;

        // #i104867#
        // Provides a graphic version number for the fetchable Graphic during this object's lifetime. Internally,
        // that number is incremented at each change of the Graphic. This mechanism is needed to identify if a
        // remembered Graphic (e.g. primitives) has changed compared to the current one, but without actively
        // fetching the Graphic what would be too expensive e.g. for charts
        sal_uInt32 getGraphicVersion() const;
        void            SetDefaultSizeForChart( const Size& rSizeIn_100TH_MM );//#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this method
    };
}

#endif
