/*************************************************************************
 *
 *  $RCSfile: DExport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 15:02:06 $
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
#ifndef DBAUI_DATABASEEXPORT_HXX
#define DBAUI_DATABASEEXPORT_HXX

#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif

namespace dbaui
{
    class OFieldDescription;
    class OTypeInfo;
    class ODatabaseExport
    {
    public:
        DECLARE_STL_MAP(::rtl::OUString,OFieldDescription*,::comphelper::UStringMixLess,TColumns);
        typedef ::std::vector<TColumns::iterator> TColumnVector;
    protected:
        ::std::vector<sal_Int32>        m_vColumns;     // Welche Spalten "ubernommen werden sollen
        ::std::vector<sal_Int32>        m_vColumnTypes; // FeldTypen f"ur schnelleren Zugriff
        ::std::vector<sal_Int32>        m_vColumnSize;
        ::std::vector<sal_Int32>        m_vFormatKey;
        ::com::sun::star::lang::Locale  m_nLocale;


        TColumns        m_aDestColumns; // container for new created columns
        TColumnVector   m_vDestVector;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xColumns;     // container
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;       // dest table
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xTables;      // container
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;  // dest conn

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >          m_xResultSet;   //
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >    m_xResultSetUpdate; //
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xResultSetMetaData;   //
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate >          m_xRowUpdate;   //
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xFactory;

        String              m_sTextToken;       // Zellen Inhalt
        OTypeInfo*          m_pTypeInfo;    // contains the default type
        sal_Int32           m_nColumnPos;       // aktuelle Spaltenposition
        sal_Int32           m_nRows;        // Anzahl der Zeilen die durchsucht werden sollen
        sal_Int32           m_nRowCount;    // current count of rows
        rtl_TextEncoding    m_nDefToken;        // Sprache
        sal_Bool            m_bError;           // Fehler und Abbruchstatus
        sal_Bool            m_bInTbl;           // Ist gesetzt, wenn der Parser sich in der RTF Tabelle befindet
        sal_Bool            m_bHead;        // ist true, wenn die Kopfzeile noch nicht gelesen wurde
        sal_Bool            m_bDontAskAgain;// Falls beim Einf"ugen ein Fehler auftritt, soll die Fehlermeldung nicht
        sal_Bool            m_bIsAutoIncrement; // if PKey is set by user


        virtual sal_Bool    CreateTable(int nToken) = 0;
        void                CreateDefaultColumn(const ::rtl::OUString& _rColumnName);
        sal_Int32           CheckString(const String& aToken, sal_Int32 _nOldFormat);
        void                insertValueIntoColumn();
        sal_Bool            createRowSet();

        virtual ~ODatabaseExport();
    public:
        ODatabaseExport(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);
        // wird f"ur auto. Typ-Erkennung gebraucht
        ODatabaseExport(sal_Int32 nRows,
                        const ::std::vector<sal_Int32> &_rColumnPositions,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        virtual void SetColumnTypes(const TColumnVector* rList,const OTypeInfoMap* _pInfoMap);
        String  ShortenFieldName( const String& rName, xub_StrLen nNewLength, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& rDestList );
    };
}

#endif // DBAUI_DATABASEEXPORT_HXX





