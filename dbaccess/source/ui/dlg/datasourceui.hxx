/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datasourceui.hxx,v $
 * $Revision: 1.3.68.1 $
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
