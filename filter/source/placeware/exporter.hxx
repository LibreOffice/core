/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: exporter.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:51:54 $
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
#ifndef _PLACEWARE_EXPORTER_HXX
#define _PLACEWARE_EXPORTER_HXX

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

class PageEntry;

class PlaceWareExporter
{
public:
    PlaceWareExporter( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~PlaceWareExporter();

    sal_Bool doExport( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xDoc,
                        ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xOutputStream,
                            const rtl::OUString& rURL,
                                ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xHandler,
                                ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator >& rxStatusIndicator );

private:
    PageEntry* exportPage( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >&xDrawPage );

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XExporter > mxGraphicExporter;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > mxInteractionHandler;
};

#endif
