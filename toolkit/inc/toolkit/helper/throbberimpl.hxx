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

#ifndef _TOOLKIT_HELPER_THROBBERIMPL_HXX_
#define _TOOLKIT_HELPER_THROBBERIMPL_HXX_

#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <vcl/timer.hxx>
#include <osl/mutex.hxx>

//........................................................................
namespace toolkit
//........................................................................
{
#define CSS_UNO     ::com::sun::star::uno
#define CSS_GRAPHIC ::com::sun::star::graphic

    class Throbber_Impl
    {
    private:
        CSS_UNO::Sequence< CSS_UNO::Reference< CSS_GRAPHIC::XGraphic > > maImageList;
        CSS_UNO::Reference< VCLXWindow > mxParent;

        sal_Bool    mbRepeat;
        sal_Int32   mnStepTime;
        sal_Int32   mnCurStep;
        sal_Int32   mnStepCount;
        AutoTimer   maWaitTimer;

        DECL_LINK( TimeOutHdl, Throbber_Impl* );

    public:
             Throbber_Impl( CSS_UNO::Reference< VCLXWindow > xParent,
                            sal_Int32 nStepTime,
                            sal_Bool bRepeat );
            ~Throbber_Impl();

        // Properties
        void setStepTime( sal_Int32 nStepTime ) { mnStepTime = nStepTime; }
        void setRepeat( sal_Bool bRepeat ) { mbRepeat = bRepeat; }

        // XSimpleAnimation
        void start() throw ( CSS_UNO::RuntimeException );
        void stop()  throw ( CSS_UNO::RuntimeException );
        void setImageList( const CSS_UNO::Sequence< CSS_UNO::Reference< CSS_GRAPHIC::XGraphic > >& ImageList )
                     throw ( CSS_UNO::RuntimeException );
        // Helpers
        void initImage() throw ( CSS_UNO::RuntimeException );
    };
//........................................................................
#undef CSS_UNO
#undef CSS_GRAPHIC

} // namespacetoolkit
//........................................................................

#endif //_TOOLKIT_HELPER_THROBBERIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
