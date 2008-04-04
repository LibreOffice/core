/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: throbberimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:54:30 $
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

#ifndef _TOOLKIT_HELPER_THROBBERIMPL_HXX_
#define _TOOLKIT_HELPER_THROBBERIMPL_HXX_

#ifndef  _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#include <com/sun/star/graphic/XGraphic.hpp>

#ifndef   _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

//........................................................................
namespace toolkit
//........................................................................
{

    class Throbber_Impl
    {
    private:
        NAMESPACE_VOS(IMutex)&  mrMutex;    // Reference to SolarMutex
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > > maImageList;
        ::com::sun::star::uno::Reference< VCLXWindow > mxParent;

        sal_Bool    mbRepeat;
        sal_Int32   mnStepTime;
        sal_Int32   mnCurStep;
        sal_Int32   mnStepCount;
        AutoTimer   maWaitTimer;

        DECL_LINK( TimeOutHdl, Throbber_Impl* );

        NAMESPACE_VOS(IMutex)&  GetMutex() { return mrMutex; }

    public:
             Throbber_Impl( ::com::sun::star::uno::Reference< VCLXWindow > xParent,
                            sal_Int32 nStepTime,
                            sal_Bool bRepeat );
            ~Throbber_Impl();

        // Properties
        void setStepTime( sal_Int32 nStepTime ) { mnStepTime = nStepTime; }
        void setRepeat( sal_Bool bRepeat ) { mbRepeat = bRepeat; }

        // XSimpleAnimation
        void start() throw ( ::com::sun::star::uno::RuntimeException );
        void stop() throw ( ::com::sun::star::uno::RuntimeException );
        void setImageList( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& ImageList )
                    throw ( ::com::sun::star::uno::RuntimeException );
        // Helpers
        void initImage() throw ( ::com::sun::star::uno::RuntimeException );
        sal_Bool isHCMode() throw ( ::com::sun::star::uno::RuntimeException );
    };
//........................................................................
} // namespacetoolkit
//........................................................................

#endif //_TOOLKIT_HELPER_THROBBERIMPL_HXX_

