/*************************************************************************
 *
 *  $RCSfile: dbaexchange.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-26 15:05:04 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_DBAEXCHANGE_HXX_
#include "dbaexchange.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::datatransfer;
    using namespace ::svxform;

    //====================================================================
    //= OColumnTransferable
    //====================================================================
    //--------------------------------------------------------------------
    OColumnTransferable::OColumnTransferable(const ::rtl::OUString& _rDatasource, const sal_Int32 _nCommandType,
            const ::rtl::OUString& _rCommand, const ::rtl::OUString& _rFieldName, sal_Int32 _nFormats)
        :m_nFormatFlags(_nFormats)
    {
        implConstruct(_rDatasource, _nCommandType, _rCommand, _rFieldName);
    }

    //--------------------------------------------------------------------
    OColumnTransferable::OColumnTransferable(const Reference< XPropertySet >& _rxForm,
            const ::rtl::OUString& _rFieldName, sal_Int32 _nFormats)
        :m_nFormatFlags(_nFormats)
    {
        OSL_ENSURE(_rxForm.is(), "OColumnTransferable::OColumnTransferable: invalid form!");
        // collect the necessary information from the form
        ::rtl::OUString sCommand;
        sal_Int32       nCommandType = CommandType::TABLE;
        ::rtl::OUString sDatasource;

        sal_Bool        bTryToParse = sal_True;
        try
        {
            _rxForm->getPropertyValue(FM_PROP_COMMANDTYPE)  >>= nCommandType;
            _rxForm->getPropertyValue(FM_PROP_COMMAND)      >>= sCommand;
            _rxForm->getPropertyValue(FM_PROP_DATASOURCE)   >>= sDatasource;
            bTryToParse = ::cppu::any2bool(_rxForm->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OColumnTransferable::OColumnTransferable: could not collect essential data source attributes !");
        }

        // If the data source is an SQL-statement and simple enough (means "select <field list> from <table> where ....")
        // we are able to fake the drag information we are about to create.
        if (bTryToParse)
        {
            try
            {
                // need a query composer for this
                Reference< XSQLQueryComposerFactory > xComposerFac;
                _rxForm->getPropertyValue(FM_PROP_ACTIVE_CONNECTION) >>= xComposerFac;
                Reference< XSQLQueryComposer > xComposer;
                if (xComposerFac.is())
                    xComposer = xComposerFac->createQueryComposer();

                if (xComposer.is())
                {
                    ::rtl::OUString sActivaeCommand;
                    _rxForm->getPropertyValue(FM_PROP_ACTIVECOMMAND) >>= sActivaeCommand;
                    xComposer->setQuery(sActivaeCommand);
                    Reference< XTablesSupplier > xSupTab(xComposer, UNO_QUERY);
                    if(xSupTab.is())
                    {
                        Reference< XNameAccess > xNames = xSupTab->getTables();
                        if (xNames.is())
                        {
                            Sequence< ::rtl::OUString > aTables = xNames->getElementNames();
                            if (1 == aTables.getLength())
                            {
                                sCommand        = aTables[0];
                                nCommandType    = CommandType::TABLE;
                            }
                        }
                    }
                }
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OColumnTransferable::OColumnTransferable: could not collect essential data source attributes (part two) !");
            }
        }

        implConstruct(sDatasource, nCommandType, sCommand, _rFieldName);
    }

    //--------------------------------------------------------------------
    void OColumnTransferable::implConstruct(const ::rtl::OUString& _rDatasource, const sal_Int32 _nCommandType,
        const ::rtl::OUString& _rCommand, const ::rtl::OUString& _rFieldName)
    {
        const sal_Unicode       cSeparator = sal_Unicode(11);
        const ::rtl::OUString   sSeparator(&cSeparator, 1);

        m_sCompatibleFormat = ::rtl::OUString();
        m_sCompatibleFormat += _rDatasource;
        m_sCompatibleFormat += sSeparator;
        m_sCompatibleFormat += _rCommand;
        m_sCompatibleFormat += sSeparator;

        sal_Unicode cCommandType;
        switch (_nCommandType)
        {
            case CommandType::TABLE:
                cCommandType = '0';
                break;
            case CommandType::QUERY:
                cCommandType = '1';
                break;
            default:
                cCommandType = '2';
                break;
        }
        m_sCompatibleFormat += ::rtl::OUString(&cCommandType, 1);
        m_sCompatibleFormat += sSeparator;
        m_sCompatibleFormat += _rFieldName;
    }

    //--------------------------------------------------------------------
    void OColumnTransferable::AddSupportedFormats()
    {
        if (CTF_FIELD_DESCRIPTOR & m_nFormatFlags)
            AddFormat(SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE);

        if (CTF_CONTROL_EXCHANGE & m_nFormatFlags)
            AddFormat(SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE);
    }

    //--------------------------------------------------------------------
    sal_Bool OColumnTransferable::GetData( const DataFlavor& _rFlavor )
    {
        switch (SotExchange::GetFormat(_rFlavor))
        {
            case SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE:
            case SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE:
                return SetString(m_sCompatibleFormat, _rFlavor);
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool OColumnTransferable::canExtractColumnDescriptor(const DataFlavorExVector& _rFlavors, sal_Int32 _nFormats)
    {
        sal_Bool bFieldFormat   = 0 != (_nFormats & CTF_FIELD_DESCRIPTOR);
        sal_Bool bControlFormat = 0 != (_nFormats & CTF_CONTROL_EXCHANGE);
        for (   DataFlavorExVector::const_iterator aCheck = _rFlavors.begin();
                aCheck != _rFlavors.end();
                ++aCheck
            )
        {
            if (bFieldFormat && (SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE == aCheck->mnSotId))
                return sal_True;
            if (bControlFormat && (SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE == aCheck->mnSotId))
                return sal_True;
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool OColumnTransferable::extractColumnDescriptor(const TransferableDataHelper& _rData,
        ::rtl::OUString& _rDatasource, sal_Int32& _nCommandType, ::rtl::OUString& _rCommand, ::rtl::OUString& _rFieldName)
    {
        // check if we have a format we can use ....
        SotFormatStringId   nRecognizedFormat = 0;
        if (_rData.HasFormat(SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE))
            nRecognizedFormat = SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE;
        if (_rData.HasFormat(SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE))
            nRecognizedFormat = SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE;
        if (!nRecognizedFormat)
            return sal_False;

        String sFieldDescription;
        const_cast<TransferableDataHelper&>(_rData).GetString(nRecognizedFormat, sFieldDescription);

        const sal_Unicode cSeparator = sal_Unicode(11);
        _rDatasource    = sFieldDescription.GetToken(0, cSeparator);
        _rCommand       = sFieldDescription.GetToken(1, cSeparator);
        _nCommandType   = sFieldDescription.GetToken(2, cSeparator).ToInt32();
        _rFieldName     = sFieldDescription.GetToken(3, cSeparator);

        return sal_True;
    }

    //====================================================================
    //= ORowsetTransferable
    //====================================================================
    //--------------------------------------------------------------------
//  ORowsetTransferable::ORowsetTransferable(const String&  _rDatasource, const String& _rName,
//      ObjectType _eObject, const IntArray* _pSelectionList)
//  {
//  }

//........................................................................
}   // namespace svx
//........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 23.03.01 12:59:54  fs
 ************************************************************************/

