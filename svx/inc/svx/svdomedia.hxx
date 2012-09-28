/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SVDOMEDIA_HXX
#define SVDOMEDIA_HXX

#include <svx/svdorect.hxx>
#include <avmedia/mediaitem.hxx>
#include "svx/svxdllapi.h"

class Graphic;

namespace sdr { namespace contact { class ViewContactOfSdrMediaObj; } }

// ---------------
// - SdrMediaObj -
// ---------------

class SVX_DLLPUBLIC SdrMediaObj : public SdrRectObj
{
    friend class ::sdr::contact::ViewContactOfSdrMediaObj;

public:

                                    TYPEINFO();

                                    SdrMediaObj();
                                    SdrMediaObj( const Rectangle& rRect );

        virtual                     ~SdrMediaObj();

        virtual bool                HasTextEdit() const;

        virtual void                TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
        virtual sal_uInt16              GetObjIdentifier() const;

        virtual void                TakeObjNameSingul(String& rName) const;
        virtual void                TakeObjNamePlural(String& rName) const;

        virtual SdrMediaObj*            Clone() const;
        SdrMediaObj&                operator=(const SdrMediaObj& rObj);

        virtual void                AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly = false );

public:

        void                        setURL( const ::rtl::OUString& rURL );
        const ::rtl::OUString&      getURL() const;

        void                        setMediaProperties( const ::avmedia::MediaItem& rState );
        const ::avmedia::MediaItem& getMediaProperties() const;

        Size                        getPreferredSize() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                                    getSnapshot();
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>
                                    GetInputStream();

protected:

        virtual void                mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState );
        virtual ::sdr::contact::ViewContact* CreateObjectSpecificViewContact();

private:
        struct Impl;
        ::boost::scoped_ptr<Impl> m_pImpl;
};

#endif // SVDOMEDIA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
