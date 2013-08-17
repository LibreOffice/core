/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

    // ConnData     ---------->*    ConnLineData
    //    ^1                            ^1
    //    |                             |
    //  Conn        ---------->*    ConnLine

    /**
        the class OConnectionLineData contains the data of a connection
        e.g. the source and the destanation field
    **/
    class OConnectionLineData : public ::salhelper::SimpleReferenceObject
    {
        OUString m_aSourceFieldName;
        OUString m_aDestFieldName;

        friend bool operator==(const OConnectionLineData& lhs, const OConnectionLineData& rhs);
        friend bool operator!=(const OConnectionLineData& lhs, const OConnectionLineData& rhs) { return !(lhs == rhs); }
    protected:
        virtual ~OConnectionLineData();
    public:
        OConnectionLineData();
        OConnectionLineData( const OUString& rSourceFieldName, const OUString& rDestFieldName );
        OConnectionLineData( const OConnectionLineData& rConnLineData );
        // provide a copy of own instance (this is somehow more acceptable for me compared to a virtual assignment operator
        void CopyFrom(const OConnectionLineData& rSource);

        // member access (write)
        void SetFieldName(EConnectionSide nWhich, const OUString& strFieldName)
        {
            if (nWhich==JTCS_FROM)
                m_aSourceFieldName = strFieldName;
            else
                m_aDestFieldName = strFieldName;
        }
        void SetSourceFieldName( const OUString& rSourceFieldName){ SetFieldName(JTCS_FROM, rSourceFieldName); }
        void SetDestFieldName( const OUString& rDestFieldName ){ SetFieldName(JTCS_TO, rDestFieldName); }

        inline bool clearSourceFieldName() { SetSourceFieldName(OUString()); return true;}
        inline bool clearDestFieldName() { SetDestFieldName(OUString());     return true;}

        // member access (read)
        OUString GetFieldName(EConnectionSide nWhich) const { return (nWhich == JTCS_FROM) ? m_aSourceFieldName : m_aDestFieldName; }
        OUString GetSourceFieldName() const { return GetFieldName(JTCS_FROM); }
        OUString GetDestFieldName() const { return GetFieldName(JTCS_TO); }

        bool Reset();
        OConnectionLineData& operator=( const OConnectionLineData& rConnLineData );
    };

    typedef ::rtl::Reference< OConnectionLineData >     OConnectionLineDataRef;
    typedef ::std::vector< OConnectionLineDataRef > OConnectionLineDataVec;
}
#endif // DBAUI_CONNECTIONLINEDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
