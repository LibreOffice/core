/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagecollector.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 13:59:19 $
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

#ifndef PAGECOLLECTOR_HXX
#define PAGECOLLECTOR_HXX

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#include <vector>

class PageCollector
{
    public:

    struct MasterPageEntity
    {
        com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > xMasterPage;
        sal_Bool bUsed;
    };
    static void CollectCustomShowPages( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >&, const rtl::OUString& rCustomShow, std::vector< com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > >& );
    static void CollectNonCustomShowPages( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >&, const rtl::OUString& rCustomShow, std::vector< com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > >& );
    static void CollectMasterPages( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >&, std::vector< MasterPageEntity >& );
};

#endif // PAGECOLLECTOR_HXX
