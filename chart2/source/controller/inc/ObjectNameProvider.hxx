/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectNameProvider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:55:06 $
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

#ifndef _CHART2_OBJECTNAME_PROVIDER_HXX
#define _CHART2_OBJECTNAME_PROVIDER_HXX

#include "ObjectIdentifier.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/** Provides localized ui strings for the userinterface.
*/

class ObjectNameProvider
{
public:
    static rtl::OUString getName( ObjectType eObjectType, bool bPlural=false );
    static rtl::OUString getAxisName( const rtl::OUString& rObjectCID
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xChartModel  );
    static rtl::OUString getGridName( const rtl::OUString& rObjectCID
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xChartModel );
    static rtl::OUString getTitleName( const rtl::OUString& rObjectCID
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xChartModel );

    static rtl::OUString getNameForCID(
        const rtl::OUString& rObjectCID,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >& xChartDocument );

    /** Provides help texts for the various chart elements.
        The parameter rObjectCID has to be a ClassifiedIdentifier - see class ObjectIdentifier.
     */
    static rtl::OUString getHelpText( const rtl::OUString& rObjectCID, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel, bool bVerbose=false );
    static rtl::OUString getHelpText( const rtl::OUString& rObjectCID, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDocument, bool bVerbose=false );

    /** This is used for showing the currently selected object in the status bar
        (command "Context")
     */
    static rtl::OUString getSelectedObjectText( const rtl::OUString & rObjectCID, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDocument );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
