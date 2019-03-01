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

#include <UITools.hxx>
#include <sfx2/docfilt.hxx>
#include <callbacks.hxx>
#include <core_resource.hxx>
#include <stringconstants.hxx>
#include <dlgsave.hxx>
#include <dbtreelistbox.hxx>
#include <defaultobjectnamecheck.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/settings.hxx>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <TypeInfo.hxx>
#include <FieldDescriptions.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/types.hxx>
#include <comphelper/propertysequence.hxx>

#include <svx/svxids.hrc>

#include <sal/log.hxx>
#include <svl/itempool.hxx>
#include <helpids.h>
#include <svl/itemset.hxx>
#include <sbagrid.hrc>
#include <svl/rngitem.hxx>
#include <svl/intitem.hxx>
#include <svx/algitem.hxx>
#include <svx/numinf.hxx>
#include <svl/zforlist.hxx>
#include <dlgattr.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <strings.hrc>
#include <sqlmessage.hxx>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/URL.hpp>
#include <vcl/toolbox.hxx>
#include <dlgsize.hxx>
#include <svtools/editbrowsebox.hxx>
#include <unotools/configmgr.hxx>
#include <svtools/helpopt.hxx>
#include <ucbhelper/content.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <svl/numuno.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/filenotation.hxx>
#include <svtools/fileview.hxx>
#include <connectivity/FValue.hxx>

#include <editeng/justifyitem.hxx>
#include <memory>

namespace dbaui
{
using namespace ::dbtools;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::svt;
using ::com::sun::star::ucb::InteractiveIOException;
using ::com::sun::star::ucb::IOErrorCode_NO_FILE;
using ::com::sun::star::ucb::IOErrorCode_NOT_EXISTING;

SQLExceptionInfo createConnection(  const OUString& _rsDataSourceName,
                                     const Reference< css::container::XNameAccess >& _xDatabaseContext,
                                    const Reference< css::uno::XComponentContext >& _rxContext,
                                    Reference< css::lang::XEventListener> const & _rEvtLst,
                                    Reference< css::sdbc::XConnection>& _rOUTConnection )
{
    Reference<XPropertySet> xProp;
    try
    {
        xProp.set(_xDatabaseContext->getByName(_rsDataSourceName),UNO_QUERY);
    }
    catch(const Exception&)
    {
    }
    SQLExceptionInfo aInfo;

    return createConnection(xProp,_rxContext,_rEvtLst,_rOUTConnection);
}

SQLExceptionInfo createConnection(  const Reference< css::beans::XPropertySet>& _xDataSource,
                                    const Reference< css::uno::XComponentContext >& _rxContext,
                                    Reference< css::lang::XEventListener> const & _rEvtLst,
                                    Reference< css::sdbc::XConnection>& _rOUTConnection )
{
    SQLExceptionInfo aInfo;
    if ( !_xDataSource.is() )
    {
        SAL_WARN("dbaccess.ui", "createConnection: could not retrieve the data source!");
        return aInfo;
    }

    OUString sPwd, sUser;
    bool bPwdReq = false;
    try
    {
        _xDataSource->getPropertyValue(PROPERTY_PASSWORD) >>= sPwd;
        bPwdReq = ::cppu::any2bool(_xDataSource->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED));
        _xDataSource->getPropertyValue(PROPERTY_USER) >>= sUser;
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui", "createConnection: error while retrieving data source properties!");
    }

    try
    {
        if(bPwdReq && sPwd.isEmpty())
        {   // password required, but empty -> connect using an interaction handler
            Reference<XCompletedConnection> xConnectionCompletion(_xDataSource, UNO_QUERY);
            if (!xConnectionCompletion.is())
            {
                SAL_WARN("dbaccess.ui", "createConnection: missing an interface ... need an error message here!");
            }
            else
            {   // instantiate the default SDB interaction handler
                Reference< XInteractionHandler > xHandler( InteractionHandler::createWithParent(_rxContext, nullptr), UNO_QUERY);
                _rOUTConnection = xConnectionCompletion->connectWithCompletion(xHandler);
            }
        }
        else
        {
            Reference<XDataSource> xDataSource(_xDataSource,UNO_QUERY);
            _rOUTConnection = xDataSource->getConnection(sUser, sPwd);
        }
        // be notified when connection is in disposing
        Reference< XComponent >  xComponent(_rOUTConnection, UNO_QUERY);
        if (xComponent.is() && _rEvtLst.is())
            xComponent->addEventListener(_rEvtLst);
    }
    catch(const SQLContext& e) { aInfo = SQLExceptionInfo(e); }
    catch(const SQLWarning& e) { aInfo = SQLExceptionInfo(e); }
    catch(const SQLException& e) { aInfo = SQLExceptionInfo(e); }
    catch(const Exception&) {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::OnExpandEntry: could not connect - unknown exception! " << exceptionToString(ex));
    }

    return aInfo;
}

Reference< XDataSource > getDataSourceByName( const OUString& _rDataSourceName,
    vcl::Window* _pErrorMessageParent, const Reference< XComponentContext >& _rxContext, ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    Reference< XDatabaseContext > xDatabaseContext = DatabaseContext::create(_rxContext);

    Reference< XDataSource > xDatasource;
    Any aError;
    SQLExceptionInfo aSQLError;
    try
    {
        xDatabaseContext->getByName( _rDataSourceName ) >>= xDatasource;
    }
    catch(const WrappedTargetException& e)
    {
        InteractiveIOException aIOException;
        if  (   ( e.TargetException >>= aIOException )
            &&  (   ( aIOException.Code == IOErrorCode_NO_FILE )
                ||  ( aIOException.Code == IOErrorCode_NOT_EXISTING )
                )
            )
        {
            OUString sErrorMessage( DBA_RES( STR_FILE_DOES_NOT_EXIST ) );
            OFileNotation aTransformer( e.Message );
            sErrorMessage = sErrorMessage.replaceFirst( "$file$", aTransformer.get( OFileNotation::N_SYSTEM ) );
            aSQLError = SQLExceptionInfo( sErrorMessage ).get();
        }
        else
        {
            aSQLError = SQLExceptionInfo( e.TargetException );
            if ( !aSQLError.isValid() )
                aError = e.TargetException;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    if ( xDatasource.is() )
        return xDatasource;

    if ( aSQLError.isValid() )
    {
        if ( _pErrorInfo )
        {
            *_pErrorInfo = aSQLError;
        }
        else
        {
            showError( aSQLError, VCLUnoHelper::GetInterface(_pErrorMessageParent), _rxContext );
        }
    }

    return Reference<XDataSource>();
}

Reference< XInterface > getDataSourceOrModel(const Reference< XInterface >& _xObject)
{
    Reference< XInterface > xRet;
    Reference<XDocumentDataSource> xDocumentDataSource(_xObject,UNO_QUERY);
    if ( xDocumentDataSource.is() )
        xRet = xDocumentDataSource->getDatabaseDocument();

    if ( !xRet.is() )
    {
        Reference<XOfficeDatabaseDocument> xOfficeDoc(_xObject,UNO_QUERY);
        if ( xOfficeDoc.is() )
            xRet = xOfficeDoc->getDataSource();
    }

    return xRet;
}

TOTypeInfoSP getTypeInfoFromType(const OTypeInfoMap& _rTypeInfo,
                               sal_Int32 _nType,
                               const OUString& _sTypeName,
                               const OUString& _sCreateParams,
                               sal_Int32 _nPrecision,
                               sal_Int32 _nScale,
                               bool _bAutoIncrement,
                               bool& _brForceToType)
{
    TOTypeInfoSP pTypeInfo;
    _brForceToType = false;
    // search for type
    std::pair<OTypeInfoMap::const_iterator, OTypeInfoMap::const_iterator> aPair = _rTypeInfo.equal_range(_nType);
    OTypeInfoMap::const_iterator aIter = aPair.first;
    if(aIter != _rTypeInfo.end()) // compare with end is correct here
    {
        for(;aIter != aPair.second;++aIter)
        {
            // search the best matching type
    #ifdef DBG_UTIL
            OUString sDBTypeName         = aIter->second->aTypeName;         (void)sDBTypeName;
    #endif
            if  (   (
                        _sTypeName.isEmpty()
                    ||  (aIter->second->aTypeName.equalsIgnoreAsciiCase(_sTypeName))
                    )
                &&  (
                        (
                                !aIter->second->aCreateParams.getLength()
                            &&  _sCreateParams.isEmpty()
                        )
                    ||  (
                                (aIter->second->nPrecision      >= _nPrecision)
                            &&  (aIter->second->nMaximumScale   >= _nScale)
                            &&  ( (_bAutoIncrement && aIter->second->bAutoIncrement) || !_bAutoIncrement )
                        )
                    )
                )
                break;
        }

        if (aIter == aPair.second)
        {
            for(aIter = aPair.first; aIter != aPair.second; ++aIter)
            {
                sal_Int32 nPrec = aIter->second->nPrecision;
                sal_Int32 nScale = aIter->second->nMaximumScale;
                // search the best matching type (now comparing the local names)
                if  (   (aIter->second->aLocalTypeName.equalsIgnoreAsciiCase(_sTypeName))
                    &&  (nPrec  >= _nPrecision)
                    &&  (nScale >= _nScale)
                    &&  ( (_bAutoIncrement && aIter->second->bAutoIncrement) || !_bAutoIncrement )
                    )
                {
                    SAL_WARN("dbaccess.ui", "getTypeInfoFromType: assuming column type " <<
                             aIter->second->aTypeName <<  "\" (expected type name " <<
                             _sTypeName << " matches the type's local name).");
                    break;
                }
            }
        }

        if (aIter == aPair.second)
        {   // no match for the names, no match for the local names
            // -> drop the precision and the scale restriction, accept any type with the property
            // type id (nType)

            for(aIter = aPair.first; aIter != aPair.second; ++aIter)
            {
                // search the best matching type (now comparing the local names)
                sal_Int32 nPrec = aIter->second->nPrecision;
                sal_Int32 nScale = aIter->second->nMaximumScale;
                if  (   (nPrec  >= _nPrecision)
                    &&  (nScale >= _nScale)
                    &&  ( (_bAutoIncrement && aIter->second->bAutoIncrement) || !_bAutoIncrement )
                    )
                    break;
            }
        }
        if (aIter == aPair.second)
        {
            if ( _bAutoIncrement )
            {
                for(aIter = aPair.first; aIter != aPair.second; ++aIter)
                {
                    // search the best matching type (now comparing the local names)
                    sal_Int32 nScale = aIter->second->nMaximumScale;
                    if  (   (nScale >= _nScale)
                        &&  (aIter->second->bAutoIncrement  == _bAutoIncrement)
                        )
                        break;
                }
                if ( aIter == aPair.second )
                {
                    // try it without the auto increment flag
                    pTypeInfo = getTypeInfoFromType(_rTypeInfo,
                                   _nType,
                                   _sTypeName,
                                   _sCreateParams,
                                   _nPrecision,
                                   _nScale,
                                   false,
                                   _brForceToType);
                }
                else
                    pTypeInfo = aIter->second;
            }
            else
            {
                pTypeInfo = aPair.first->second;
                _brForceToType = true;
            }
        }
        else
            pTypeInfo = aIter->second;
    }
    else
    {
        ::comphelper::UStringMixEqual aCase(false);
        // search for typeinfo where the typename is equal _sTypeName
        for (auto const& elem : _rTypeInfo)
        {
            if ( aCase( elem.second->getDBName() , _sTypeName ) )
            {
                pTypeInfo = elem.second;
                break;
            }
        }
    }

    OSL_ENSURE(pTypeInfo, "getTypeInfoFromType: no type info found for this type!");
    return pTypeInfo;
}

void fillTypeInfo(  const Reference< css::sdbc::XConnection>& _rxConnection,
                    const OUString& _rsTypeNames,
                    OTypeInfoMap& _rTypeInfoMap,
                    std::vector<OTypeInfoMap::iterator>& _rTypeInfoIters)
{
    if(!_rxConnection.is())
        return;
    Reference< XResultSet> xRs = _rxConnection->getMetaData ()->getTypeInfo ();
    Reference< XRow> xRow(xRs,UNO_QUERY);
    // Information for a single SQL type
    if(xRs.is())
    {
        Reference<XResultSetMetaData> xResultSetMetaData = Reference<XResultSetMetaDataSupplier>(xRs,UNO_QUERY)->getMetaData();
        ::connectivity::ORowSetValue aValue;
        std::vector<sal_Int32> aTypes;
        std::vector<bool> aNullable;
        // Loop on the result set until we reach end of file
        while (xRs->next())
        {
            TOTypeInfoSP pInfo(new OTypeInfo());
            sal_Int32 nPos = 1;
            if ( aTypes.empty() )
            {
                sal_Int32 nCount = xResultSetMetaData->getColumnCount();
                if ( nCount < 1 )
                    nCount = 18;
                aTypes.reserve(nCount+1);
                aTypes.push_back(-1);
                aNullable.push_back(false);
                for (sal_Int32 j = 1; j <= nCount ; ++j)
                {
                    aTypes.push_back(xResultSetMetaData->getColumnType(j));
                    aNullable.push_back(xResultSetMetaData->isNullable(j) != ColumnValue::NO_NULLS);
                }
            }

            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->aTypeName        = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->nType            = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->nPrecision       = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow); // LiteralPrefix
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow); //LiteralSuffix
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->aCreateParams    = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->bNullable        = static_cast<sal_Int32>(aValue) == ColumnValue::NULLABLE;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            // bCaseSensitive
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->nSearchType      = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            // bUnsigned
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->bCurrency        = static_cast<bool>(aValue);
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->bAutoIncrement   = static_cast<bool>(aValue);
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->aLocalTypeName   = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->nMinimumScale    = aValue;
            ++nPos;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->nMaximumScale    = aValue;
            assert(nPos == 15);
            // 16 and 17 are unused
            nPos = 18;
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            pInfo->nNumPrecRadix    = aValue;

            // check if values are less than zero like it happens in a oracle jdbc driver
            if( pInfo->nPrecision < 0)
                pInfo->nPrecision = 0;
            if( pInfo->nMinimumScale < 0)
                pInfo->nMinimumScale = 0;
            if( pInfo->nMaximumScale < 0)
                pInfo->nMaximumScale = 0;
            if( pInfo->nNumPrecRadix <= 1)
                pInfo->nNumPrecRadix = 10;

            OUString aName;
            switch(pInfo->nType)
            {
                case DataType::CHAR:
                    aName = _rsTypeNames.getToken(TYPE_CHAR, ';');
                    break;
                case DataType::VARCHAR:
                    aName = _rsTypeNames.getToken(TYPE_TEXT, ';');
                    break;
                case DataType::DECIMAL:
                    aName = _rsTypeNames.getToken(TYPE_DECIMAL, ';');
                    break;
                case DataType::NUMERIC:
                    aName = _rsTypeNames.getToken(TYPE_NUMERIC, ';');
                    break;
                case DataType::BIGINT:
                    aName = _rsTypeNames.getToken(TYPE_BIGINT, ';');
                    break;
                case DataType::FLOAT:
                    aName = _rsTypeNames.getToken(TYPE_FLOAT, ';');
                    break;
                case DataType::DOUBLE:
                    aName = _rsTypeNames.getToken(TYPE_DOUBLE, ';');
                    break;
                case DataType::LONGVARCHAR:
                    aName = _rsTypeNames.getToken(TYPE_MEMO, ';');
                    break;
                case DataType::LONGVARBINARY:
                    aName = _rsTypeNames.getToken(TYPE_IMAGE, ';');
                    break;
                case DataType::DATE:
                    aName = _rsTypeNames.getToken(TYPE_DATE, ';');
                    break;
                case DataType::TIME:
                    aName = _rsTypeNames.getToken(TYPE_TIME, ';');
                    break;
                case DataType::TIMESTAMP:
                    aName = _rsTypeNames.getToken(TYPE_DATETIME, ';');
                    break;
                case DataType::BIT:
                    if ( !pInfo->aCreateParams.isEmpty() )
                    {
                        aName = _rsTypeNames.getToken(TYPE_BIT, ';');
                        break;
                    }
                    [[fallthrough]];
                case DataType::BOOLEAN:
                    aName = _rsTypeNames.getToken(TYPE_BOOL, ';');
                    break;
                case DataType::TINYINT:
                    aName = _rsTypeNames.getToken(TYPE_TINYINT, ';');
                    break;
                case DataType::SMALLINT:
                    aName = _rsTypeNames.getToken(TYPE_SMALLINT, ';');
                    break;
                case DataType::INTEGER:
                    aName = _rsTypeNames.getToken(TYPE_INTEGER, ';');
                    break;
                case DataType::REAL:
                    aName = _rsTypeNames.getToken(TYPE_REAL, ';');
                    break;
                case DataType::BINARY:
                    aName = _rsTypeNames.getToken(TYPE_BINARY, ';');
                    break;
                case DataType::VARBINARY:
                    aName = _rsTypeNames.getToken(TYPE_VARBINARY, ';');
                    break;
                case DataType::SQLNULL:
                    aName = _rsTypeNames.getToken(TYPE_SQLNULL, ';');
                    break;
                case DataType::OBJECT:
                    aName = _rsTypeNames.getToken(TYPE_OBJECT, ';');
                    break;
                case DataType::DISTINCT:
                    aName = _rsTypeNames.getToken(TYPE_DISTINCT, ';');
                    break;
                case DataType::STRUCT:
                    aName = _rsTypeNames.getToken(TYPE_STRUCT, ';');
                    break;
                case DataType::ARRAY:
                    aName = _rsTypeNames.getToken(TYPE_ARRAY, ';');
                    break;
                case DataType::BLOB:
                    aName = _rsTypeNames.getToken(TYPE_BLOB, ';');
                    break;
                case DataType::CLOB:
                    aName = _rsTypeNames.getToken(TYPE_CLOB, ';');
                    break;
                case DataType::REF:
                    aName = _rsTypeNames.getToken(TYPE_REF, ';');
                    break;
                case DataType::OTHER:
                    aName = _rsTypeNames.getToken(TYPE_OTHER, ';');
                    break;
            }
            if ( !aName.isEmpty() )
            {
                pInfo->aUIName = aName;
                pInfo->aUIName += " [ ";
            }
            pInfo->aUIName += pInfo->aTypeName;
            if ( !aName.isEmpty() )
                pInfo->aUIName += " ]";
            // Now that we have the type info, save it in the multimap
            _rTypeInfoMap.emplace(pInfo->nType,pInfo);
        }
        // for a faster index access
        _rTypeInfoIters.reserve(_rTypeInfoMap.size());

        OTypeInfoMap::iterator aIter = _rTypeInfoMap.begin();
        OTypeInfoMap::const_iterator aEnd = _rTypeInfoMap.end();
        for(;aIter != aEnd;++aIter)
            _rTypeInfoIters.push_back(aIter);

        // Close the result set/statement.

        ::comphelper::disposeComponent(xRs);
    }
}

void setColumnProperties(const Reference<XPropertySet>& _rxColumn,const OFieldDescription* _pFieldDesc)
{
    _rxColumn->setPropertyValue(PROPERTY_NAME,makeAny(_pFieldDesc->GetName()));
    _rxColumn->setPropertyValue(PROPERTY_TYPENAME,makeAny(_pFieldDesc->getTypeInfo()->aTypeName));
    _rxColumn->setPropertyValue(PROPERTY_TYPE,makeAny(_pFieldDesc->GetType()));
    _rxColumn->setPropertyValue(PROPERTY_PRECISION,makeAny(_pFieldDesc->GetPrecision()));
    _rxColumn->setPropertyValue(PROPERTY_SCALE,makeAny(_pFieldDesc->GetScale()));
    _rxColumn->setPropertyValue(PROPERTY_ISNULLABLE, makeAny(_pFieldDesc->GetIsNullable()));
    _rxColumn->setPropertyValue(PROPERTY_ISAUTOINCREMENT, css::uno::makeAny(_pFieldDesc->IsAutoIncrement()));
    _rxColumn->setPropertyValue(PROPERTY_DESCRIPTION,makeAny(_pFieldDesc->GetDescription()));
    if ( _rxColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ISCURRENCY) && _pFieldDesc->IsCurrency() )
        _rxColumn->setPropertyValue(PROPERTY_ISCURRENCY, css::uno::makeAny(_pFieldDesc->IsCurrency()));
    // set autoincrement value when available
    // and only set when the entry is not empty, that lets the value in the column untouched
    if ( _pFieldDesc->IsAutoIncrement() && !_pFieldDesc->GetAutoIncrementValue().isEmpty() && _rxColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_AUTOINCREMENTCREATION) )
        _rxColumn->setPropertyValue(PROPERTY_AUTOINCREMENTCREATION,makeAny(_pFieldDesc->GetAutoIncrementValue()));
}

OUString createDefaultName(const Reference< XDatabaseMetaData>& _xMetaData,const Reference<XNameAccess>& _xTables,const OUString& _sName)
{
    OSL_ENSURE(_xMetaData.is(),"No MetaData!");
    OUString sDefaultName = _sName;
    try
    {
        OUString sCatalog,sSchema,sCompsedName;
        if(_xMetaData->supportsCatalogsInTableDefinitions())
        {
            try
            {
                Reference< XConnection> xCon = _xMetaData->getConnection();
                if ( xCon.is() )
                    sCatalog = xCon->getCatalog();
                if ( sCatalog.isEmpty() )
                {
                    Reference<XResultSet> xRes = _xMetaData->getCatalogs();
                    Reference<XRow> xRow(xRes,UNO_QUERY);
                    while(xRes.is() && xRes->next())
                    {
                        sCatalog = xRow->getString(1);
                        if(!xRow->wasNull())
                            break;
                    }
                }
            }
            catch(const SQLException&)
            {
            }
        }
        if(_xMetaData->supportsSchemasInTableDefinitions())
        {
            sSchema = _xMetaData->getUserName();
        }
        sCompsedName = ::dbtools::composeTableName( _xMetaData, sCatalog, sSchema, _sName, false, ::dbtools::EComposeRule::InDataManipulation );
        sDefaultName = ::dbtools::createUniqueName(_xTables,sCompsedName);
    }
    catch(const SQLException&)
    {
    }
    return sDefaultName;
}

bool checkDataSourceAvailable(const OUString& _sDataSourceName,const Reference< css::uno::XComponentContext >& _xContext)
{
    Reference< XDatabaseContext > xDataBaseContext = DatabaseContext::create(_xContext);
    bool bRet = xDataBaseContext->hasByName(_sDataSourceName);
    if ( !bRet )
    { // try if this one is a URL
        try
        {
            bRet = xDataBaseContext->getByName(_sDataSourceName).hasValue();
        }
        catch(const Exception&)
        {
        }
    }
    return bRet;
}

sal_Int32 mapTextAllign(const SvxCellHorJustify& _eAlignment)
{
    sal_Int32 nAlignment = css::awt::TextAlign::LEFT;
    switch (_eAlignment)
    {
        case SvxCellHorJustify::Standard:
        case SvxCellHorJustify::Left:      nAlignment = css::awt::TextAlign::LEFT;    break;
        case SvxCellHorJustify::Center:    nAlignment = css::awt::TextAlign::CENTER;  break;
        case SvxCellHorJustify::Right:     nAlignment = css::awt::TextAlign::RIGHT;   break;
        default:
            SAL_WARN("dbaccess.ui", "Invalid TextAlign!");
    }
    return nAlignment;
}

SvxCellHorJustify mapTextJustify(sal_Int32 _nAlignment)
{
    SvxCellHorJustify eJustify = SvxCellHorJustify::Left;
    switch (_nAlignment)
    {
        case css::awt::TextAlign::LEFT     : eJustify = SvxCellHorJustify::Left; break;
        case css::awt::TextAlign::CENTER   : eJustify = SvxCellHorJustify::Center; break;
        case css::awt::TextAlign::RIGHT    : eJustify = SvxCellHorJustify::Right; break;
        default:
            SAL_WARN("dbaccess.ui", "Invalid TextAlign!");
    }
    return eJustify;
}

void callColumnFormatDialog(const Reference<XPropertySet>& xAffectedCol,
                            const Reference<XPropertySet>& xField,
                            SvNumberFormatter* _pFormatter,
                            const vcl::Window* _pParent)
{
    if (xAffectedCol.is() && xField.is())
    {
        try
        {
            Reference< XPropertySetInfo >  xInfo = xAffectedCol->getPropertySetInfo();
            bool bHasFormat = xInfo->hasPropertyByName(PROPERTY_FORMATKEY);
            sal_Int32 nDataType = ::comphelper::getINT32(xField->getPropertyValue(PROPERTY_TYPE));

            SvxCellHorJustify eJustify(SvxCellHorJustify::Standard);
            Any aAlignment = xAffectedCol->getPropertyValue(PROPERTY_ALIGN);
            if (aAlignment.hasValue())
                eJustify = dbaui::mapTextJustify(::comphelper::getINT16(aAlignment));
            sal_Int32  nFormatKey = 0;
            if ( bHasFormat )
                nFormatKey = ::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_FORMATKEY));

            if(callColumnFormatDialog(_pParent,_pFormatter,nDataType,nFormatKey,eJustify,bHasFormat))
            {
                xAffectedCol->setPropertyValue(PROPERTY_ALIGN, makeAny(static_cast<sal_Int16>(dbaui::mapTextAllign(eJustify))));
                if (bHasFormat)
                    xAffectedCol->setPropertyValue(PROPERTY_FORMATKEY, makeAny(nFormatKey));

            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

bool callColumnFormatDialog(const vcl::Window* _pParent,
                                SvNumberFormatter* _pFormatter,
                                sal_Int32 _nDataType,
                                sal_Int32& _nFormatKey,
                                SvxCellHorJustify& _eJustify,
                                bool  _bHasFormat)
{
    bool bRet = false;

    // UNO->ItemSet
    static SfxItemInfo aItemInfos[] =
    {
        { 0, false },
        { SID_ATTR_NUMBERFORMAT_VALUE,      true },
        { SID_ATTR_ALIGN_HOR_JUSTIFY,       true },
        { SID_ATTR_NUMBERFORMAT_ONE_AREA,   true },
        { SID_ATTR_NUMBERFORMAT_INFO,       true }
    };
    static const sal_uInt16 aAttrMap[] =
    {
        SBA_DEF_RANGEFORMAT, SBA_ATTR_ALIGN_HOR_JUSTIFY,
        SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
        SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO,
        0
    };

    std::vector<SfxPoolItem*> pDefaults
    {
        new SfxRangeItem(SBA_DEF_RANGEFORMAT, SBA_DEF_FMTVALUE, SBA_ATTR_ALIGN_HOR_JUSTIFY),
        new SfxUInt32Item(SBA_DEF_FMTVALUE),
        new SvxHorJustifyItem(SvxCellHorJustify::Standard, SBA_ATTR_ALIGN_HOR_JUSTIFY),
        new SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, false),
        new SvxNumberInfoItem(SID_ATTR_NUMBERFORMAT_INFO)
    };

    SfxItemPool* pPool = new SfxItemPool("GridBrowserProperties", SBA_DEF_RANGEFORMAT, SBA_ATTR_ALIGN_HOR_JUSTIFY, aItemInfos, &pDefaults);
    pPool->SetDefaultMetric( MapUnit::MapTwip );    // ripped, don't understand why
    pPool->FreezeIdRanges();                        // the same

    std::unique_ptr<SfxItemSet> pFormatDescriptor(new SfxItemSet(*pPool, aAttrMap));
    // fill it
    pFormatDescriptor->Put(SvxHorJustifyItem(_eJustify, SBA_ATTR_ALIGN_HOR_JUSTIFY));
    bool bText = false;
    if (_bHasFormat)
    {
        // if the col is bound to a text field we have to disallow all non-text formats
        if ((DataType::CHAR == _nDataType) || (DataType::VARCHAR == _nDataType) || (DataType::LONGVARCHAR == _nDataType) || (DataType::CLOB == _nDataType))
        {
            bText = true;
            pFormatDescriptor->Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, true));
            if (!_pFormatter->IsTextFormat(_nFormatKey))
                // text fields can only have text formats
                _nFormatKey = _pFormatter->GetStandardFormat(SvNumFormatType::TEXT,_pParent->GetSettings().GetLanguageTag().getLanguageType());
        }

        pFormatDescriptor->Put(SfxUInt32Item(SBA_DEF_FMTVALUE, _nFormatKey));
    }

    if (!bText)
    {
        SvxNumberInfoItem aFormatter(_pFormatter, 1234.56789, SID_ATTR_NUMBERFORMAT_INFO);
        pFormatDescriptor->Put(aFormatter);
    }

    {   // want the dialog to be destroyed before our set
        SbaSbAttrDlg aDlg(_pParent->GetFrameWeld(), pFormatDescriptor.get(), _pFormatter, _bHasFormat);
        if (RET_OK == aDlg.run())
        {
            // ItemSet->UNO
            // UNO-properties
            const SfxItemSet* pSet = aDlg.GetExampleSet();
            // (of course we could put the modified items directly into the column, but then the UNO-model
            // won't reflect these changes, and why do we have a model, then ?)

            // horizontal justify
            const SvxHorJustifyItem* pHorJustify = pSet->GetItem<SvxHorJustifyItem>(SBA_ATTR_ALIGN_HOR_JUSTIFY);

            _eJustify = pHorJustify->GetValue();

            // format key
            if (_bHasFormat)
            {
                const SfxUInt32Item* pFormat = pSet->GetItem<SfxUInt32Item>(SBA_DEF_FMTVALUE);
                _nFormatKey = static_cast<sal_Int32>(pFormat->GetValue());
            }
            bRet = true;
        }
            // deleted formats
        const SfxItemSet* pResult = aDlg.GetOutputItemSet();
        if (pResult)
        {
            const SfxPoolItem* pItem = pResult->GetItem( SID_ATTR_NUMBERFORMAT_INFO );
            const SvxNumberInfoItem* pInfoItem = static_cast<const SvxNumberInfoItem*>(pItem);
            if (pInfoItem)
            {
                for (sal_uInt32 key : pInfoItem->GetDelFormats())
                    _pFormatter->DeleteEntry(key);
            }
        }
    }

    pFormatDescriptor.reset();
    SfxItemPool::Free(pPool);
    for (SfxPoolItem* pDefault : pDefaults)
        delete pDefault;

    return bRet;
}

std::shared_ptr<const SfxFilter> getStandardDatabaseFilter()
{
    std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetFilterByName("StarOffice XML (Base)");
    OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
    return pFilter;
}

bool appendToFilter(const Reference<XConnection>& _xConnection,
                    const OUString& _sName,
                    const Reference< XComponentContext >& _rxContext,
                    weld::Window* pParent)
{
    bool bRet = false;
    Reference< XChild> xChild(_xConnection,UNO_QUERY);
    if(xChild.is())
    {
        Reference< XPropertySet> xProp(xChild->getParent(),UNO_QUERY);
        if(xProp.is())
        {
            Sequence< OUString > aFilter;
            xProp->getPropertyValue(PROPERTY_TABLEFILTER) >>= aFilter;
            // first check if we have something like SCHEMA.%
            bool bHasToInsert = true;
            for (const OUString& rItem : aFilter)
            {
                if(rItem.indexOf('%') != -1)
                {
                    sal_Int32 nLen;
                    if((nLen = rItem.lastIndexOf('.')) != -1 && !rItem.compareTo(_sName,nLen))
                        bHasToInsert = false;
                    else if(rItem.getLength() == 1)
                        bHasToInsert = false;
                }
            }

            bRet = true;
            if(bHasToInsert)
            {
                if(! ::dbaui::checkDataSourceAvailable(::comphelper::getString(xProp->getPropertyValue(PROPERTY_NAME)),_rxContext))
                {
                    OUString aMessage(DBA_RES(STR_TABLEDESIGN_DATASOURCE_DELETED));
                    OSQLWarningBox aWarning(pParent, aMessage);
                    aWarning.run();
                    bRet = false;
                }
                else
                {
                    aFilter.realloc(aFilter.getLength()+1);
                    aFilter.getArray()[aFilter.getLength()-1] = _sName;
                    xProp->setPropertyValue(PROPERTY_TABLEFILTER,makeAny(aFilter));
                }
            }
        }
    }
    return bRet;
}

void notifySystemWindow(vcl::Window const * _pWindow, vcl::Window* _pToRegister, const ::comphelper::mem_fun1_t<TaskPaneList,vcl::Window*>& _rMemFunc)
{
    OSL_ENSURE(_pWindow,"Window can not be null!");
    SystemWindow* pSystemWindow = _pWindow ? _pWindow->GetSystemWindow() : nullptr;
    if ( pSystemWindow )
    {
        _rMemFunc( pSystemWindow->GetTaskPaneList(), _pToRegister );
    }
}

void adjustToolBoxSize(ToolBox* _pToolBox)
{
    // adjust the toolbox size, otherwise large bitmaps don't fit into
    Size aOldSize = _pToolBox->GetSizePixel();
    Size aSize = _pToolBox->CalcWindowSizePixel();
    if ( !aSize.Width() )
        aSize.setWidth( aOldSize.Width() );
    else if ( !aSize.Height() )
        aSize.setHeight( aOldSize.Height() );

    Size aTbSize = _pToolBox->GetSizePixel();
    if ( (aSize.Width() && aSize.Width() != aTbSize.Width()) ||
            (aSize.Height() && aSize.Height() != aTbSize.Height()) )
    {
        _pToolBox->SetPosSizePixel( _pToolBox->GetPosPixel(), aSize );
        _pToolBox->Invalidate();
    }
}

void adjustBrowseBoxColumnWidth( ::svt::EditBrowseBox* _pBox, sal_uInt16 _nColId )
{
    sal_Int32 nColSize = -1;
    sal_uInt32 nDefaultWidth = _pBox->GetDefaultColumnWidth( _pBox->GetColumnTitle( _nColId ) );
    if ( nDefaultWidth != _pBox->GetColumnWidth( _nColId ) )
    {
        Size aSizeMM = _pBox->PixelToLogic( Size( _pBox->GetColumnWidth( _nColId ), 0 ), MapMode( MapUnit::MapMM ) );
        nColSize = aSizeMM.Width() * 10;
    }

    Size aDefaultMM = _pBox->PixelToLogic( Size( nDefaultWidth, 0 ), MapMode( MapUnit::MapMM ) );

    DlgSize aColumnSizeDlg(_pBox->GetFrameWeld(), nColSize, false, aDefaultMM.Width() * 10);
    if (aColumnSizeDlg.run() == RET_OK)
    {
        sal_Int32 nValue = aColumnSizeDlg.GetValue();
        if ( -1 == nValue )
        {   // default width
            nValue = _pBox->GetDefaultColumnWidth( _pBox->GetColumnTitle( _nColId ) );
        }
        else
        {
            Size aSizeMM( nValue / 10, 0 );
            nValue = _pBox->LogicToPixel( aSizeMM, MapMode( MapUnit::MapMM ) ).Width();
        }
        _pBox->SetColumnWidth( _nColId, nValue );
    }
}

// check if SQL92 name checking is enabled
bool isSQL92CheckEnabled(const Reference<XConnection>& _xConnection)
{
    return ::dbtools::getBooleanDataSourceSetting( _xConnection, PROPERTY_ENABLESQL92CHECK );
}

bool isAppendTableAliasEnabled(const Reference<XConnection>& _xConnection)
{
    return ::dbtools::getBooleanDataSourceSetting( _xConnection, INFO_APPEND_TABLE_ALIAS );
}

bool generateAsBeforeTableAlias(const Reference<XConnection>& _xConnection)
{
    return ::dbtools::getBooleanDataSourceSetting( _xConnection, INFO_AS_BEFORE_CORRELATION_NAME );
}

void fillAutoIncrementValue(const Reference<XPropertySet>& _xDatasource,
                            bool& _rAutoIncrementValueEnabled,
                            OUString& _rsAutoIncrementValue)
{
    if ( _xDatasource.is() )
    {
        OSL_ENSURE(_xDatasource->getPropertySetInfo()->hasPropertyByName(PROPERTY_INFO),"NO datasource supplied!");
        Sequence<PropertyValue> aInfo;
        _xDatasource->getPropertyValue(PROPERTY_INFO) >>= aInfo;

        // search the right propertyvalue
        const PropertyValue* pValue =std::find_if(aInfo.begin(), aInfo.end(),
                                            [](const PropertyValue& lhs)
                                            {return lhs.Name == PROPERTY_AUTOINCREMENTCREATION;} );

        if ( pValue != aInfo.end() )
            pValue->Value >>= _rsAutoIncrementValue;
        pValue =std::find_if(aInfo.begin(), aInfo.end(),
                             [](const PropertyValue& lhs)
                             {return lhs.Name == "IsAutoRetrievingEnabled";} );

        if ( pValue != aInfo.end() )
            pValue->Value >>= _rAutoIncrementValueEnabled;
    }
}

void fillAutoIncrementValue(const Reference<XConnection>& _xConnection,
                            bool& _rAutoIncrementValueEnabled,
                            OUString& _rsAutoIncrementValue)
{
    Reference< XChild> xChild(_xConnection,UNO_QUERY);
    if(xChild.is())
    {
        Reference< XPropertySet> xProp(xChild->getParent(),UNO_QUERY);
        fillAutoIncrementValue(xProp,_rAutoIncrementValueEnabled,_rsAutoIncrementValue);
    }
}

OUString getStrippedDatabaseName(const Reference<XPropertySet>& _xDataSource,OUString& _rsDatabaseName)
{
    if ( _rsDatabaseName.isEmpty() && _xDataSource.is() )
    {
        try
        {
            _xDataSource->getPropertyValue(PROPERTY_NAME) >>= _rsDatabaseName;
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
    OUString sName = _rsDatabaseName;
    INetURLObject aURL(sName);
    if ( aURL.GetProtocol() != INetProtocol::NotValid )
        sName = aURL.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DecodeMechanism::Unambiguous);
    return sName;
}

void setEvalDateFormatForFormatter(Reference< css::util::XNumberFormatter > const & _rxFormatter)
{
    OSL_ENSURE( _rxFormatter.is(),"setEvalDateFormatForFormatter: Formatter is NULL!");
    if ( _rxFormatter.is() )
    {
        Reference< css::util::XNumberFormatsSupplier >  xSupplier = _rxFormatter->getNumberFormatsSupplier();

        Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
        SvNumberFormatsSupplierObj* pSupplierImpl = reinterpret_cast<SvNumberFormatsSupplierObj*>(xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));
        OSL_ENSURE(pSupplierImpl,"No Supplier!");

        if ( pSupplierImpl )
        {
            SvNumberFormatter* pFormatter = pSupplierImpl->GetNumberFormatter();
            pFormatter->SetEvalDateFormat(NF_EVALDATEFORMAT_FORMAT);
        }
    }
}

TOTypeInfoSP queryPrimaryKeyType(const OTypeInfoMap& _rTypeInfo)
{
    TOTypeInfoSP pTypeInfo;
    // first we search for a type which supports autoIncrement
    for (auto const& elem : _rTypeInfo)
    {
        // OJ: we don't want to set an autoincrement column to be key
        // because we don't have the possibility to know how to create
        // such auto increment column later on
        // so until we know how to do it, we create a column without autoincrement
        // therefore we have searched
        if ( elem.second->nType == DataType::INTEGER )
        {
            pTypeInfo = elem.second; // alternative
            break;
        }
        else if ( !pTypeInfo.get() && elem.second->nType == DataType::DOUBLE )
            pTypeInfo = elem.second; // alternative
        else if ( !pTypeInfo.get() && elem.second->nType == DataType::REAL )
            pTypeInfo = elem.second; // alternative
    }
    if ( !pTypeInfo.get() ) // just a fallback
        pTypeInfo = queryTypeInfoByType(DataType::VARCHAR,_rTypeInfo);

    OSL_ENSURE(pTypeInfo.get(),"checkColumns: can't find a type which is usable as a key!");
    return pTypeInfo;
}

TOTypeInfoSP queryTypeInfoByType(sal_Int32 _nDataType,const OTypeInfoMap& _rTypeInfo)
{
    OTypeInfoMap::const_iterator aIter = _rTypeInfo.find(_nDataType);
    if(aIter != _rTypeInfo.end())
        return aIter->second;
    // fall back if the type is unknown
    TOTypeInfoSP pTypeInfo;
    switch(_nDataType)
    {
        case DataType::TINYINT:
            if( (pTypeInfo = queryTypeInfoByType(DataType::SMALLINT,_rTypeInfo) ) )
                break;
            [[fallthrough]];
        case DataType::SMALLINT:
            if( (pTypeInfo = queryTypeInfoByType(DataType::INTEGER,_rTypeInfo) ) )
                break;
            [[fallthrough]];
        case DataType::INTEGER:
            if( (pTypeInfo = queryTypeInfoByType(DataType::FLOAT,_rTypeInfo) ) )
                break;
            [[fallthrough]];
        case DataType::FLOAT:
            if( (pTypeInfo = queryTypeInfoByType(DataType::REAL,_rTypeInfo) ) )
                break;
            [[fallthrough]];
        case DataType::DATE:
        case DataType::TIME:
            if( DataType::DATE == _nDataType || DataType::TIME == _nDataType )
            {
                if( (pTypeInfo = queryTypeInfoByType(DataType::TIMESTAMP,_rTypeInfo) ) )
                    break;
            }
            [[fallthrough]];
        case DataType::TIMESTAMP:
        case DataType::REAL:
        case DataType::BIGINT:
            if (  (pTypeInfo = queryTypeInfoByType(DataType::DOUBLE,_rTypeInfo) ) )
                break;
            [[fallthrough]];
        case DataType::DOUBLE:
            if (  (pTypeInfo = queryTypeInfoByType(DataType::NUMERIC,_rTypeInfo) ) )
                break;
            [[fallthrough]];
        case DataType::NUMERIC:
             pTypeInfo = queryTypeInfoByType(DataType::DECIMAL,_rTypeInfo);
            break;
        case DataType::DECIMAL:
            if (  (pTypeInfo = queryTypeInfoByType(DataType::NUMERIC,_rTypeInfo) ) )
                break;
            if (  (pTypeInfo = queryTypeInfoByType(DataType::DOUBLE,_rTypeInfo) ) )
                break;
            break;
        case DataType::VARCHAR:
            if (  (pTypeInfo = queryTypeInfoByType(DataType::LONGVARCHAR,_rTypeInfo) ) )
                break;
            break;
        case DataType::LONGVARCHAR:
            if (  (pTypeInfo = queryTypeInfoByType(DataType::CLOB,_rTypeInfo) ) )
                break;
            break;
        default:
            ;
    }
    if ( !pTypeInfo )
    {
        bool bForce = true;
        pTypeInfo = ::dbaui::getTypeInfoFromType(_rTypeInfo,DataType::VARCHAR,OUString(),"x",50,0,false,bForce);
    }
    OSL_ENSURE(pTypeInfo,"Wrong DataType supplied!");
    return pTypeInfo;
}

sal_Int32 askForUserAction(weld::Window* pParent, const char* pTitle, const char* pText, bool _bAll, const OUString& _sName)
{
    SolarMutexGuard aGuard;
    OUString aMsg = DBA_RES(pText);
    aMsg = aMsg.replaceFirst("%1", _sName);
    OSQLMessageBox aAsk(pParent, DBA_RES(pTitle), aMsg, MessBoxStyle::YesNo | MessBoxStyle::DefaultYes, MessageType::Query);
    if ( _bAll )
    {
        aAsk.add_button(DBA_RES(STR_BUTTON_TEXT_ALL), RET_ALL, HID_CONFIRM_DROP_BUTTON_ALL);
    }
    return aAsk.run();
}

namespace
{
    OUString lcl_createSDBCLevelStatement( const OUString& _rStatement, const Reference< XConnection >& _rxConnection )
    {
        OUString sSDBCLevelStatement( _rStatement );
        try
        {
            Reference< XMultiServiceFactory > xAnalyzerFactory( _rxConnection, UNO_QUERY_THROW );
            Reference< XSingleSelectQueryAnalyzer > xAnalyzer( xAnalyzerFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );
            xAnalyzer->setQuery( _rStatement );
            sSDBCLevelStatement = xAnalyzer->getQueryWithSubstitution();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        return sSDBCLevelStatement;
    }
}

Reference< XPropertySet > createView( const OUString& _rName, const Reference< XConnection >& _rxConnection,
                                    const OUString& _rCommand )
{
    Reference<XViewsSupplier> xSup(_rxConnection,UNO_QUERY);
    Reference< XNameAccess > xViews;
    if(xSup.is())
        xViews = xSup->getViews();
    Reference<XDataDescriptorFactory> xFact(xViews,UNO_QUERY);
    OSL_ENSURE(xFact.is(),"No XDataDescriptorFactory available!");
    if(!xFact.is())
        return nullptr;

    Reference<XPropertySet> xView = xFact->createDataDescriptor();
    if ( !xView.is() )
        return nullptr;

    OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(_rxConnection->getMetaData(),
                                        _rName,
                                        sCatalog,
                                        sSchema,
                                        sTable,
                                        ::dbtools::EComposeRule::InDataManipulation);

    xView->setPropertyValue(PROPERTY_CATALOGNAME,makeAny(sCatalog));
    xView->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(sSchema));
    xView->setPropertyValue(PROPERTY_NAME,makeAny(sTable));

    xView->setPropertyValue( PROPERTY_COMMAND, makeAny( _rCommand ) );

    Reference<XAppend> xAppend(xViews,UNO_QUERY);
    if(xAppend.is())
        xAppend->appendByDescriptor(xView);

    xView = nullptr;
    // we need to reget the view because after appending it, it is no longer valid
    // but this time it isn't a view object it is a table object with type "VIEW"
    Reference<XTablesSupplier> xTabSup(_rxConnection,UNO_QUERY);
    Reference< XNameAccess > xTables;
    if ( xTabSup.is() )
    {
        xTables = xTabSup->getTables();
        if ( xTables.is() && xTables->hasByName( _rName ) )
            xTables->getByName( _rName ) >>= xView;
    }

    return xView;
}

Reference<XPropertySet> createView( const OUString& _rName, const Reference< XConnection >& _rxConnection
                                   ,const Reference<XPropertySet>& _rxSourceObject)
{
    OUString sCommand;
    Reference< XPropertySetInfo > xPSI( _rxSourceObject->getPropertySetInfo(), UNO_SET_THROW );
    if ( xPSI->hasPropertyByName( PROPERTY_COMMAND ) )
    {
        _rxSourceObject->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand;

        bool bEscapeProcessing( false );
        OSL_VERIFY( _rxSourceObject->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bEscapeProcessing );
        if ( bEscapeProcessing )
            sCommand = lcl_createSDBCLevelStatement( sCommand, _rxConnection );
    }
    else
    {
        sCommand =  "SELECT * FROM " + composeTableNameForSelect( _rxConnection, _rxSourceObject );
    }
    return createView( _rName, _rxConnection, sCommand );
}

bool insertHierachyElement(weld::Window* pParent, const Reference< XComponentContext >& _rxContext,
                           const Reference<XHierarchicalNameContainer>& _xNames,
                           const OUString& _sParentFolder,
                           bool _bForm,
                           bool _bCollection,
                           const Reference<XContent>& _xContent,
                           bool _bMove)
{
    OSL_ENSURE( _xNames.is(), "insertHierachyElement: illegal name container!" );
    if ( !_xNames.is() )
        return false;

    Reference<XNameAccess> xNameAccess( _xNames, UNO_QUERY );
    if ( _xNames->hasByHierarchicalName(_sParentFolder) )
    {
        Reference<XChild> xChild(_xNames->getByHierarchicalName(_sParentFolder),UNO_QUERY);
        xNameAccess.set(xChild,UNO_QUERY);
        if ( !xNameAccess.is() && xChild.is() )
            xNameAccess.set(xChild->getParent(),UNO_QUERY);
    }

    OSL_ENSURE( xNameAccess.is(), "insertHierachyElement: could not find the proper name container!" );
    if ( !xNameAccess.is() )
        return false;

    OUString sNewName;
    Reference<XPropertySet> xProp(_xContent,UNO_QUERY);
    if ( xProp.is() )
        xProp->getPropertyValue(PROPERTY_NAME) >>= sNewName;

    if ( !_bMove || sNewName.isEmpty() )
    {
        OUString sTargetName,sLabel;
        if ( sNewName.isEmpty() || xNameAccess->hasByName(sNewName) )
        {
            if ( !sNewName.isEmpty() )
                sTargetName = sNewName;
            else
                sTargetName = DBA_RES( _bCollection ? STR_NEW_FOLDER : ((_bForm) ? RID_STR_FORM : RID_STR_REPORT));
            sLabel = DBA_RES( _bCollection ? STR_FOLDER_LABEL  : ((_bForm) ? STR_FRM_LABEL : STR_RPT_LABEL));
            sTargetName = ::dbtools::createUniqueName(xNameAccess,sTargetName);

            // here we have everything needed to create a new query object ...
            HierarchicalNameCheck aNameChecker( _xNames.get(), _sParentFolder );
            // ... ehm, except a new name
            OSaveAsDlg aAskForName(pParent,
                                   _rxContext,
                                   sTargetName,
                                   sLabel,
                                   aNameChecker,
                                   SADFlags::AdditionalDescription | SADFlags::TitlePasteAs);
            if ( RET_OK != aAskForName.run() )
                // cancelled by the user
                return false;

            sNewName = aAskForName.getName();
        }
    }
    else if ( xNameAccess->hasByName(sNewName) )
    {
        OUString sError(DBA_RES(STR_NAME_ALREADY_EXISTS));
        sError = sError.replaceFirst("#",sNewName);
        throw SQLException(sError,nullptr,"S1000",0,Any());
    }

    try
    {
        Reference<XMultiServiceFactory> xORB( xNameAccess, UNO_QUERY_THROW );
        uno::Sequence<uno::Any> aArguments(comphelper::InitAnyPropertySequence(
        {
            {"Name", uno::Any(sNewName)}, // set as folder
            {"Parent", uno::Any(xNameAccess)},
            {PROPERTY_EMBEDDEDOBJECT, uno::Any(_xContent)},
        }));
        OUString sServiceName(_bCollection ? (_bForm ? OUString(SERVICE_NAME_FORM_COLLECTION) : OUString(SERVICE_NAME_REPORT_COLLECTION)) : OUString(SERVICE_SDB_DOCUMENTDEFINITION));

        Reference<XContent > xNew( xORB->createInstanceWithArguments( sServiceName, aArguments ), UNO_QUERY_THROW );
        Reference< XNameContainer > xNameContainer( xNameAccess, UNO_QUERY_THROW );
        xNameContainer->insertByName( sNewName, makeAny( xNew ) );
    }
    catch( const IllegalArgumentException& e )
    {
        ::dbtools::throwGenericSQLException( e.Message, e.Context );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
        return false;
    }

    return true;
}

Reference< XNumberFormatter > getNumberFormatter(const Reference< XConnection >& _rxConnection, const Reference< css::uno::XComponentContext >& _rxContext )
{
    // create a formatter working with the connections format supplier
    Reference< XNumberFormatter > xFormatter;

    try
    {
        Reference< css::util::XNumberFormatsSupplier >  xSupplier(::dbtools::getNumberFormats(_rxConnection, true, _rxContext));

        if ( xSupplier.is() )
        {
            // create a new formatter
            xFormatter.set(util::NumberFormatter::create( _rxContext ), UNO_QUERY_THROW);
            xFormatter->attachNumberFormatsSupplier(xSupplier);
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return xFormatter;
}

} // dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
