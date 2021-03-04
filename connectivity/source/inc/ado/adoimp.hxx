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
#pragma once

#include <com/sun/star/sdbc/SQLException.hpp>

#include <adoctint.h>

struct ADOConnection;

namespace connectivity::ado
{

        class WpADOField;
        class OLEString;
        class ADOS
        {
        public:
            // Also here: Free BSTR with SysFreeString()!
            static OLEString& GetKeyStr();

            static const CLSID  CLSID_ADOCATALOG_25;
            static const IID    IID_ADOCATALOG_25;

            static const CLSID  CLSID_ADOCONNECTION_21;
            static const IID    IID_ADOCONNECTION_21;

            static const CLSID  CLSID_ADOCOMMAND_21;
            static const IID    IID_ADOCOMMAND_21;

            static const CLSID  CLSID_ADORECORDSET_21;
            static const IID    IID_ADORECORDSET_21;

            static const CLSID  CLSID_ADOINDEX_25;
            static const IID    IID_ADOINDEX_25;

            static const CLSID  CLSID_ADOCOLUMN_25;
            static const IID    IID_ADOCOLUMN_25;

            static const CLSID  CLSID_ADOKEY_25;
            static const IID    IID_ADOKEY_25;

            static const CLSID  CLSID_ADOTABLE_25;
            static const IID    IID_ADOTABLE_25;

            static const CLSID  CLSID_ADOGROUP_25;
            static const IID    IID_ADOGROUP_25;

            static const CLSID  CLSID_ADOUSER_25;
            static const IID    IID_ADOUSER_25;

            static const CLSID  CLSID_ADOVIEW_25;
            static const IID    IID_ADOVIEW_25;

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static void ThrowException(ADOConnection* _pAdoCon,const css::uno::Reference< css::uno::XInterface >& _xInterface);
            static sal_Int32 MapADOType2Jdbc(DataTypeEnum eType);
            static DataTypeEnum MapJdbc2ADOType(sal_Int32 _nType,sal_Int32 _nJetEngine);
            static bool isJetEngine(sal_Int32 _nEngineType);

            static ObjectTypeEnum   mapObjectType2Ado(sal_Int32 objType);
            static sal_Int32        mapAdoType2Object(ObjectTypeEnum objType);
            static sal_Int32        mapAdoRights2Sdbc(RightsEnum eRights);
            static sal_Int32        mapRights2Ado(sal_Int32 nRights);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static WpADOField       getField(ADORecordset* _pRecordSet,sal_Int32 _nColumnIndex);
        };


}


#define ADO_PROP(ItemName)                                          \
        WpADOProperty aProp(aProps.GetItem(ItemName));              \
        OLEVariant aVar;                                            \
        if(aProp.IsValid())                                         \
            aVar = aProp.GetValue();                                \
        else                                                        \
            ADOS::ThrowException(*m_pADOConnection,*this);



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
