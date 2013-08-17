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
#ifndef DBAUI_TOKENWRITER_HXX
#define DBAUI_TOKENWRITER_HXX

#include "DExport.hxx"
#include "moduledbu.hxx"
#include "commontypes.hxx"

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>

#include <cppuhelper/implbase1.hxx>
#include <tools/stream.hxx>
#include <svx/dataaccessdescriptor.hxx>

#include <memory>

namespace com { namespace sun { namespace star {
    namespace sdbc{
        class XRowUpdate;
    }
}}}

namespace dbaui
{
    // ODatabaseImportExport base class for import/export
    class ODatabaseExport;
    typedef ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener> ODatabaseImportExport_BASE;
    class ODatabaseImportExport : public ODatabaseImportExport_BASE
    {
    private:
        void impl_initializeRowMember_throw();

    protected:
        ::com::sun::star::lang::Locale                                                  m_aLocale;
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>                    m_aSelection;
        sal_Bool                                                                        m_bBookmarkSelection;
        SvStream*                                                                       m_pStream;
        ::com::sun::star::awt::FontDescriptor                                           m_aFont;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xObject;      // table/query
        SharedConnection                                                                m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >          m_xResultSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >                m_xRow;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate >         m_xRowLocate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xResultSetMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >   m_xRowSetColumns;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;

        OUString m_sName;

        //for transfor the tablename
        OUString m_sDefaultTableName;

        OUString m_sDataSourceName;
        sal_Int32       m_nCommandType;
        bool            m_bNeedToReInitialize;

#if defined UNX
        static const char sNewLine;
#else
        static const char sNewLine[];
#endif

        ODatabaseExport*    m_pReader;
        sal_Int32*          m_pRowMarker; // if set, then copy only these rows
        rtl_TextEncoding    m_eDestEnc;
        sal_Bool            m_bInInitialize;
        sal_Bool            m_bCheckOnly;

        // export data
        ODatabaseImportExport(  const ::svx::ODataAccessDescriptor& _aDataDescriptor,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                                const String& rExchange = String());

        // import data
        ODatabaseImportExport(  const SharedConnection& _rxConnection,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM);

        virtual ~ODatabaseImportExport();

        virtual void initialize();
    public:
        void setStream(SvStream* _pStream){  m_pStream = _pStream; }

        //for set the tablename
        void setSTableName(const OUString &_sTableName){ m_sDefaultTableName = _sTableName; }

        virtual sal_Bool Write(); // Export
        virtual sal_Bool Read(); // Import

        void initialize(const ::svx::ODataAccessDescriptor& _aDataDescriptor);
        void dispose();

        void enableCheckOnly() { m_bCheckOnly = sal_True; }
        sal_Bool isCheckEnabled() const { return m_bCheckOnly; }

    private:
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
        void impl_initFromDescriptor( const ::svx::ODataAccessDescriptor& _aDataDescriptor, bool _bPlusDefaultInit );
    };

    // RTF Import and Export

    class ORTFImportExport : public ODatabaseImportExport
    {
        void appendRow(OString* pHorzChar,sal_Int32 _nColumnCount,sal_Int32& k,sal_Int32& kk);
    public:
        // export data
        ORTFImportExport(   const ::svx::ODataAccessDescriptor& _aDataDescriptor,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const String& rExchange = String())
                            : ODatabaseImportExport(_aDataDescriptor,_rM,_rxNumberF,rExchange) {};

        // import data
        ORTFImportExport(   const SharedConnection& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM)
                        : ODatabaseImportExport(_rxConnection,_rxNumberF,_rM)
        {}

        virtual sal_Bool Write();
        virtual sal_Bool Read();
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
        sal_Bool                    m_bCheckFont;
    #endif

        void WriteHeader();
        void WriteBody();
        void WriteTables();
        void WriteCell( sal_Int32 nFormat,sal_Int32 nWidthPixel,sal_Int32 nHeightPixel,const char* pChar,const String& rValue,const char* pHtmlTag);
        void IncIndent( sal_Int16 nVal );
        const char*         GetIndentStr() { return sIndent; }
        void FontOn();
        inline void FontOff();

    public:
        // export data
        OHTMLImportExport(  const ::svx::ODataAccessDescriptor& _aDataDescriptor,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const String& rExchange = String());
        // import data
        OHTMLImportExport(  const SharedConnection& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM)
                        : ODatabaseImportExport(_rxConnection,_rxNumberF,_rM)
        {}

        virtual sal_Bool Write();
        virtual sal_Bool Read();

    };
    // normal RowSet Import and Export

    class ORowSetImportExport : public ODatabaseImportExport
    {
        OModuleClient       m_aModuleClient;
        ::std::vector<sal_Int32>    m_aColumnMapping;
        ::std::vector<sal_Int32>    m_aColumnTypes;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >    m_xTargetResultSetUpdate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate >          m_xTargetRowUpdate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xTargetResultSetMetaData;
        Window*                     m_pParent;
        sal_Bool                    m_bAlreadyAsked;

        sal_Bool insertNewRow();
    protected:
        virtual void initialize();

    public:
        // export data
        ORowSetImportExport(Window* _pParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >& _xResultSetUpdate,
                            const ::svx::ODataAccessDescriptor& _aDataDescriptor,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM,
                            const String& rExchange = String());

        // import data
        ORowSetImportExport(const SharedConnection& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM)
                        : ODatabaseImportExport(_rxConnection,NULL,_rM)
        {}

        virtual sal_Bool Write();
        virtual sal_Bool Read();

    private:
        using ODatabaseImportExport::initialize;
    };

}
#endif // DBAUI_TOKENWRITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
