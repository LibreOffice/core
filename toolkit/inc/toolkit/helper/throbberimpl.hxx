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

#ifndef _TOOLKIT_HELPER_THROBBERIMPL_HXX_
#define _TOOLKIT_HELPER_THROBBERIMPL_HXX_

#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <vcl/timer.hxx>
#include <vos/mutex.hxx>

//........................................................................
namespace toolkit
//........................................................................
{

    class Throbber_Impl
    {
    private:
        vos::IMutex&    mrMutex;    // Reference to SolarMutex
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > > maImageList;
        VCLXWindow&     mrParent;

        sal_Bool    mbRepeat;
        sal_Int32   mnStepTime;
        sal_Int32   mnCurStep;
        sal_Int32   mnStepCount;
        AutoTimer   maWaitTimer;

        DECL_LINK( TimeOutHdl, Throbber_Impl* );

        vos::IMutex&    GetMutex() { return mrMutex; }

    public:
             Throbber_Impl( VCLXWindow& i_rParent );
            ~Throbber_Impl();

        // Properties
        void            setStepTime( sal_Int32 nStepTime )  { mnStepTime = nStepTime; }
        sal_Int32       getStepTime() const                 { return mnStepTime; }

        void            setRepeat( sal_Bool bRepeat )       { mbRepeat = bRepeat; }
        sal_Bool        getRepeat() const                   { return mbRepeat; }

        Window const*   getWindow() const { return mrParent.GetWindow(); }
              Window*   getWindow()       { return mrParent.GetWindow(); }

        // XSimpleAnimation
        void start();
        void stop();
        bool isRunning() const;
        void setImageList( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& ImageList );

        // Helpers
        sal_Bool isHCMode();
    };

//........................................................................
} // namespacetoolkit
//........................................................................

#endif //_TOOLKIT_HELPER_THROBBERIMPL_HXX_

