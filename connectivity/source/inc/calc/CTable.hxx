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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CTABLE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CTABLE_HXX

#include "file/FTable.hxx"
#include <tools/date.hxx>

namespace com { namespace sun { namespace star { namespace sheet {
    class XSpreadsheet;
} } } }

namespace com { namespace sun { namespace star { namespace util {
    class XNumberFormats;
} } } }


namespace connectivity
{
    namespace calc
    {
        typedef file::OFileTable OCalcTable_BASE;
        class OCalcConnection;

        class OCalcTable :  public OCalcTable_BASE
        {
        private:
            ::std::vector<sal_Int32> m_aTypes;      // holds all type for columns just to avoid to ask the propertyset
            ::std::vector<sal_Int32> m_aPrecisions; // same as aboth
            ::std::vector<sal_Int32> m_aScales;
            ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >           m_xSheet;
            OCalcConnection* m_pConnection;
            sal_Int32 m_nStartCol;
            sal_Int32 m_nStartRow;
            sal_Int32 m_nDataCols;
            sal_Int32 m_nDataRows;
            bool      m_bHasHeaders;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > m_xFormats;
            ::Date m_aNullDate;

            void fillColumns();

        protected:
            virtual void FileClose() override;
        public:
            virtual void refreshColumns() override;
            virtual void refreshIndexes() override;

        public:
            OCalcTable( sdbcx::OCollection* _pTables,OCalcConnection* _pConnection,
                    const OUString& _Name,
                    const OUString& _Type,
                    const OUString& _Description = OUString(),
                    const OUString& _SchemaName = OUString(),
                    const OUString& _CatalogName = OUString()
                );

            virtual sal_Int32 getCurrentLastPos() const override;
            virtual bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos) override;
            virtual bool fetchRow(OValueRefRow& _rRow, const OSQLColumns& _rCols, bool _bUseTableDefs, bool bRetrieveData) override;

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL disposing() override;

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            void construct() override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
