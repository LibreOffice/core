/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#define DBAUI_CONNECTIONLINEDATA_HXX

#include "QEnumTypes.hxx"
#include <vector>

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include "RefFunctor.hxx"
#include <rtl/ustring.hxx>

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
    class OConnectionLineData : public ::salhelper::SimpleReferenceObject
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
    typedef ::rtl::Reference< OConnectionLineData >     OConnectionLineDataRef;
    typedef ::std::vector< OConnectionLineDataRef > OConnectionLineDataVec;
}
#endif // DBAUI_CONNECTIONLINEDATA_HXX

