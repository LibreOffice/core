/*************************************************************************
 *
 *  $RCSfile: TokenWriter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:10:01 $
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
#ifndef DBAUI_TOKENWRITER_HXX
#define DBAUI_TOKENWRITER_HXX

#ifndef DBAUI_DATABASEEXPORT_HXX
#include "DExport.hxx"
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif

namespace dbaui
{
    // =========================================================================
    // ODatabaseImportExport Basisklasse f"ur Import/Export
    // =========================================================================
    class ODatabaseExport;
    class ODatabaseImportExport : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener>
    {
        void initialize();
        void disposing();
    protected:
        SvStream*                               m_pStream;
        ::com::sun::star::awt::FontDescriptor   m_aFont;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xObject;      // table/query
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;  //
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >          m_xResultSet;   //
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >                m_xRow; //
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xResultSetMetaData;   //
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xFactory;

        ::rtl::OUString m_sName;
        ::rtl::OUString m_sDataSourceName;
        sal_Int32       m_nCommandType;
        sal_Bool        m_bDisposeConnection;

#if defined UNX || defined MAC
        static const char __FAR_DATA sNewLine;
#else
        static const char __FAR_DATA sNewLine[];
#endif
        ODatabaseExport*    m_pReader;
        sal_Int32*          m_pRowMarker; // wenn gesetzt, dann nur diese Rows kopieren

        // export data
        ODatabaseImportExport(  const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aSeq,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                                const String& rExchange = String());

        // import data
        ODatabaseImportExport(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
                                :m_xConnection(_rxConnection)
        ,m_pReader(NULL)
        ,m_pRowMarker(NULL)
        ,m_xFormatter(_rxNumberF)
        ,m_xFactory(_rM)
        ,m_nCommandType(::com::sun::star::sdb::CommandType::TABLE)
        ,m_bDisposeConnection(sal_False)
        {}

        virtual ~ODatabaseImportExport();
    public:
        void setStream(SvStream* _pStream){  m_pStream = _pStream; }

        virtual BOOL Write()    = 0; // Export
        virtual BOOL Read()     = 0; // Import

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    };

    // =========================================================================
    // RTF Im- und Export
    // =========================================================================

    class ORTFImportExport : public ODatabaseImportExport
    {

    public:
        // export data
        ORTFImportExport(   const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aSeq,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const String& rExchange = String())
                            : ODatabaseImportExport(_aSeq,_rM,_rxNumberF,rExchange) {};

        // import data
        ORTFImportExport(   const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
                        : ODatabaseImportExport(_rxConnection,_rxNumberF,_rM)
        {}

        virtual BOOL Write();
        virtual BOOL Read();
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
    #if DBG_UTIL
        BOOL                    m_bCheckFont;
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
        OHTMLImportExport(  const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aSeq,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const String& rExchange = String());
        // import data
        OHTMLImportExport(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
                        : ODatabaseImportExport(_rxConnection,_rxNumberF,_rM)
        {}

        virtual BOOL Write();
        virtual BOOL Read();

    };

}
#endif // DBAUI_TOKENWRITER_HXX



