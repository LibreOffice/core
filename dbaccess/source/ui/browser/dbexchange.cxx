/*************************************************************************
 *
 *  $RCSfile: dbexchange.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-28 15:47:17 $
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

#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _DBACCESS_DBATOOLS_HXX_
#include "dbatools.hxx"
#endif
#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::datatransfer;

    // -----------------------------------------------------------------------------
    ODataClipboard::ODataClipboard(
                    const ::rtl::OUString&  _rDatasource,
                    const sal_Int32         _nCommandType,
                    const ::rtl::OUString&  _rCommand,
                    const Reference< XConnection >& _rxConnection,
                    const Reference< XNumberFormatter >& _rxFormatter,
                    const Reference< XMultiServiceFactory >& _rxORB,
                    const sal_Int32 _nFormats)
        :m_pHtml(NULL)
        ,m_pRtf(NULL)
        ,m_nObjectType(CommandType::TABLE)
        ,m_nFormats(_nFormats)
    {
        // build the descriptor (the property sequence)
        m_aSeq.realloc(4);
        m_aSeq[0].Name  = PROPERTY_DATASOURCENAME;
        m_aSeq[0].Value <<= _rDatasource;
        m_aSeq[1].Name  = PROPERTY_ACTIVECONNECTION;
        m_aSeq[1].Value <<= _rxConnection;
        m_aSeq[2].Name  = PROPERTY_COMMANDTYPE;
        m_aSeq[2].Value <<= _nCommandType;
        m_aSeq[3].Name  = PROPERTY_COMMAND;
        m_aSeq[3].Value <<= _rCommand;

        // calculate some stuff which helps us providing the different formats
        if (m_nFormats && DCF_OBJECT_DESCRIPTOR)
        {
            // extract the single values from the sequence
            ::rtl::OUString sDatasourceName;
            ::rtl::OUString sObjectName;
            sal_Bool bEscapeProcessing = sal_True;
            extractObjectDescription(m_aSeq, &sDatasourceName, &m_nObjectType, &sObjectName, &bEscapeProcessing);

            // for compatibility: create a string which can be used for the SOT_FORMATSTR_ID_SBA_DATAEXCHANGE format

            sal_Bool bTreatAsStatement = (CommandType::COMMAND == m_nObjectType);
                // statements are - in this old and ugly format - described as queries

            const sal_Unicode       cSeparator = sal_Unicode(11);
            const ::rtl::OUString   sSeparator(&cSeparator, 1);

            const sal_Unicode       cTableMark = '1';
            const sal_Unicode       cQueryMark = '0';

            // build the descriptor string
            m_sCompatibleObjectDescription += sDatasourceName;
            m_sCompatibleObjectDescription += sSeparator;
            m_sCompatibleObjectDescription += bTreatAsStatement ? String() : sObjectName;
            m_sCompatibleObjectDescription += sSeparator;
            switch (m_nObjectType)
            {
                case CommandType::TABLE:
                    m_sCompatibleObjectDescription += ::rtl::OUString(&cTableMark, 1);
                    break;
                case CommandType::QUERY:
                    m_sCompatibleObjectDescription += ::rtl::OUString(&cQueryMark, 1);
                    break;
                case CommandType::COMMAND:
                    m_sCompatibleObjectDescription += ::rtl::OUString(&cQueryMark, 1);
                    // think of it as a query
                    break;
            }
            m_sCompatibleObjectDescription += sSeparator;
            m_sCompatibleObjectDescription += bTreatAsStatement ? sObjectName : String();
            m_sCompatibleObjectDescription += sSeparator;
        }

        if (m_nFormats && DCF_HTML_TABLE)
        {
            m_pHtml = new OHTMLImportExport(m_aSeq, _rxORB, _rxFormatter);
            m_xHtml = m_pHtml;
            m_pHtml->initialize();
        }

        if (m_nFormats && DCF_RTF_TABLE)
        {
            m_pRtf = new ORTFImportExport(m_aSeq, _rxORB, _rxFormatter);
            m_xRtf = m_pRtf;
            m_pRtf->initialize();
        }
    }

    // -----------------------------------------------------------------------------
    ODataClipboard::ODataClipboard(const Reference< XPropertySet >& _rxLivingForm, const Reference< XSQLQueryComposer >& _rxComposer)
        :m_pHtml(NULL)
        ,m_pRtf(NULL)
        ,m_nObjectType(CommandType::TABLE)
        ,m_nFormats(DCF_OBJECT_DESCRIPTOR)
    {
        // collect some properties of the form
        ::rtl::OUString sDatasourceName;
        sal_Int32       nObjectType = CommandType::COMMAND;
        ::rtl::OUString sObjectName;
        try
        {
            _rxLivingForm->getPropertyValue(PROPERTY_COMMANDTYPE) >>= nObjectType;
            _rxLivingForm->getPropertyValue(PROPERTY_COMMAND) >>= sObjectName;
            _rxLivingForm->getPropertyValue(PROPERTY_DATASOURCENAME) >>= sDatasourceName;
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "ODataClipboard::ODataClipboard: could not collect essential form attributes !");
            m_nFormats = 0;
            return;
        }

        sal_Bool bIsStatement = CommandType::COMMAND == nObjectType;
        String sObjectKind = (CommandType::TABLE == nObjectType) ? String('1') : String('0');

        // check if the SQL-statement is modified
        sal_Bool bHasFilterOrSort(sal_False);
        ::rtl::OUString sCompleteStatement;
        try
        {
            ::rtl::OUString sFilter;
            if (::cppu::any2bool(_rxLivingForm->getPropertyValue(PROPERTY_APPLYFILTER)))
                _rxLivingForm->getPropertyValue(PROPERTY_FILTER) >>= sFilter;
            ::rtl::OUString sSort;
            _rxLivingForm->getPropertyValue(PROPERTY_ORDER) >>= sSort;
            bHasFilterOrSort = (sFilter.len()>0) || (sSort.len()>0);

            _rxLivingForm->getPropertyValue(PROPERTY_ACTIVECOMMAND) >>= sCompleteStatement;
            if (_rxComposer.is())
            {
                _rxComposer->setQuery(sCompleteStatement);
                _rxComposer->setFilter(sFilter);
                _rxComposer->setOrder(sSort);
                sCompleteStatement = _rxComposer->getComposedQuery();
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "ODataClipboard::ODataClipboard: could not collect essential form attributes (part two) !");
            m_nFormats = 0;
            return;
        }

        // build the object description (as string)
        const sal_Unicode       cSeparator(11);
        const ::rtl::OUString   sSeparator(&cSeparator, 1);

        m_sCompatibleObjectDescription = sDatasourceName;
        m_sCompatibleObjectDescription  += sSeparator;
        m_sCompatibleObjectDescription  += bIsStatement ? String() : sDatasourceName;
        m_sCompatibleObjectDescription  += sSeparator;
        m_sCompatibleObjectDescription  += sObjectKind;
        m_sCompatibleObjectDescription  += sSeparator;
        m_sCompatibleObjectDescription  +=
                (CommandType::QUERY == nObjectType) && !bHasFilterOrSort
                ? ::rtl::OUString()
            : sCompleteStatement;
            // compatibility says : always add the statement, but don't if it is a "pure" query
        m_sCompatibleObjectDescription  += sSeparator;
    }

    // -----------------------------------------------------------------------------
    void ODataClipboard::addRow(sal_Int32 _nRow)
    {
        OSL_ENSURE(m_nFormats && DCF_OBJECT_DESCRIPTOR, "ODataClipboard::addRow: don't have this (object descriptor) format!");

        const sal_Unicode       cSeparator(11);
        const ::rtl::OUString   sSeparator(&cSeparator, 1);

        m_sCompatibleObjectDescription += ::rtl::OUString::valueOf((sal_Int32)_nRow);
        m_sCompatibleObjectDescription += sSeparator;
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODataClipboard::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        if (nUserObjectId == SOT_FORMAT_RTF || nUserObjectId == SOT_FORMATSTR_ID_HTML)
        {
            ODatabaseImportExport* pExport = reinterpret_cast<ODatabaseImportExport*>(pUserObject);
            if(pExport)
            {
                pExport->setStream(&rxOStm);
                return pExport->Write();
            }
        }
        return sal_False;
    }

    // -----------------------------------------------------------------------------
    void ODataClipboard::AddSupportedFormats()
    {
        // RTF?
        if (m_nFormats && DCF_RTF_TABLE)
            AddFormat(SOT_FORMAT_RTF);

        // HTML?
        if (m_nFormats && DCF_HTML_TABLE)
            AddFormat(SOT_FORMATSTR_ID_HTML);

        // object descriptor?
        if (m_nFormats && DCF_OBJECT_DESCRIPTOR)
        {
            switch (m_nObjectType)
            {
                case CommandType::TABLE:
                    AddFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE);
                    break;
                case CommandType::QUERY:
                    AddFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY);
                    break;
                case CommandType::COMMAND:
                    AddFormat(SOT_FORMATSTR_ID_DBACCESS_COMMAND);
                    break;
            }

            sal_Int32 nDescriptorLen = m_sCompatibleObjectDescription.getLength();
            if (nDescriptorLen)
            {
                if (m_sCompatibleObjectDescription.getStr()[nDescriptorLen] == 11)
                    m_sCompatibleObjectDescription = m_sCompatibleObjectDescription.copy(0, nDescriptorLen - 1);

                if (nDescriptorLen)
                    AddFormat(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE);
            }
        }
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODataClipboard::GetData( const DataFlavor& rFlavor )
    {
        ULONG nFormat = SotExchange::GetFormat(rFlavor);
        switch (nFormat)
        {
            case SOT_FORMAT_RTF:
                return SetObject(m_pRtf,SOT_FORMAT_RTF,rFlavor);

            case SOT_FORMATSTR_ID_HTML:
                return SetObject(m_pHtml,SOT_FORMATSTR_ID_HTML,rFlavor);

            case SOT_FORMATSTR_ID_DBACCESS_TABLE:
            case SOT_FORMATSTR_ID_DBACCESS_QUERY:
            case SOT_FORMATSTR_ID_DBACCESS_COMMAND:
                return SetAny(makeAny(m_aSeq), rFlavor);

            case SOT_FORMATSTR_ID_SBA_DATAEXCHANGE:
                return SetString(m_sCompatibleObjectDescription, rFlavor);
        }
        return sal_False;
    }

    // -----------------------------------------------------------------------------
    void ODataClipboard::ObjectReleased()
    {
        m_xHtml = m_xRtf = NULL;
        m_pHtml = NULL;
        m_pRtf = NULL;
        m_aSeq.realloc(0);
    }

    // -----------------------------------------------------------------------------
}








