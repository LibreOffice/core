/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "FieldDescriptions.hxx"
#include "TEditControl.hxx"
#include "TableController.hxx"
#include "TableDesignView.hxx"
#include "TableRow.hxx"
#include "TypeInfo.hxx"
#include "UITools.hxx"
#include "browserids.hxx"
#include "dbu_reghelper.hxx"
#include "dbu_tbl.hrc"
#include "dbustrings.hrc"
#include "defaultobjectnamecheck.hxx"
#include "dlgsave.hxx"
#include "dsmeta.hxx"
#include "indexdialog.hxx"
#include "sqlmessage.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XTitleChangeListener.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/streamsection.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbmetadata.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <sfx2/sfxsids.hrc>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>

#include <boost/mem_fn.hpp>
#include <boost/bind.hpp>

#include <algorithm>
#include <functional>

extern "C" void SAL_CALL createRegistryInfo_OTableControl()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OTableController > aAutoRegistration;
}

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::util;
using namespace ::dbtools;
using namespace ::dbaui;
using namespace ::comphelper;


#define NEWCOLS        128

namespace
{
    void dropTable(const Reference<XNameAccess>& _rxTable,const OUString& _sTableName)
    {
        if ( _rxTable->hasByName(_sTableName) )
        {
            Reference<XDrop> xNameCont(_rxTable,UNO_QUERY);
            OSL_ENSURE(xNameCont.is(),"No drop interface for tables!");
            if ( xNameCont.is() )
                xNameCont->dropByName(_sTableName);
        }
    }
}

OUString SAL_CALL OTableController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

OUString OTableController::getImplementationName_Static() throw( RuntimeException )
{
    return OUString("org.openoffice.comp.dbu.OTableDesign");
}

Sequence< OUString> OTableController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< OUString> aSupported(1);
    aSupported[0] = "com.sun.star.sdb.TableDesign";
    return aSupported;
}

Sequence< OUString> SAL_CALL OTableController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

Reference< XInterface > SAL_CALL OTableController::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new OTableController(comphelper::getComponentContext(_rxFactory)));
}

OTableController::OTableController(const Reference< XComponentContext >& _rM) : OTableController_BASE(_rM)
    ,m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_pTypeInfo()
    ,m_bAllowAutoIncrementValue(sal_False)
    ,m_bNew(sal_True)
{

    InvalidateAll();
    m_pTypeInfo = TOTypeInfoSP(new OTypeInfo());
    m_pTypeInfo->aUIName = m_sTypeNames.getToken(TYPE_OTHER, ';');
}

OTableController::~OTableController()
{
    m_aTypeInfoIndex.clear();
    m_aTypeInfo.clear();

}

void OTableController::startTableListening()
{
    Reference< XComponent >  xComponent(m_xTable, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(static_cast<XModifyListener*>(this));
}

void OTableController::stopTableListening()
{
    Reference< XComponent >  xComponent(m_xTable, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(static_cast<XModifyListener*>(this));
}

void OTableController::disposing()
{
    OTableController_BASE::disposing();
    clearView();

    m_vRowList.clear();
}

FeatureState OTableController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    

    switch (_nId)
    {
        case ID_BROWSER_CLOSE:
            aReturn.bEnabled = sal_True;
            break;
        case ID_BROWSER_EDITDOC:
            aReturn.bChecked = isEditable();
            aReturn.bEnabled = m_bNew || isEditable();
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = impl_isModified();
            if ( aReturn.bEnabled )
            {
                ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = ::std::find_if(m_vRowList.begin(),m_vRowList.end(),
                    ::boost::mem_fn(&OTableRow::isValid));
                aReturn.bEnabled = aIter != m_vRowList.end();
            }
            break;
        case ID_BROWSER_SAVEASDOC:
            aReturn.bEnabled = isConnected() && isEditable();
            if ( aReturn.bEnabled )
            {
                ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = ::std::find_if(m_vRowList.begin(),m_vRowList.end(),
                    ::boost::mem_fn(&OTableRow::isValid));
                aReturn.bEnabled = aIter != m_vRowList.end();
            }
            break;

        case ID_BROWSER_CUT:
            aReturn.bEnabled = isEditable() && m_aCurrentFrame.isActive() && getView() && static_cast<OTableDesignView*>(getView())->isCutAllowed();
            break;
        case ID_BROWSER_COPY:
            aReturn.bEnabled = m_aCurrentFrame.isActive() && getView() && static_cast<OTableDesignView*>(getView())->isCopyAllowed();
            break;
        case ID_BROWSER_PASTE:
            aReturn.bEnabled = isEditable() && m_aCurrentFrame.isActive() && getView() && static_cast<OTableDesignView*>(getView())->isPasteAllowed();
            break;
        case SID_INDEXDESIGN:
            aReturn.bEnabled =
                (   (   ((!m_bNew && impl_isModified()) || impl_isModified())
                    ||  Reference< XIndexesSupplier >(m_xTable, UNO_QUERY).is()
                    )
                &&  isConnected()
                );
            if ( aReturn.bEnabled )
            {
                ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = ::std::find_if(m_vRowList.begin(),m_vRowList.end(),
                    ::boost::mem_fn(&OTableRow::isValid));
                aReturn.bEnabled = aIter != m_vRowList.end();
            }
            break;
        default:
            aReturn = OTableController_BASE::GetState(_nId);
    }
    return aReturn;
}

void OTableController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
{
    switch(_nId)
    {
        case ID_BROWSER_EDITDOC:
            setEditable(!isEditable());
            static_cast<OTableDesignView*>(getView())->setReadOnly(!isEditable());
            InvalidateFeature(ID_BROWSER_PASTE);
            InvalidateFeature(SID_BROWSER_CLEAR_QUERY);
            break;
        case ID_BROWSER_SAVEASDOC:
            doSaveDoc(sal_True);
            break;
        case ID_BROWSER_SAVEDOC:
            static_cast<OTableDesignView*>(getView())->GetEditorCtrl()->SaveCurRow();
            doSaveDoc(sal_False);
            break;
        case ID_BROWSER_CUT:
            static_cast<OTableDesignView*>(getView())->cut();
            break;
        case ID_BROWSER_COPY:
            static_cast<OTableDesignView*>(getView())->copy();
            break;
        case ID_BROWSER_PASTE:
            static_cast<OTableDesignView*>(getView())->paste();
            break;
        case SID_INDEXDESIGN:
            doEditIndexes();
            break;
        default:
            OTableController_BASE::Execute(_nId,aArgs);
    }
    InvalidateFeature(_nId);
}

sal_Bool OTableController::doSaveDoc(sal_Bool _bSaveAs)
{
    if (!isConnected())
        reconnect(sal_True); 
    Reference<XTablesSupplier> xTablesSup(getConnection(),UNO_QUERY);

    if (!xTablesSup.is())
    {
        OUString aMessage(ModuleRes(STR_TABLEDESIGN_CONNECTION_MISSING));
        OSQLWarningBox( getView(), aMessage ).Execute();
        return sal_False;
    }

    
    

    Reference<XNameAccess> xTables;
    OUString sCatalog, sSchema;

    sal_Bool bNew = m_sName.isEmpty();
    bNew = bNew || m_bNew || _bSaveAs;

    try
    {
        xTables = xTablesSup->getTables();
        OSL_ENSURE(xTables.is(),"The tables can't be null!");
        bNew = bNew || (xTables.is() && !xTables->hasByName(m_sName));

        
        if(bNew)
        {
            OUString aDefaultName;
            if (_bSaveAs && !bNew)
                 aDefaultName = m_sName;
            else
            {
                OUString aName = ModuleRes(STR_TBL_TITLE);
                aDefaultName = aName.getToken(0,' ');
                aDefaultName = ::dbtools::createUniqueName(xTables,aDefaultName);
            }

            DynamicTableOrQueryNameCheck aNameChecker( getConnection(), CommandType::TABLE );
            OSaveAsDlg aDlg( getView(), CommandType::TABLE, getORB(), getConnection(), aDefaultName, aNameChecker );
            if ( aDlg.Execute() != RET_OK )
                return sal_False;

            m_sName = aDlg.getName();
            sCatalog = aDlg.getCatalog();
            sSchema  = aDlg.getSchema();
        }

        
        if(m_sName.isEmpty())
            return sal_False;
    }
    catch(Exception&)
    {
        OSL_FAIL("OTableController::doSaveDoc: nothing is expected to happen here!");
    }

    sal_Bool bAlter = sal_False;
    sal_Bool bError = sal_False;
    SQLExceptionInfo aInfo;
    try
    {
        
        if(!checkColumns(bNew || !xTables->hasByName(m_sName)))
        {
            return sal_False;
        }

        Reference<XPropertySet> xTable;
        if(bNew || !xTables->hasByName(m_sName)) 
        {
            dropTable(xTables,m_sName);

            Reference<XDataDescriptorFactory> xFact(xTables,UNO_QUERY);
            OSL_ENSURE(xFact.is(),"OTableController::doSaveDoc: No XDataDescriptorFactory available!");
            xTable = xFact->createDataDescriptor();
            OSL_ENSURE(xTable.is(),"OTableController::doSaveDoc: Create query failed!");
            
            xTable->setPropertyValue(PROPERTY_CATALOGNAME,makeAny(sCatalog));
            xTable->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(sSchema));
            xTable->setPropertyValue(PROPERTY_NAME,makeAny(m_sName));

            
            Reference<XColumnsSupplier> xColSup(xTable,UNO_QUERY);
            appendColumns(xColSup,bNew);
            
            Reference<XKeysSupplier> xKeySup(xTable,UNO_QUERY);
            appendPrimaryKey(xKeySup,bNew);
        }
        
        if(bNew)
        {
            Reference<XAppend> xAppend(xTables,UNO_QUERY);
            OSL_ENSURE(xAppend.is(),"OTableController::doSaveDoc: No XAppend Interface!");
            xAppend->appendByDescriptor(xTable);

            assignTable();
            if(!m_xTable.is()) 
            {
                
                m_sName = ::dbtools::composeTableName( getConnection()->getMetaData(), xTable, ::dbtools::eInDataManipulation, false, false, false );
                assignTable();
            }
            
            ::dbaui::appendToFilter(getConnection(),m_sName,getORB(),getView()); 
            Reference< frame::XTitleChangeListener> xEventListener(impl_getTitleHelper_throw(),UNO_QUERY);
            if ( xEventListener.is() )
            {
                frame::TitleChangedEvent aEvent;
                xEventListener->titleChanged(aEvent);
            }
            releaseNumberForComponent();
        }
        else if(m_xTable.is())
        {
            bAlter = sal_True;
            alterColumns();
        }
        reSyncRows();
    }
    catch(const SQLContext& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch(const SQLWarning& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch(const SQLException& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch(const ElementExistException& )
    {
        OUString sText( ModuleRes( STR_NAME_ALREADY_EXISTS ) );
        sText = sText.replaceFirst( "#" , m_sName);
        OSQLMessageBox aDlg( getView(), OUString( ModuleRes( STR_ERROR_DURING_CREATION ) ), sText, WB_OK, OSQLMessageBox::Error );

        aDlg.Execute();
        bError = sal_True;
    }
    catch( const Exception& )
    {
        bError = sal_True;
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( aInfo.isValid() )
        aInfo.prepend( OUString( ModuleRes( STR_TABLEDESIGN_SAVE_ERROR ) ) );
    showError(aInfo);

    if (aInfo.isValid() || bError)
    {
        if(!bAlter || bNew)
        {
            m_sName = "";
            stopTableListening();
            m_xTable = NULL;
        }
    }
    return ! (aInfo.isValid() || bError);
}

void OTableController::doEditIndexes()
{
    
    if (m_bNew || isModified())
    {
        QueryBox aAsk(getView(), ModuleRes(QUERY_SAVE_TABLE_EDIT_INDEXES));
        if (RET_YES != aAsk.Execute())
            return;

        if (!doSaveDoc(sal_False))
            return;

        OSL_ENSURE(!m_bNew && !isModified(), "OTableController::doEditIndexes: what the hell did doSaveDoc do?");
    }

    Reference< XNameAccess > xIndexes;          
    Sequence< OUString > aFieldNames;    
    try
    {
        
        Reference< XIndexesSupplier > xIndexesSupp(m_xTable, UNO_QUERY);
        if (xIndexesSupp.is())
        {
            xIndexes = xIndexesSupp->getIndexes();
            OSL_ENSURE(xIndexes.is(), "OTableController::doEditIndexes: no keys got from the indexes supplier!");
        }
        else
            OSL_FAIL("OTableController::doEditIndexes: should never have reached this (no indexes supplier)!");

        
        Reference< XColumnsSupplier > xColSupp(m_xTable, UNO_QUERY);
        OSL_ENSURE(xColSupp.is(), "OTableController::doEditIndexes: no columns supplier!");
        if (xColSupp.is())
        {
            Reference< XNameAccess > xCols = xColSupp->getColumns();
            OSL_ENSURE(xCols.is(), "OTableController::doEditIndexes: no columns!");
            if (xCols.is())
                aFieldNames = xCols->getElementNames();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if (!xIndexes.is())
        return;

    DbaIndexDialog aDialog(getView(), aFieldNames, xIndexes, getConnection(), getORB(), isConnected() && getConnection()->getMetaData().is() ? getConnection()->getMetaData()->getMaxColumnsInIndex() : 0);
    if (RET_OK != aDialog.Execute())
        return;

}

void OTableController::impl_initialize()
{
    try
    {
        OTableController_BASE::impl_initialize();

        const NamedValueCollection& rArguments( getInitParams() );

        rArguments.get_ensureType( (OUString)PROPERTY_CURRENTTABLE, m_sName );

        
        ::dbaui::fillAutoIncrementValue(getDataSource(),m_bAllowAutoIncrementValue,m_sAutoIncrementValue);

        assignTable();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    try
    {
        ::dbaui::fillTypeInfo(getConnection(),m_sTypeNames,m_aTypeInfo,m_aTypeInfoIndex);               
    }
    catch(const SQLException&)
    {
        OSQLWarningBox( getView(), ModuleRes( STR_NO_TYPE_INFO_AVAILABLE ) ).Execute();
        throw;
    }
    try
    {
        loadData();                 
        getView()->initialize();    
        ClearUndoManager();
        setModified(sal_False);     
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

sal_Bool OTableController::Construct(Window* pParent)
{
    setView( * new OTableDesignView( pParent, getORB(), *this ) );
    OTableController_BASE::Construct(pParent);
    return sal_True;
}

sal_Bool SAL_CALL OTableController::suspend(sal_Bool /*_bSuspend*/) throw( RuntimeException )
{
    if ( getBroadcastHelper().bInDispose || getBroadcastHelper().bDisposed )
        return sal_True;

    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    if ( getView() && getView()->IsInModalMode() )
        return sal_False;
    if ( getView() )
        static_cast<OTableDesignView*>(getView())->GrabFocus();
    sal_Bool bCheck = sal_True;
    if ( isModified() )
    {
        ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aIter = ::std::find_if(m_vRowList.begin(),m_vRowList.end(),
            ::boost::mem_fn(&OTableRow::isValid));
        if ( aIter != m_vRowList.end() )
        {
            QueryBox aQry(getView(), ModuleRes(TABLE_DESIGN_SAVEMODIFIED));
            switch (aQry.Execute())
            {
                case RET_YES:
                    Execute(ID_BROWSER_SAVEDOC,Sequence<PropertyValue>());
                    if ( isModified() )
                        bCheck = sal_False; 
                    break;
                case RET_CANCEL:
                    bCheck = sal_False;
                default:
                    break;
            }
        }
        else if ( !m_bNew )
        {
            QueryBox aQry(getView(), ModuleRes(TABLE_DESIGN_ALL_ROWS_DELETED));
            switch (aQry.Execute())
            {
                case RET_YES:
                    {
                        try
                        {
                            Reference<XTablesSupplier> xTablesSup(getConnection(),UNO_QUERY);
                            Reference<XNameAccess> xTables = xTablesSup->getTables();
                            dropTable(xTables,m_sName);
                        }
                        catch(const Exception&)
                        {
                            OSL_FAIL("OTableController::suspend: nothing is expected to happen here!");
                        }

                    }
                    break;
                case RET_CANCEL:
                    bCheck = sal_False;
                default:
                    break;
            }
        }
    }

    return bCheck;
}

void OTableController::describeSupportedFeatures()
{
    OSingleDocumentController::describeSupportedFeatures();

    implDescribeSupportedFeature( ".uno:Redo",          ID_BROWSER_REDO,        CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Save",          ID_BROWSER_SAVEDOC,     CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Undo",          ID_BROWSER_UNDO,        CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:HelpMenu",      SID_HELPMENU,           CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:NewDoc",        SID_NEWDOC,             CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:SaveAs",        ID_BROWSER_SAVEASDOC,   CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:DBIndexDesign", SID_INDEXDESIGN,        CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:EditDoc",       ID_BROWSER_EDITDOC,     CommandGroup::EDIT );
}

void OTableController::impl_onModifyChanged()
{
    OSingleDocumentController::impl_onModifyChanged();
    InvalidateFeature( SID_INDEXDESIGN );
}

void SAL_CALL OTableController::disposing( const EventObject& _rSource ) throw(RuntimeException)
{
    if ( _rSource.Source == m_xTable )
    {   
        stopTableListening();
        m_xTable    = NULL;
        m_bNew      = sal_True;
        setModified(sal_True);
    }
    else
        OTableController_BASE::disposing( _rSource );
}

void OTableController::Save(const Reference< XObjectOutputStream>& _rxOut)
{
    OStreamSection aSection(_rxOut.get());

}

void OTableController::Load(const Reference< XObjectInputStream>& _rxIn)
{
    OStreamSection aSection(_rxIn.get());
}

void OTableController::losingConnection( )
{
    
    OTableController_BASE::losingConnection( );

    
    Reference< XComponent >  xComponent(m_xTable, UNO_QUERY);
    if (xComponent.is())
    {
        Reference<XEventListener> xEvtL( static_cast< ::cppu::OWeakObject*>(this), UNO_QUERY);
        xComponent->removeEventListener(xEvtL);
    }
    stopTableListening();
    m_xTable    = NULL;
    assignTable();
    if(!m_xTable.is())
    {
        m_bNew      = sal_True;
        setModified(sal_True);
    }
    InvalidateAll();
}

TOTypeInfoSP OTableController::getTypeInfoByType(sal_Int32 _nDataType) const
{
    return queryTypeInfoByType(_nDataType,m_aTypeInfo);
}

void OTableController::appendColumns(Reference<XColumnsSupplier>& _rxColSup,sal_Bool _bNew,sal_Bool _bKeyColumns)
{
    try
    {
        
        OSL_ENSURE(_rxColSup.is(),"No columns supplier");
        if(!_rxColSup.is())
            return;
        Reference<XNameAccess> xColumns = _rxColSup->getColumns();
        OSL_ENSURE(xColumns.is(),"No columns");
        Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);

        Reference<XAppend> xAppend(xColumns,UNO_QUERY);
        OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

        ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aIter = m_vRowList.begin();
        ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aEnd = m_vRowList.end();
        for(;aIter != aEnd;++aIter)
        {
            OSL_ENSURE(*aIter,"OTableRow is null!");
            OFieldDescription* pField = (*aIter)->GetActFieldDescr();
            if ( !pField || (!_bNew && (*aIter)->IsReadOnly() && !_bKeyColumns) )
                continue;

            Reference<XPropertySet> xColumn;
            if(pField->IsPrimaryKey() || !_bKeyColumns)
                xColumn = xColumnFactory->createDataDescriptor();
            if(xColumn.is())
            {
                if(!_bKeyColumns)
                    ::dbaui::setColumnProperties(xColumn,pField);
                else
                    xColumn->setPropertyValue(PROPERTY_NAME,makeAny(pField->GetName()));

                xAppend->appendByDescriptor(xColumn);
                xColumn = NULL;
                
                if(xColumns->hasByName(pField->GetName()))
                {
                    xColumns->getByName(pField->GetName()) >>= xColumn;
                    if(xColumn.is())
                        pField->copyColumnSettingsTo(xColumn);
                }
                else
                {
                    OSL_FAIL("OTableController::appendColumns: invalid field name!");
                }

            }
        }
    }
    catch(const SQLException& )
    {
        showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OTableController::appendPrimaryKey(Reference<XKeysSupplier>& _rxSup,sal_Bool _bNew)
{
    if(!_rxSup.is())
        return; 

    OSL_ENSURE(_rxSup.is(),"No XKeysSupplier!");
    Reference<XIndexAccess> xKeys(_rxSup->getKeys(),UNO_QUERY);
    Reference<XPropertySet> xProp;
    if (!xKeys.is())
        return;
    const sal_Int32 nCount = xKeys->getCount();
    for(sal_Int32 i=0;i< nCount ;++i)
    {
        xKeys->getByIndex(i) >>= xProp;
        sal_Int32 nKeyType = 0;
        xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
        if(KeyType::PRIMARY == nKeyType)
        {
            return; 
        }
    }
    Reference<XDataDescriptorFactory> xKeyFactory(xKeys,UNO_QUERY);
    OSL_ENSURE(xKeyFactory.is(),"No XDataDescriptorFactory Interface!");
    if ( !xKeyFactory.is() )
        return;
    Reference<XAppend> xAppend(xKeyFactory,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    Reference<XPropertySet> xKey = xKeyFactory->createDataDescriptor();
    OSL_ENSURE(xKey.is(),"Key is null!");
    xKey->setPropertyValue(PROPERTY_TYPE,makeAny(KeyType::PRIMARY));

    Reference<XColumnsSupplier> xColSup(xKey,UNO_QUERY);
    if(xColSup.is())
    {
        appendColumns(xColSup,_bNew,sal_True);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        if(xColumns->hasElements())
            xAppend->appendByDescriptor(xKey);
    }
}

void OTableController::loadData()
{
    
    m_vRowList.clear();

    ::boost::shared_ptr<OTableRow>  pTabEdRow;
    Reference< XDatabaseMetaData> xMetaData = getMetaData( );
    
    if(m_xTable.is() && xMetaData.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xTable,UNO_QUERY);
        OSL_ENSURE(xColSup.is(),"No XColumnsSupplier!");
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        OFieldDescription* pActFieldDescr = NULL;
        
        
        
        
        
        sal_Bool bIsAlterAllowed = isAlterAllowed();
        Sequence< OUString> aColumns = xColumns->getElementNames();
        const OUString* pIter    = aColumns.getConstArray();
        const OUString* pEnd     = pIter + aColumns.getLength();

        for(;pIter != pEnd;++pIter)
        {
            Reference<XPropertySet> xColumn;
            xColumns->getByName(*pIter) >>= xColumn;
            sal_Int32 nType         = 0;
            sal_Int32 nScale        = 0;
            sal_Int32 nPrecision    = 0;
            sal_Int32 nNullable     = 0;
            sal_Int32 nFormatKey    = 0;
            sal_Int32 nAlign        = 0;

            sal_Bool bIsAutoIncrement = false, bIsCurrency = false;
            OUString sName,sDescription,sTypeName,sHelpText;
            Any aControlDefault;

            
            xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
            xColumn->getPropertyValue(PROPERTY_TYPENAME)        >>= sTypeName;
            xColumn->getPropertyValue(PROPERTY_ISNULLABLE)      >>= nNullable;
            xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT) >>= bIsAutoIncrement;
            xColumn->getPropertyValue(PROPERTY_ISCURRENCY)      >>= bIsCurrency;
            xColumn->getPropertyValue(PROPERTY_TYPE)            >>= nType;
            xColumn->getPropertyValue(PROPERTY_SCALE)           >>= nScale;
            xColumn->getPropertyValue(PROPERTY_PRECISION)       >>= nPrecision;
            xColumn->getPropertyValue(PROPERTY_DESCRIPTION)     >>= sDescription;

            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_HELPTEXT))
                xColumn->getPropertyValue(PROPERTY_HELPTEXT)    >>= sHelpText;

            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_CONTROLDEFAULT))
                aControlDefault = xColumn->getPropertyValue(PROPERTY_CONTROLDEFAULT);
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
                xColumn->getPropertyValue(PROPERTY_FORMATKEY)   >>= nFormatKey;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
                xColumn->getPropertyValue(PROPERTY_ALIGN)       >>= nAlign;

            pTabEdRow.reset(new OTableRow());
            pTabEdRow->SetReadOnly(!bIsAlterAllowed);
            
            sal_Bool bForce;
            OUString sCreate("x");
            TOTypeInfoSP pTypeInfo = ::dbaui::getTypeInfoFromType(m_aTypeInfo,nType,sTypeName,sCreate,nPrecision,nScale,bIsAutoIncrement,bForce);
            if ( !pTypeInfo.get() )
                pTypeInfo = m_pTypeInfo;
            pTabEdRow->SetFieldType( pTypeInfo, bForce );

            pActFieldDescr = pTabEdRow->GetActFieldDescr();
            OSL_ENSURE(pActFieldDescr, "OTableController::loadData: invalid field description generated by the table row!");
            if ( pActFieldDescr )
            {
                pActFieldDescr->SetName(sName);
                pActFieldDescr->SetFormatKey(nFormatKey);
                pActFieldDescr->SetDescription(sDescription);
                pActFieldDescr->SetHelpText(sHelpText);
                pActFieldDescr->SetAutoIncrement(bIsAutoIncrement);
                pActFieldDescr->SetHorJustify(dbaui::mapTextJustify(nAlign));
                pActFieldDescr->SetCurrency(bIsCurrency);

                
                pActFieldDescr->SetIsNullable(nNullable);
                pActFieldDescr->SetControlDefault(aControlDefault);
                pActFieldDescr->SetPrecision(nPrecision);
                pActFieldDescr->SetScale(nScale);
            }
            m_vRowList.push_back( pTabEdRow);
        }
        
        Reference<XNameAccess> xKeyColumns  = getKeyColumns();
        if(xKeyColumns.is())
        {
            Sequence< OUString> aKeyColumns = xKeyColumns->getElementNames();
            const OUString* pKeyBegin    = aKeyColumns.getConstArray();
            const OUString* pKeyEnd      = pKeyBegin + aKeyColumns.getLength();

            for(;pKeyBegin != pKeyEnd;++pKeyBegin)
            {
                ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator rowIter = m_vRowList.begin();
                ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator rowEnd = m_vRowList.end();
                for(;rowIter != rowEnd;++rowIter)
                {
                    if((*rowIter)->GetActFieldDescr()->GetName() == *pKeyBegin)
                    {
                        (*rowIter)->SetPrimaryKey(sal_True);
                        break;
                    }
                }
            }
        }
    }

    

    OTypeInfoMap::iterator aTypeIter = m_aTypeInfo.find(DataType::VARCHAR);
    if(aTypeIter == m_aTypeInfo.end())
        aTypeIter = m_aTypeInfo.begin();

    OSL_ENSURE(aTypeIter != m_aTypeInfo.end(),"We have no type information!");

    bool bReadRow = !isAddAllowed();
    for(sal_Int32 i=m_vRowList.size(); i < NEWCOLS; i++ )
    {
        pTabEdRow.reset(new OTableRow());
        pTabEdRow->SetReadOnly(bReadRow);
        m_vRowList.push_back( pTabEdRow);
    }
}

Reference<XNameAccess> OTableController::getKeyColumns() const
{
    return getPrimaryKeyColumns_throw(m_xTable);
}

sal_Bool OTableController::checkColumns(sal_Bool _bNew) throw(::com::sun::star::sdbc::SQLException)
{
    sal_Bool bOk = sal_True;
    sal_Bool bFoundPKey = sal_False;
    Reference< XDatabaseMetaData > xMetaData = getMetaData( );
    DatabaseMetaData aMetaData( getConnection() );

    ::comphelper::UStringMixEqual bCase(xMetaData.is() ? xMetaData->supportsMixedCaseQuotedIdentifiers() : sal_True);
    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = m_vRowList.begin();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aEnd = m_vRowList.end();
    for(;aIter != aEnd;++aIter)
    {
        OFieldDescription* pFieldDesc = (*aIter)->GetActFieldDescr();
        if (pFieldDesc && !pFieldDesc->GetName().isEmpty())
        {
            bFoundPKey |=  (*aIter)->IsPrimaryKey();
            
            ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter2 = aIter+1;
            for(;aIter2 != aEnd;++aIter2)
            {
                OFieldDescription* pCompareDesc = (*aIter2)->GetActFieldDescr();
                if (pCompareDesc && bCase(pCompareDesc->GetName(),pFieldDesc->GetName()))
                {
                    OUString strMessage = ModuleRes(STR_TABLEDESIGN_DUPLICATE_NAME);
                    strMessage = strMessage.replaceFirst("$column$", pFieldDesc->GetName());
                    OSQLWarningBox( getView(), strMessage ).Execute();
                    return sal_False;
                }
            }
        }
    }
    if ( _bNew && !bFoundPKey && aMetaData.supportsPrimaryKeys() )
    {
        OUString sTitle(ModuleRes(STR_TABLEDESIGN_NO_PRIM_KEY_HEAD));
        OUString sMsg(ModuleRes(STR_TABLEDESIGN_NO_PRIM_KEY));
        OSQLMessageBox aBox(getView(), sTitle,sMsg, WB_YES_NO_CANCEL | WB_DEF_YES);

        switch ( aBox.Execute() )
        {
        case RET_YES:
        {
            ::boost::shared_ptr<OTableRow>  pNewRow(new OTableRow());
            TOTypeInfoSP pTypeInfo = ::dbaui::queryPrimaryKeyType(m_aTypeInfo);
            if ( !pTypeInfo.get() )
                break;

            pNewRow->SetFieldType( pTypeInfo );
            OFieldDescription* pActFieldDescr = pNewRow->GetActFieldDescr();

            pActFieldDescr->SetAutoIncrement(sal_False);
            pActFieldDescr->SetIsNullable(ColumnValue::NO_NULLS);

            pActFieldDescr->SetName( createUniqueName(OUString("ID") ));
            pActFieldDescr->SetPrimaryKey( sal_True );
            m_vRowList.insert(m_vRowList.begin(),pNewRow);

            static_cast<OTableDesignView*>(getView())->GetEditorCtrl()->Invalidate();
            static_cast<OTableDesignView*>(getView())->GetEditorCtrl()->RowInserted(0);
        }
        break;
        case RET_CANCEL:
            bOk = sal_False;
            break;
        }
    }
    return bOk;
}

void OTableController::alterColumns()
{
    Reference<XColumnsSupplier> xColSup(m_xTable,UNO_QUERY_THROW);
    OSL_ENSURE(xColSup.is(),"What happen here?!");

    Reference<XNameAccess> xColumns = xColSup->getColumns();
    Reference<XIndexAccess> xIdxColumns(xColumns,UNO_QUERY_THROW);
    OSL_ENSURE(xColumns.is(),"No columns");
    if ( !xColumns.is() )
        return;
    Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);  

    sal_Int32 nColumnCount = xIdxColumns->getCount();
    Reference<XDrop> xDrop(xColumns,UNO_QUERY);         
    Reference<XAppend> xAppend(xColumns,UNO_QUERY);     
    Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY); 

    sal_Bool bReload = sal_False; 

    
    Reference< XDatabaseMetaData> xMetaData = getMetaData( );

    ::std::map< OUString,sal_Bool,::comphelper::UStringMixLess> aColumns(xMetaData.is() ? (xMetaData->supportsMixedCaseQuotedIdentifiers() ? true : false): sal_True);
    ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aIter = m_vRowList.begin();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aEnd = m_vRowList.end();
    
    sal_Int32 nPos = 0;
    for(;aIter != aEnd;++aIter,++nPos)
    {
        OSL_ENSURE(*aIter,"OTableRow is null!");
        OFieldDescription* pField = (*aIter)->GetActFieldDescr();
        if ( !pField )
            continue;
        if ( (*aIter)->IsReadOnly() )
        {
            aColumns[pField->GetName()] = sal_True;
            continue;
        }

        Reference<XPropertySet> xColumn;
        if ( xColumns->hasByName(pField->GetName()) )
        {
            aColumns[pField->GetName()] = sal_True;
            xColumns->getByName(pField->GetName()) >>= xColumn;
            OSL_ENSURE(xColumn.is(),"Column is null!");

            sal_Int32 nType=0,nPrecision=0,nScale=0,nNullable=0;
            sal_Bool bAutoIncrement = false;
            OUString sTypeName,sDescription;

            xColumn->getPropertyValue(PROPERTY_TYPE)            >>= nType;
            xColumn->getPropertyValue(PROPERTY_PRECISION)       >>= nPrecision;
            xColumn->getPropertyValue(PROPERTY_SCALE)           >>= nScale;
            xColumn->getPropertyValue(PROPERTY_ISNULLABLE)      >>= nNullable;
            xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT) >>= bAutoIncrement;
            xColumn->getPropertyValue(PROPERTY_DESCRIPTION)     >>= sDescription;

            try { xColumn->getPropertyValue(PROPERTY_TYPENAME) >>= sTypeName; }
            catch( const Exception& )
            {
                OSL_FAIL( "no TypeName property?!" );
                
                
                
                
            }

            
            if((nType != pField->GetType()                  ||
                sTypeName != pField->GetTypeName()         ||
                (nPrecision != pField->GetPrecision() && nPrecision )       ||
                nScale != pField->GetScale()                ||
                nNullable != pField->GetIsNullable()        ||
                sDescription != pField->GetDescription()        ||
                bAutoIncrement != pField->IsAutoIncrement())&&
                xColumnFactory.is())
            {
                Reference<XPropertySet> xNewColumn;
                xNewColumn = xColumnFactory->createDataDescriptor();
                ::dbaui::setColumnProperties(xNewColumn,pField);
                
                sal_Bool bNotOk = sal_False;
                try
                {
                    
                    if(xAlter.is())
                        xAlter->alterColumnByName(pField->GetName(),xNewColumn);
                }
                catch(const SQLException&)
                {
                    if(xDrop.is() && xAppend.is())
                    {
                        OUString aMessage( ModuleRes( STR_TABLEDESIGN_ALTER_ERROR ) );
                        aMessage = aMessage.replaceFirst( "$column$", pField->GetName() );

                        SQLExceptionInfo aError( ::cppu::getCaughtException() );
                        OSQLWarningBox aMsg( getView(), aMessage, WB_YES_NO | WB_DEF_YES , &aError );
                        bNotOk = aMsg.Execute() == RET_YES;
                    }
                    else
                        throw;
                }
                
                
                if((!xAlter.is() || bNotOk) && xDrop.is() && xAppend.is())
                {
                    xDrop->dropByName(pField->GetName());
                    try
                    {
                        xAppend->appendByDescriptor(xNewColumn);
                    }
                    catch(const SQLException&)
                    { 
                        xAppend->appendByDescriptor(xColumn);
                        throw;
                    }
                }
                
                xNewColumn = NULL;
                if(xColumns->hasByName(pField->GetName()))
                    xColumns->getByName(pField->GetName()) >>= xColumn;
                bReload = sal_True;
            }

        }
        else if(xColumnFactory.is() && xAlter.is() && nPos < nColumnCount)
        { 
            try
            {
                Reference<XPropertySet> xNewColumn;
                xNewColumn = xColumnFactory->createDataDescriptor();
                ::dbaui::setColumnProperties(xNewColumn,pField);
                xAlter->alterColumnByIndex(nPos,xNewColumn);
                if(xColumns->hasByName(pField->GetName()))
                {   
                    aColumns[pField->GetName()] = sal_True;
                    xColumns->getByName(pField->GetName()) >>= xColumn;
                    if(xColumn.is())
                        pField->copyColumnSettingsTo(xColumn);
                }
                else
                {
                    OSL_FAIL("OTableController::alterColumns: invalid column (2)!");
                }
            }
            catch(const SQLException&)
            { 
                bReload = sal_True;
                if(xDrop.is() && xAppend.is())
                {
                    OUString aMessage(ModuleRes(STR_TABLEDESIGN_ALTER_ERROR));
                    aMessage = aMessage.replaceFirst("$column$",pField->GetName());
                    OSQLWarningBox aMsg( getView(), aMessage, WB_YES_NO | WB_DEF_YES );
                    if ( aMsg.Execute() != RET_YES )
                    {
                        Reference<XPropertySet> xNewColumn(xIdxColumns->getByIndex(nPos),UNO_QUERY_THROW);
                        OUString sName;
                        xNewColumn->getPropertyValue(PROPERTY_NAME) >>= sName;
                        aColumns[sName] = sal_True;
                        aColumns[pField->GetName()] = sal_True;
                        continue;
                    }
                }
                else
                    throw;
            }
        }
        else
            bReload = sal_True;
    }
    
    aIter = m_vRowList.begin();

    
    for(nPos = 0;aIter != aEnd;++aIter,++nPos)
    {
        OSL_ENSURE(*aIter,"OTableRow is null!");
        OFieldDescription* pField = (*aIter)->GetActFieldDescr();
        if ( !pField )
            continue;
        if ( (*aIter)->IsReadOnly() )
        {
            aColumns[pField->GetName()] = sal_True;
            continue;
        }

        Reference<XPropertySet> xColumn;
        if ( xColumns->hasByName(pField->GetName()) )
        {
            xColumns->getByName(pField->GetName()) >>= xColumn;
            Reference<XPropertySetInfo> xInfo = xColumn->getPropertySetInfo();
            if ( xInfo->hasPropertyByName(PROPERTY_HELPTEXT) )
                xColumn->setPropertyValue(PROPERTY_HELPTEXT,makeAny(pField->GetHelpText()));

            if(xInfo->hasPropertyByName(PROPERTY_CONTROLDEFAULT))
                xColumn->setPropertyValue(PROPERTY_CONTROLDEFAULT,pField->GetControlDefault());
            if(xInfo->hasPropertyByName(PROPERTY_FORMATKEY))
                xColumn->setPropertyValue(PROPERTY_FORMATKEY,makeAny(pField->GetFormatKey()));
            if(xInfo->hasPropertyByName(PROPERTY_ALIGN))
                xColumn->setPropertyValue(PROPERTY_ALIGN,makeAny(dbaui::mapTextAllign(pField->GetHorJustify())));
        }
    }
    
    Reference<XNameAccess> xKeyColumns  = getKeyColumns();
    
    if ( xDrop.is() )
    {
        Sequence< OUString> aColumnNames = xColumns->getElementNames();
        const OUString* pIter = aColumnNames.getConstArray();
        const OUString* pEnd = pIter + aColumnNames.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if(aColumns.find(*pIter) == aColumns.end()) 
            {
                if(xKeyColumns.is() && xKeyColumns->hasByName(*pIter)) 
                {
                    OUString aMsgT(ModuleRes(STR_TBL_COLUMN_IS_KEYCOLUMN));
                    aMsgT = aMsgT.replaceFirst("$column$",*pIter);
                    OUString aTitle(ModuleRes(STR_TBL_COLUMN_IS_KEYCOLUMN_TITLE));
                    OSQLMessageBox aMsg(getView(),aTitle,aMsgT,WB_YES_NO| WB_DEF_YES);
                    if(aMsg.Execute() == RET_YES)
                    {
                        xKeyColumns = NULL;
                        dropPrimaryKey();
                    }
                    else
                    {
                        bReload = sal_True;
                        continue;
                    }
                }
                try
                {
                    xDrop->dropByName(*pIter);
                }
                catch (const SQLException&)
                {
                    OUString sError( ModuleRes( STR_TABLEDESIGN_COULD_NOT_DROP_COL ) );
                    sError = sError.replaceFirst( "$column$", *pIter );

                    SQLException aNewException;
                    aNewException.Message = sError;
                    aNewException.SQLState = "S1000";
                    aNewException.NextException = ::cppu::getCaughtException();

                    throw aNewException;
                }
            }
        }
    }

    
    aIter = m_vRowList.begin();
    for(;aIter != aEnd;++aIter)
    {
        OSL_ENSURE(*aIter,"OTableRow is null!");
        OFieldDescription* pField = (*aIter)->GetActFieldDescr();
        if ( !pField || (*aIter)->IsReadOnly() || aColumns.find(pField->GetName()) != aColumns.end() )
            continue;

        Reference<XPropertySet> xColumn;
        if(!xColumns->hasByName(pField->GetName()))
        {
            if(xColumnFactory.is() && xAppend.is())
            {
                
                xColumn = xColumnFactory->createDataDescriptor();
                ::dbaui::setColumnProperties(xColumn,pField);
                xAppend->appendByDescriptor(xColumn);
                if(xColumns->hasByName(pField->GetName()))
                {   
                    aColumns[pField->GetName()] = sal_True;
                    xColumns->getByName(pField->GetName()) >>= xColumn;
                    if(xColumn.is())
                        pField->copyColumnSettingsTo(xColumn);
                }
                else
                {
                    OSL_FAIL("OTableController::alterColumns: invalid column!");
                }
            }
        }
    }

    
    sal_Bool bNeedDropKey = sal_False;
    sal_Bool bNeedAppendKey = sal_False;
    if ( xKeyColumns.is() )
    {
        aIter = m_vRowList.begin();
        for(;aIter != aEnd;++aIter)
        {
            OSL_ENSURE(*aIter,"OTableRow is null!");
            OFieldDescription* pField = (*aIter)->GetActFieldDescr();
            if ( !pField )
                continue;

            if  (   pField->IsPrimaryKey()
                &&  !xKeyColumns->hasByName( pField->GetName() )
                )
            {   
                bNeedDropKey = bNeedAppendKey = sal_True;
                break;
            }
            else if (   !pField->IsPrimaryKey()
                    &&  xKeyColumns->hasByName( pField->GetName() )
                    )
            {   
                bNeedDropKey = bNeedAppendKey = sal_True;
                break;
            }
        }
    }
    else
    {   
        bNeedAppendKey = sal_True;
    }

    if ( bNeedDropKey && xKeyColumns.is() && xKeyColumns->getElementNames().getLength() )
        dropPrimaryKey();

    if ( bNeedAppendKey )
    {
        Reference< XKeysSupplier > xKeySup( m_xTable, UNO_QUERY );
        appendPrimaryKey( xKeySup ,sal_False);
    }

    reSyncRows();

    if ( bReload )
        reload();
}

void OTableController::dropPrimaryKey()
{
    SQLExceptionInfo aInfo;
    try
    {
        Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
        Reference<XIndexAccess> xKeys;
        if(xKeySup.is())
            xKeys = xKeySup->getKeys();

        if(xKeys.is())
        {
            Reference<XPropertySet> xProp;
            for(sal_Int32 i=0;i< xKeys->getCount();++i)
            {
                xProp.set(xKeys->getByIndex(i),UNO_QUERY);
                sal_Int32 nKeyType = 0;
                xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                if(KeyType::PRIMARY == nKeyType)
                {
                    Reference<XDrop> xDrop(xKeys,UNO_QUERY);
                    xDrop->dropByIndex(i); 
                    break;
                }
            }
        }
    }
    catch(const SQLContext& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch(const SQLWarning& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch(const SQLException& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    showError(aInfo);
}

void OTableController::assignTable()
{
    
    if(!m_sName.isEmpty())
    {
        Reference<XNameAccess> xNameAccess;
        Reference<XTablesSupplier> xSup(getConnection(),UNO_QUERY);
        if(xSup.is())
        {
            xNameAccess = xSup->getTables();
            OSL_ENSURE(xNameAccess.is(),"no nameaccess for the queries!");

            Reference<XPropertySet> xProp;
            if(xNameAccess->hasByName(m_sName) && ::cppu::extractInterface(xProp,xNameAccess->getByName(m_sName)) && xProp.is())
            {
                m_xTable = xProp;
                startTableListening();

                
                Reference<XDatabaseMetaData> xMeta = getConnection()->getMetaData();
                setEditable( xMeta.is() && !xMeta->isReadOnly() && (isAlterAllowed() || isDropAllowed() || isAddAllowed()) );
                if(!isEditable())
                {
                    ::std::for_each(m_vRowList.begin(),m_vRowList.end(),boost::bind( &OTableRow::SetReadOnly, _1, boost::cref( sal_True )));
                }
                m_bNew = sal_False;
                
                InvalidateAll();
            }
        }
    }
}

sal_Bool OTableController::isAddAllowed() const
{
    Reference<XColumnsSupplier> xColsSup(m_xTable,UNO_QUERY);
    sal_Bool bAddAllowed = !m_xTable.is();
    if(xColsSup.is())
        bAddAllowed = Reference<XAppend>(xColsSup->getColumns(),UNO_QUERY).is();

    try
    {
        Reference< XDatabaseMetaData > xMetaData = getMetaData( );
        bAddAllowed = bAddAllowed || ( xMetaData.is() && xMetaData->supportsAlterTableWithAddColumn());
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
        bAddAllowed = sal_False;
    }

    return bAddAllowed;
}

sal_Bool OTableController::isDropAllowed() const
{
    Reference<XColumnsSupplier> xColsSup(m_xTable,UNO_QUERY);
    sal_Bool bDropAllowed = !m_xTable.is();
    if(xColsSup.is())
    {
        Reference<XNameAccess> xNameAccess = xColsSup->getColumns();
        bDropAllowed = Reference<XDrop>(xNameAccess,UNO_QUERY).is() && xNameAccess->hasElements();
    }

    Reference< XDatabaseMetaData> xMetaData = getMetaData( );
    bDropAllowed = bDropAllowed || ( xMetaData.is() && xMetaData->supportsAlterTableWithDropColumn());

    return bDropAllowed;
}

sal_Bool OTableController::isAlterAllowed() const
{
    sal_Bool bAllowed(!m_xTable.is() || Reference<XAlterTable>(m_xTable,UNO_QUERY).is());
    return bAllowed;
}

void OTableController::reSyncRows()
{
    sal_Bool bAlterAllowed  = isAlterAllowed();
    sal_Bool bAddAllowed    = isAddAllowed();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aIter = m_vRowList.begin();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aEnd = m_vRowList.end();
    for(;aIter != aEnd;++aIter)
    {
        OSL_ENSURE(*aIter,"OTableRow is null!");
        OFieldDescription* pField = (*aIter)->GetActFieldDescr();
        if ( pField )
            (*aIter)->SetReadOnly(!bAlterAllowed);
        else
            (*aIter)->SetReadOnly(!bAddAllowed);

    }
    static_cast<OTableDesignView*>(getView())->reSync();    

    ClearUndoManager();
    setModified(sal_False);     
}

OUString OTableController::createUniqueName(const OUString& _rName)
{
    OUString sName = _rName;
    Reference< XDatabaseMetaData> xMetaData = getMetaData( );

    ::comphelper::UStringMixEqual bCase(xMetaData.is() ? xMetaData->supportsMixedCaseQuotedIdentifiers() : sal_True);

    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = m_vRowList.begin();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aEnd = m_vRowList.end();
    for(sal_Int32 i=0;aIter != aEnd;++aIter)
    {
        OFieldDescription* pFieldDesc = (*aIter)->GetActFieldDescr();
        if (pFieldDesc && !pFieldDesc->GetName().isEmpty() && bCase(sName,pFieldDesc->GetName()))
        { 
            sName = _rName + OUString::number(++i);
            aIter = m_vRowList.begin(); 
        }
    }
    return sName;
}

OUString OTableController::getPrivateTitle() const
{
    OUString sTitle;
    try
    {
        
        if ( !m_sName.isEmpty() && getConnection().is() )
        {
            if ( m_xTable.is() )
                sTitle = ::dbtools::composeTableName( getConnection()->getMetaData(), m_xTable, ::dbtools::eInDataManipulation, false, false, false );
            else
                sTitle = m_sName;
        }
        if ( sTitle.isEmpty() )
        {
            OUString aName = ModuleRes(STR_TBL_TITLE);
            sTitle = aName.getToken(0,' ');
            sTitle += OUString::number(getCurrentStartNumber());
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return sTitle;
}

void OTableController::reload()
{
    loadData();                 
    static_cast<OTableDesignView*>(getView())->reSync();    
    ClearUndoManager();
    setModified(sal_False);     
    static_cast<OTableDesignView*>(getView())->Invalidate();
}

sal_Int32 OTableController::getFirstEmptyRowPosition()
{
    sal_Int32 nRet = -1;
    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = m_vRowList.begin();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aEnd = m_vRowList.end();
    for(;aIter != aEnd;++aIter)
    {
        if ( !*aIter || !(*aIter)->GetActFieldDescr() || (*aIter)->GetActFieldDescr()->GetName().isEmpty() )
        {
            nRet = aIter - m_vRowList.begin();
            break;
        }
    }
    if ( nRet == -1 )
    {
        bool bReadRow = !isAddAllowed();
        ::boost::shared_ptr<OTableRow> pTabEdRow(new OTableRow());
        pTabEdRow->SetReadOnly(bReadRow);
        nRet = m_vRowList.size();
        m_vRowList.push_back( pTabEdRow);
    }
    return nRet;
}

bool OTableController::isAutoIncrementPrimaryKey() const
{
    return getSdbMetaData().isAutoIncrementPrimaryKey();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
