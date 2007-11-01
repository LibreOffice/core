/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datasourceui.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:10:14 $
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

#ifndef DBACCESS_DATASOURCEUI_HXX
#define DBACCESS_DATASOURCEUI_HXX

#include "dsntypes.hxx"
#include "dsmeta.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <boost/shared_ptr.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= DataSourceUI
    //====================================================================
    /** encapsulates information about available UI features of a data source type
    */
    class DataSourceUI
    {
    public:
        DataSourceUI( DATASOURCE_TYPE _eType );
        DataSourceUI( const DataSourceMetaData& _rDSMeta );
        ~DataSourceUI();

        /** returns whether the data source's UI contains the specified setting

            Note that at the moment, not all items are supported by this method. In particular, use
            it for the following only
            <ul><li>All items which refer to advanced settings (see AdvancedSettingsSupport)</li>
            </ul>

            A complete support of *all* items is a medium-term goal.

            @param _nItemId
                the UI's item ID for the setting in question. See dsitems.hxx.
        */
        bool    hasSetting( const USHORT _nItemId ) const;

    private:
        DataSourceMetaData  m_aDSMeta;
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_DATASOURCEUI_HXX
