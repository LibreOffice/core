/*************************************************************************
 *
 *  $RCSfile: ConnectionLineData.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2002-02-06 07:23:39 $
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

#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif
#include <vector>

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef DBAUI_REFFUNCTOR_HXX
#include "RefFunctor.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
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
    class OConnectionLineData : public ::vos::OReference
    {
        ::rtl::OUString m_aSourceFieldName;
        ::rtl::OUString m_aDestFieldName;

    protected:
        virtual ~OConnectionLineData();
    public:
        OConnectionLineData();
        OConnectionLineData( const ::rtl::OUString& rSourceFieldName, const ::rtl::OUString& rDestFieldName );
        OConnectionLineData( const OConnectionLineData& rConnLineData );

        // eine Kopie der eigenen Instanz liefern (das ist mir irgendwie angenehmer als ein virtueller Zuweisungsoperator)
        void CopyFrom(const OConnectionLineData& rSource);

        // Memberzugriff (schreiben)
        void SetFieldName(EConnectionSide nWhich, const ::rtl::OUString& strFieldName)
        {
            if (nWhich==JTCS_FROM)
                m_aSourceFieldName = strFieldName;
            else
                m_aDestFieldName = strFieldName;
        }
        void SetSourceFieldName( const ::rtl::OUString& rSourceFieldName){ SetFieldName(JTCS_FROM, rSourceFieldName); }
        void SetDestFieldName( const ::rtl::OUString& rDestFieldName ){ SetFieldName(JTCS_TO, rDestFieldName); }

        inline bool clearSourceFieldName() { SetSourceFieldName(::rtl::OUString()); return true;}
        inline bool clearDestFieldName() { SetDestFieldName(::rtl::OUString());     return true;}

        // Memberzugriff (lesen)
        ::rtl::OUString GetFieldName(EConnectionSide nWhich) const { return (nWhich == JTCS_FROM) ? m_aSourceFieldName : m_aDestFieldName; }
        ::rtl::OUString GetSourceFieldName() const { return GetFieldName(JTCS_FROM); }
        ::rtl::OUString GetDestFieldName() const { return GetFieldName(JTCS_TO); }

        bool IsValid() const;
        bool Reset();
        OConnectionLineData& operator=( const OConnectionLineData& rConnLineData );
    };

    //------------------------------------------------------------------
    typedef ::vos::ORef< OConnectionLineData >      OConnectionLineDataRef;
    typedef ::std::vector< OConnectionLineDataRef > OConnectionLineDataVec;
}
#endif // DBAUI_CONNECTIONLINEDATA_HXX

