/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConnectionLineData.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:14:51 $
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

        friend bool operator==(const OConnectionLineData& lhs, const OConnectionLineData& rhs);
        friend bool operator!=(const OConnectionLineData& lhs, const OConnectionLineData& rhs) { return !(lhs == rhs); }
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

        bool Reset();
        OConnectionLineData& operator=( const OConnectionLineData& rConnLineData );
    };

    //-------------------------------------------------------------------------
    //------------------------------------------------------------------
    typedef ::vos::ORef< OConnectionLineData >      OConnectionLineDataRef;
    typedef ::std::vector< OConnectionLineDataRef > OConnectionLineDataVec;
}
#endif // DBAUI_CONNECTIONLINEDATA_HXX

