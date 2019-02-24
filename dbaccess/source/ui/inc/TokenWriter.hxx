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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TOKENWRITER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TOKENWRITER_HXX

#include "DExport.hxx"
#include "commontypes.hxx"

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <svx/dataaccessdescriptor.hxx>

namespace com { namespace sun { namespace star {
    namespace sdbc{
        class XRowUpdate;
    }
}}}

class SvStream;

namespace dbaui
{
    // ODatabaseImportExport base class for import/export
    class ODatabaseExport;
    class ODatabaseImportExport : public ::cppu::WeakImplHelper< css::lang::XEventListener>
    {
    protected:
        css::uno::Sequence< css::uno::Any>                    m_aSelection;
        bool                                                  m_bBookmarkSelection;
        SvStream*                                             m_pStream;
        css::awt::FontDescriptor                              m_aFont;
        css::uno::Reference< css::beans::XPropertySet >       m_xObject;      // table/query
        SharedConnection                                                                m_xConnection;
        css::uno::Reference< css::sdbc::XResultSet >          m_xResultSet;
        css::uno::Reference< css::sdbc::XRow >                m_xRow;
        css::uno::Reference< css::sdbcx::XRowLocate >         m_xRowLocate;
        css::uno::Reference< css::sdbc::XResultSetMetaData >  m_xResultSetMetaData;
        css::uno::Reference< css::container::XIndexAccess >   m_xRowSetColumns;
        css::uno::Reference< css::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;

        OUString            m_sName;

        //for transfor the tablename
        OUString            m_sDefaultTableName;

        OUString            m_sDataSourceName;
        sal_Int32           m_nCommandType;
        bool                m_bNeedToReInitialize;

        rtl_TextEncoding    m_eDestEnc;
        bool                m_bInInitialize;
        bool                m_bCheckOnly;

        // export data
        ODatabaseImportExport(  const svx::ODataAccessDescriptor& _aDataDescriptor,
                                const css::uno::Reference< css::uno::XComponentContext >& _rM,
                                const css::uno::Reference< css::util::XNumberFormatter >& _rxNumberF);

        // import data
        ODatabaseImportExport(  const SharedConnection& _rxConnection,
                                const css::uno::Reference< css::util::XNumberFormatter >& _rxNumberF,
                                const css::uno::Reference< css::uno::XComponentContext >& _rM);

        virtual ~ODatabaseImportExport() override;

        virtual void initialize();
    public:
        void setStream(SvStream* _pStream){  m_pStream = _pStream; }

        //for set the tablename
        void setSTableName(const OUString &_sTableName){ m_sDefaultTableName = _sTableName; }

        virtual bool Write(); // Export
        virtual bool Read(); // Import

        void initialize(const svx::ODataAccessDescriptor& _aDataDescriptor);
        void dispose();

        void enableCheckOnly() { m_bCheckOnly = true; }
        bool isCheckEnabled() const { return m_bCheckOnly; }

    private:
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
        void impl_initFromDescriptor( const svx::ODataAccessDescriptor& _aDataDescriptor, bool _bPlusDefaultInit );
    };

    // RTF Import and Export

    class ORTFImportExport : public ODatabaseImportExport
    {
        void appendRow(OString const * pHorzChar,sal_Int32 _nColumnCount,sal_Int32& k,sal_Int32& kk);
    public:
        // export data
        ORTFImportExport(   const svx::ODataAccessDescriptor& _aDataDescriptor,
                            const css::uno::Reference< css::uno::XComponentContext >& _rM,
                            const css::uno::Reference< css::util::XNumberFormatter >& _rxNumberF)
                            : ODatabaseImportExport(_aDataDescriptor,_rM,_rxNumberF) {};

        // import data
        ORTFImportExport(   const SharedConnection& _rxConnection,
                            const css::uno::Reference< css::util::XNumberFormatter >& _rxNumberF,
                            const css::uno::Reference< css::uno::XComponentContext >& _rM)
                        : ODatabaseImportExport(_rxConnection,_rxNumberF,_rM)
        {}

        virtual bool Write() override;
        virtual bool Read() override;
    };
    // HTML Import and Export
    #define SBA_HTML_FONTSIZES 7
    const sal_Int16 nIndentMax = 23;
    class OHTMLImportExport : public ODatabaseImportExport
    {
        // default HtmlFontSz[1-7]
        static const sal_Int16  nDefaultFontSize[SBA_HTML_FONTSIZES];
        // HtmlFontSz[1-7] in s*3.ini [user]
        static sal_Int16        nFontSize[SBA_HTML_FONTSIZES];
        static const sal_Int16  nCellSpacing;
        static const char sIndentSource[];
        char                    sIndent[nIndentMax+1];
        sal_Int16               m_nIndent;
    #if OSL_DEBUG_LEVEL > 0
        bool                    m_bCheckFont;
    #endif

        void WriteHeader();
        void WriteBody();
        void WriteTables();
        void WriteCell( sal_Int32 nFormat,sal_Int32 nWidthPixel,sal_Int32 nHeightPixel,const char* pChar,const OUString& rValue,const char* pHtmlTag);
        void IncIndent( sal_Int16 nVal );
        const char*         GetIndentStr() { return sIndent; }
        void FontOn();
        inline void FontOff();

    public:
        // export data
        OHTMLImportExport(  const svx::ODataAccessDescriptor& _aDataDescriptor,
                            const css::uno::Reference< css::uno::XComponentContext >& _rM,
                            const css::uno::Reference< css::util::XNumberFormatter >& _rxNumberF);
        // import data
        OHTMLImportExport(  const SharedConnection& _rxConnection,
                            const css::uno::Reference< css::util::XNumberFormatter >& _rxNumberF,
                            const css::uno::Reference< css::uno::XComponentContext >& _rM)
            : ODatabaseImportExport(_rxConnection,_rxNumberF,_rM)
            , m_nIndent(0)
        {}

        virtual bool Write() override;
        virtual bool Read() override;

    };
    // normal RowSet Import and Export

    class ORowSetImportExport : public ODatabaseImportExport
    {
        std::vector<sal_Int32>    m_aColumnMapping;
        std::vector<sal_Int32>    m_aColumnTypes;
        css::uno::Reference< css::sdbc::XResultSetUpdate >    m_xTargetResultSetUpdate;
        css::uno::Reference< css::sdbc::XRowUpdate >          m_xTargetRowUpdate;
        css::uno::Reference< css::sdbc::XResultSetMetaData >  m_xTargetResultSetMetaData;
        VclPtr<vcl::Window>         m_pParent;
        bool                        m_bAlreadyAsked;

        bool insertNewRow();
    protected:
        virtual void initialize() override;

    public:
        // export data
        ORowSetImportExport(vcl::Window* _pParent,
                            const css::uno::Reference< css::sdbc::XResultSetUpdate >& _xResultSetUpdate,
                            const svx::ODataAccessDescriptor& _aDataDescriptor,
                            const css::uno::Reference< css::uno::XComponentContext >& _rM);

        virtual bool Write() override;
        virtual bool Read() override;

    private:
        using ODatabaseImportExport::initialize;
    };

}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TOKENWRITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
