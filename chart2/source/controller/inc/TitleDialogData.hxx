/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TitleDialogData.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:57:07 $
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
#ifndef _CHART2_TITLEDIALOGDATA_HXX
#define _CHART2_TITLEDIALOGDATA_HXX

#include "ReferenceSizeProvider.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

struct TitleDialogData
{
    ::com::sun::star::uno::Sequence< sal_Bool > aPossibilityList;
    ::com::sun::star::uno::Sequence< sal_Bool > aExistenceList;
    ::com::sun::star::uno::Sequence< rtl::OUString > aTextList;
    ::std::auto_ptr< ReferenceSizeProvider >    apReferenceSizeProvider;

    TitleDialogData( ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProvider =
                     ::std::auto_ptr< ReferenceSizeProvider >( 0 ));

    void readFromModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    /* return true if anything has changed;
    when pOldState is NULL then all data are written to the model
    */
    bool writeDifferenceToModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext
                        , TitleDialogData* pOldState=NULL );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
