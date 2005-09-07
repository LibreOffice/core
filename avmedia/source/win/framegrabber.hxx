/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: framegrabber.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:44:54 $
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

#ifndef _FRAMEGRABBER_HXX
#define _FRAMEGRABBER_HXX

#include "wincommon.hxx"

#ifndef _COM_SUN_STAR_MEDIA_XFRAMEGRABBER_HDL_
#include "com/sun/star/media/XFrameGrabber.hdl"
#endif

struct IMediaDet;

namespace avmedia { namespace win {

// ----------------
// - FrameGrabber -
// ----------------

class FrameGrabber : public ::cppu::WeakImplHelper2 < ::com::sun::star::media::XFrameGrabber,
                                                      ::com::sun::star::lang::XServiceInfo >
{
public:

            FrameGrabber( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
            ~FrameGrabber();

    bool    create( const ::rtl::OUString& rURL );

    // XFrameGrabber
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    mxMgr;
    ::rtl::OUString                                                                     maURL;

    IMediaDet* implCreateMediaDet( const ::rtl::OUString& rURL ) const;
};

} // namespace win
} // namespace avmedia

#endif // _FRAMEGRABBER_HXX
