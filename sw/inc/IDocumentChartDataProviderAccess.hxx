/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentChartDataProviderAccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: gm $ $Date: 2007-05-29 09:05:21 $
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

 #ifndef IDOCUMENTCHARTDATAPROVIDER_HXX_INCLUDED
 #define IDOCUMENTCHARTDATAPROVIDER_HXX_INCLUDED

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif

namespace com { namespace sun { namespace star { namespace frame {
    class XModel;
}}}}

class SwTable;
class SwChartDataProvider;
class SwChartLockController_Helper;


 /** Give access to the data-provider for chart
 */
 class IDocumentChartDataProviderAccess
 {
 public:

    /**
        returns or creates the data-provider for chart
    */
    virtual SwChartDataProvider * GetChartDataProvider( bool bCreate = false ) const = 0;

    /**
        calls createInternalDataProvider for all charts using the specified table
    */
    virtual void CreateChartInternalDataProviders( const SwTable *pTable ) = 0;

    /**
        returns chart controller helper that is used to lock/unlock all
        charts when relevant parts of tables are changed
    */
    virtual SwChartLockController_Helper & GetChartControllerHelper() = 0;

 protected:
    virtual ~IDocumentChartDataProviderAccess() {};
 };

 #endif // IDOCUMENTCHARTDATAPROVIDER_HXX_INCLUDED

