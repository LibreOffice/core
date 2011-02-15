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
#ifndef DBAUI_TOKENWRITER_HXX
#define DBAUI_TOKENWRITER_HXX

#include "DExport.hxx"
#include "moduledbu.hxx"
#include "commontypes.hxx"

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
    // =========================================================================
    // ODatabaseImportExport Basisklasse f"ur Import/Export
    // =========================================================================
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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xFactory;

        ::rtl::OUString m_sName;
        //dyf add 20070601
        //for transfor the tablename
        ::rtl::OUString m_sDefaultTableName;
        //dyf add end
        ::rtl::OUString m_sDataSourceName;
        sal_Int32       m_nCommandType;
        bool            m_bNeedToReInitialize;

#if defined UNX
        static const char __FAR_DATA sNewLine;
#else
        static const char __FAR_DATA sNewLine[];
#endif

        ODatabaseExport*    m_pReader;
        sal_Int32*          m_pRowMarker; // wenn gesetzt, dann nur diese Rows kopieren
        rtl_TextEncoding    m_eDestEnc;
        sal_Bool            m_bInInitialize;
        sal_Bool            m_bCheckOnly;

        // export data
        ODatabaseImportExport(  const ::svx::ODataAccessDescriptor& _aDataDescriptor,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                                const String& rExchange = String());

        // import data
        ODatabaseImportExport(  const SharedConnection& _rxConnection,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        virtual ~ODatabaseImportExport();

        virtual void initialize();
    public:
        void setStream(SvStream* _pStream){  m_pStream = _pStream; }

        //dyf add 20070601
        //for set the tablename
        void setSTableName(const ::rtl::OUString &_sTableName){ m_sDefaultTableName = _sTableName; }
        //dyf add end

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

    // =========================================================================
    // RTF Im- und Export
    // =========================================================================

    class ORTFImportExport : public ODatabaseImportExport
    {
        void appendRow(::rtl::OString* pHorzChar,sal_Int32 _nColumnCount,sal_Int32& k,sal_Int32& kk);
    public:
        // export data
        ORTFImportExport(   const ::svx::ODataAccessDescriptor& _aDataDescriptor,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const String& rExchange = String())
                            : ODatabaseImportExport(_aDataDescriptor,_rM,_rxNumberF,rExchange) {};

        // import data
        ORTFImportExport(   const SharedConnection& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
                        : ODatabaseImportExport(_rxConnection,_rxNumberF,_rM)
        {}

        virtual sal_Bool Write();
        virtual sal_Bool Read();
    };
    // =========================================================================
    // HTML Im- und Export
    // =========================================================================
    #define SBA_HTML_FONTSIZES 7
    const sal_Int16 nIndentMax = 23;
    class OHTMLImportExport : public ODatabaseImportExport
    {
        // default HtmlFontSz[1-7]
        static const sal_Int16  nDefaultFontSize[SBA_HTML_FONTSIZES];
        // HtmlFontSz[1-7] in s*3.ini [user]
        static sal_Int16        nFontSize[SBA_HTML_FONTSIZES];
        static const sal_Int16  nCellSpacing;
        static const char __FAR_DATA sIndentSource[];
        char                    sIndent[nIndentMax+1];
        sal_Int16               m_nIndent;
    #ifdef DBG_UTIL
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
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const String& rExchange = String());
        // import data
        OHTMLImportExport(  const SharedConnection& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
                        : ODatabaseImportExport(_rxConnection,_rxNumberF,_rM)
        {}

        virtual sal_Bool Write();
        virtual sal_Bool Read();

    };
    // =========================================================================
    // normal RowSet Im- und Export
    // =========================================================================

    class ORowSetImportExport : public ODatabaseImportExport
    {
        OModuleClient       m_aModuleClient;
        ::std::vector<sal_Int32>    m_aColumnMapping;
        ::std::vector<sal_Int32>    m_aColumnTypes;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >    m_xTargetResultSetUpdate;   //
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate >          m_xTargetRowUpdate;         //
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xTargetResultSetMetaData; //
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
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                            const String& rExchange = String());

        // import data
        ORowSetImportExport(const SharedConnection& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
                        : ODatabaseImportExport(_rxConnection,NULL,_rM)
        {}

        virtual sal_Bool Write();
        virtual sal_Bool Read();

    private:
        using ODatabaseImportExport::initialize;
    };

}
#endif // DBAUI_TOKENWRITER_HXX



