/*************************************************************************
 *
 *  $RCSfile: ConnectionLineData.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:06:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#define DBAUI_CONNECTIONLINEDATA_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

namespace dbaui
{

    //==================================================================
    // ConnData     ---------->*    ConnLineData
    //    ^1                            ^1
    //    |                             |
    //  Conn        ---------->*    ConnLine
    //==================================================================


    //==================================================================
    /**
        the class OConnectionLineData contains the data of a connection
        e.g. the source and the destanation field
    **/
    class OConnectionLineData
    {
        ::rtl::OUString m_aSourceFieldName;
        ::rtl::OUString m_aDestFieldName;

    public:
        OConnectionLineData();
        OConnectionLineData( const ::rtl::OUString& rSourceFieldName, const ::rtl::OUString& rDestFieldName );
        OConnectionLineData( const OConnectionLineData& rConnLineData );
        virtual ~OConnectionLineData();

        // eine Kopie der eigenen Instanz liefern (das ist mir irgendwie angenehmer als ein virtueller Zuweisungsoperator)
        virtual void CopyFrom(const OConnectionLineData& rSource);

        // Memberzugriff (schreiben)
        void SetFieldName(EConnectionSide nWhich, const ::rtl::OUString& strFieldName)
        {
            if (nWhich==JTCS_FROM)
                m_aSourceFieldName = strFieldName;
            else
                m_aDestFieldName = strFieldName;
        }
        void SetSourceFieldName( const ::rtl::OUString& rSourceFieldName ){ SetFieldName(JTCS_FROM, rSourceFieldName); }
        void SetDestFieldName( const ::rtl::OUString& rDestFieldName ){ SetFieldName(JTCS_TO, rDestFieldName); }

        // Memberzugriff (lesen)
        ::rtl::OUString GetFieldName(EConnectionSide nWhich) const { return (nWhich == JTCS_FROM) ? m_aSourceFieldName : m_aDestFieldName; }
        ::rtl::OUString GetSourceFieldName() const { return GetFieldName(JTCS_FROM); }
        ::rtl::OUString GetDestFieldName() const { return GetFieldName(JTCS_TO); }

        virtual BOOL IsValid();
        virtual void Reset();
        virtual OConnectionLineData& operator=( const OConnectionLineData& rConnLineData );
    };

}
#endif // DBAUI_CONNECTIONLINEDATA_HXX

