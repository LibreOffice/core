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



#ifndef _FRAMEGRABBER_HXX
#define _FRAMEGRABBER_HXX

#include "gstplayer.hxx"
#include <gdk-pixbuf/gdk-pixbuf.h>

#ifndef _COM_SUN_STAR_MEDIA_XFRAMEGRABBER_HDL_
#include "com/sun/star/media/XFrameGrabber.hdl"
#endif
#include <cppuhelper/implbase1.hxx>

namespace avmedia { namespace gst {

// ----------------
// - FrameGrabber -
// ----------------

typedef ::cppu::ImplInheritanceHelper1
<    Player
,   ::com::sun::star::media::XFrameGrabber
> FrameGrabber_BASE;

class FrameGrabber
    : public FrameGrabber_BASE
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
