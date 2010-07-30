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

#ifndef _FRAMEGRABBER_HXX
#define _FRAMEGRABBER_HXX

#include "gstplayer.hxx"
#include <gdk-pixbuf/gdk-pixbuf.h>

#ifndef _COM_SUN_STAR_MEDIA_XFRAMEGRABBER_HDL_
#include "com/sun/star/media/XFrameGrabber.hdl"
#endif

namespace avmedia { namespace gst {

// ----------------
// - FrameGrabber -
// ----------------

class FrameGrabber : public Player,
                     public ::cppu::WeakImplHelper2 < ::com::sun::star::media::XFrameGrabber,
                                                      ::com::sun::star::lang::XServiceInfo >
{
public:

    // static create method instead of public Ctor
    static FrameGrabber* create( const GString* pUri );

    ~FrameGrabber();

    // XFrameGrabber
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

protected: FrameGrabber( GString* pURI = NULL );

    virtual gboolean busCallback( GstBus* pBus,
                                  GstMessage* pMsg );

private: FrameGrabber( const FrameGrabber& );
    FrameGrabber& operator=( const FrameGrabber& );

    GMutex* mpFrameMutex;
    GCond* mpFrameCond;
    GdkPixbuf* mpLastPixbuf;
    double mfGrabTime;
    bool mbIsInGrabMode;
};

} // namespace gst
} // namespace avmedia

#endif // _FRAMEGRABBER_HXX
