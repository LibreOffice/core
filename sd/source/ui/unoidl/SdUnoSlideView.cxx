/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdUnoSlideView.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:46:47 $
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

#include "SdUnoSlideView.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshcol.hxx>

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_SLIDE_VIEW_SHELL_HXX
#include "SlideViewShell.hxx"
#endif
#include "unohelp.hxx"
#include "unopage.hxx"
#include "unomodel.hxx"
#include "drawdoc.hxx"
/*
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#include "PresentationViewShell.hxx"
#endif
#ifndef SD_PREVIEW_VIEW_SHELL_HXX
#include "PreviewViewShell.hxx"
#endif
*/
using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {


SdUnoSlideView::SdUnoSlideView (
    ViewShellBase& rBase,
    ViewShell& rViewShell,
    View& rView) throw()
    : DrawController(rBase, rViewShell, rView)
{
}

SdUnoSlideView::~SdUnoSlideView() throw()
{
}




// XTypeProvider

IMPLEMENT_GET_IMPLEMENTATION_ID(SdUnoSlideView);




// XServiceInfo

sal_Char pImplSdUnoSlideViewService[sizeof("com.sun.star.presentation.SlidesView")] = "com.sun.star.presentation.SlidesView";

OUString SAL_CALL SdUnoSlideView::getImplementationName(  ) throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdUnoSlideView" ) );
}



} // end of namespace sd
