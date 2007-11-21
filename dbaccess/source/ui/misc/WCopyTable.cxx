/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WCopyTable.cxx,v $
 *
 *  $Revision: 1.54 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:07:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif
#ifndef DBAUI_WIZARD_PAGES_HRC
#include "WizardPages.hrc"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_WIZARD_CPAGE_HXX
#include "WCPage.hxx"
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef DBAUI_WIZ_TYPESELECT_HXX
#include "WTypeSelect.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#include <functional>

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;


#define MAX_PAGES   4   // max. Pages die angezeigt werden

DBG_NAME(OCopyTableWizard)
namespace
{
    void clearColumns(ODatabaseExport::TColumns& _rColumns,ODatabaseExport::TColumnVector& _rColumnsVec)
    {
        ODatabaseExport::TColumns::iterator aIter = _rColumns.begin();
        ODatabaseExport::TColumns::iterator aEnd  = _rColumns.end();

        for(;aIter != aEnd;++aIter)
            delete aIter->second;

        _rColumnsVec.clear();
        _rColumns.clear();
    }
}
//------------------------------------------------------------------------
OCopyTableWizard::OCopyTableWizard(Window * pParent,
                                   const Reference< XPropertySet >& _xSourceObject,
                                   const Reference< XConnection >& _xSourceConnection,
                                   const Reference< XConnection >& _xConnection,
                                   const Reference< XNumberFormatter >& _xFormatter,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    : WizardDialog( pParent, ModuleRes(WIZ_RTFCOPYTABLE))
    ,m_pbHelp( this , ModuleRes(PB_HELP))
    ,m_pbCancel( this , ModuleRes(PB_CANCEL))
    ,m_pbPrev( this , ModuleRes(PB_PREV))
    ,m_pbNext( this , ModuleRes(PB_NEXT))
    ,m_pbFinish( this , ModuleRes(PB_OK))
    ,m_mNameMapping(_xConnection->getMetaData().is() && _xConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers())
    ,m_xConnection(_xConnection)
    ,m_xSourceObject(_xSourceObject)
    ,m_xSourceConnection(_xSourceConnection)
    ,m_xFormatter(_xFormatter)
    ,m_xFactory(_rM)
    ,m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_nPageCount(0)
    ,m_bDeleteSourceColumns(sal_True)
    ,m_eCreateStyle(WIZARD_DEF_DATA)
    ,m_ePressed( WIZARD_NONE )
    ,m_bCreatePrimaryColumn(false)
{
    DBG_CTOR(OCopyTableWizard,NULL);
    construct();
    // Tabellennamen extrahieren
    try
    {
        if ( m_xSourceObject.is() )
        {
            Reference<XColumnsSupplier> xColSupp(m_xSourceObject,UNO_QUERY);
            if(xColSupp.is())
                m_xSourceColumns = xColSupp->getColumns();

            if ( !m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_COMMAND) )
                m_sSourceName = ::dbtools::composeTableName( m_xConnection->getMetaData(), m_xSourceObject, ::dbtools::eInDataManipulation, false, false, false );
            else
                _xSourceObject->getPropertyValue(PROPERTY_NAME) >>= m_sSourceName;
        }

        if ( m_xSourceConnection == m_xConnection )
        {
            Reference<XTablesSupplier> xSup(m_xConnection,UNO_QUERY_THROW);
            m_sName = ::dbtools::createUniqueName(xSup->getTables(),m_sSourceName,sal_False);
        }
        else
            m_sName = m_sSourceName;
    }
    catch(const Exception&)
    {
        m_sName = m_sSourceName;
    }

}
// -----------------------------------------------------------------------------
OCopyTableWizard::OCopyTableWizard(Window * pParent,
                                   const ::rtl::OUString& _rDefaultName,
                                   const ODatabaseExport::TColumns& _rSourceColumns,
                                   const ODatabaseExport::TColumnVector& _rSourceColVec,
                                   const Reference< XConnection >& _xConnection,
                                   const Reference< XNumberFormatter >& _xFormatter,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    :WizardDialog( pParent, ModuleRes(WIZ_RTFCOPYTABLE))
    ,m_vSourceColumns(_rSourceColumns)
    ,m_pbHelp( this , ModuleRes(PB_HELP))
    ,m_pbCancel( this , ModuleRes(PB_CANCEL))
    ,m_pbPrev( this , ModuleRes(PB_PREV))
    ,m_pbNext( this , ModuleRes(PB_NEXT))
    ,m_pbFinish( this , ModuleRes(PB_OK))
    ,m_mNameMapping(_xConnection->getMetaData().is() && _xConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers())
    ,m_xConnection(_xConnection)
    ,m_xSourceConnection(_xConnection) // in this case source connection and dest connection are the same
    ,m_xFormatter(_xFormatter)
    ,m_xFactory(_rM)
    ,m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_nPageCount(0)
    ,m_bDeleteSourceColumns(sal_False)
    ,m_sName(_rDefaultName)
    ,m_eCreateStyle(WIZARD_DEF_DATA)
    ,m_ePressed( WIZARD_NONE )
    ,m_bCreatePrimaryColumn(false)
{
    DBG_CTOR(OCopyTableWizard,NULL);
    construct();
    ODatabaseExport::TColumnVector::const_iterator aIter = _rSourceColVec.begin();
    ODatabaseExport::TColumnVector::const_iterator aEnd = _rSourceColVec.end();
    for (; aIter != aEnd ; ++aIter)
    {
        m_vSourceVec.push_back(m_vSourceColumns.find((*aIter)->first));
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::construct()
{
    AddButton( &m_pbHelp, WIZARDDIALOG_BUTTON_STDOFFSET_X );
    AddButton( &m_pbCancel, WIZARDDIALOG_BUTTON_STDOFFSET_X );
    AddButton( &m_pbPrev );
    AddButton( &m_pbNext, WIZARDDIALOG_BUTTON_STDOFFSET_X );
    AddButton( &m_pbFinish );

    m_pbPrev.SetClickHdl( LINK( this, OCopyTableWizard, ImplPrevHdl ) );
    m_pbNext.SetClickHdl( LINK( this, OCopyTableWizard, ImplNextHdl ) );
    m_pbFinish.SetClickHdl( LINK( this, OCopyTableWizard, ImplOKHdl ) );

    SetActivatePageHdl( LINK( this, OCopyTableWizard, ImplActivateHdl ) );

    SetPrevButton( &m_pbPrev );
    SetNextButton( &m_pbNext );

    ShowButtonFixedLine( sal_True );

    m_pbNext.GrabFocus();

    if (m_vDestColumns.size())
        // source is a html or rtf table
        m_pbNext.SetStyle(m_pbFinish.GetStyle() | WB_DEFBUTTON);
    else
        m_pbFinish.SetStyle(m_pbFinish.GetStyle() | WB_DEFBUTTON);

    FreeResource();

    m_pTypeInfo = TOTypeInfoSP(new OTypeInfo());
    m_pTypeInfo->aUIName = m_sTypeNames.GetToken(TYPE_OTHER);
    m_bAddPKFirstTime = sal_True;
}
//------------------------------------------------------------------------
OCopyTableWizard::~OCopyTableWizard()
{
    DBG_DTOR(OCopyTableWizard,NULL);
    for ( ;; )
    {
        TabPage *pPage = GetPage(0);
        if ( pPage == NULL )
            break;
        RemovePage( pPage );
        delete pPage;
    }

    if ( m_bDeleteSourceColumns )
        clearColumns(m_vSourceColumns,m_vSourceVec);

    clearColumns(m_vDestColumns,m_aDestVec);

    // clear the type information
    m_aTypeInfoIndex.clear();
    m_aTypeInfo.clear();
    m_aDestTypeInfoIndex.clear();
}
// -----------------------------------------------------------------------
IMPL_LINK( OCopyTableWizard, ImplPrevHdl, PushButton*, EMPTYARG )
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    m_ePressed = WIZARD_PREV;
    if ( GetCurLevel() )
    {
        if(getCreateStyle() != WIZARD_APPEND_DATA)
        {
            if(GetCurLevel() == 2)
                ShowPage(GetCurLevel()-2);
            else
                ShowPrevPage();
        }
        else
            ShowPrevPage();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( OCopyTableWizard, ImplNextHdl, PushButton*, EMPTYARG )
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    m_ePressed = WIZARD_NEXT;
    if ( GetCurLevel() < MAX_PAGES )
    {
        if(getCreateStyle() != WIZARD_APPEND_DATA)
        {
            if(GetCurLevel() == 0)
                ShowPage(GetCurLevel()+2);
            else
                ShowNextPage();
        }
        else
            ShowNextPage();
    }
    return 0;
}
// -----------------------------------------------------------------------
sal_Bool OCopyTableWizard::CheckColumns(sal_Int32& _rnBreakPos)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    sal_Bool bRet = sal_True;
    m_vColumnPos.clear();
    m_vColumnTypes.clear();

    OSL_ENSURE(m_xConnection.is(),"OCopyTableWizard::CheckColumns: No connection!");
    //////////////////////////////////////////////////////////////////////
    // Wenn Datenbank PrimaryKeys verarbeiten kann, PrimaryKey anlegen
    if ( m_xConnection.is() )
    {
        Reference< XDatabaseMetaData >  xMetaData(m_xConnection->getMetaData());
        sal_Bool bPKeyAllowed = xMetaData->supportsCoreSQLGrammar();

        sal_Bool bContainsColumns = !m_vDestColumns.empty();

        if ( bPKeyAllowed && isAutoincrementEnabled() )
        {
            // add extra column for the primary key
            TOTypeInfoSP pTypeInfo = queryPrimaryKeyType(m_aDestTypeInfo);
            if ( pTypeInfo.get() )
            {
                if ( m_bAddPKFirstTime )
                {
                    OFieldDescription* pField = new OFieldDescription();
                    pField->SetName(m_aKeyName);
                    pField->FillFromTypeInfo(pTypeInfo,sal_True,sal_True);
                    pField->SetPrimaryKey(sal_True);
                    m_bAddPKFirstTime = sal_False;
                    insertColumn(0,pField);
                }
                m_vColumnPos.push_back(ODatabaseExport::TPositions::value_type(1,1));
                m_vColumnTypes.push_back(pTypeInfo->nType);
            }
        }

        if ( bContainsColumns )
        {   // we have dest columns so look for the matching column
            ODatabaseExport::TColumnVector::const_iterator aSrcIter = m_vSourceVec.begin();
            for(;aSrcIter != m_vSourceVec.end();++aSrcIter)
            {
                ODatabaseExport::TColumns::iterator aDestIter = m_vDestColumns.find(m_mNameMapping[(*aSrcIter)->first]);

                if ( aDestIter != m_vDestColumns.end() )
                {
                    ODatabaseExport::TColumnVector::const_iterator aFind = ::std::find(m_aDestVec.begin(),m_aDestVec.end(),aDestIter);
                    sal_Int32 nPos = (aFind - m_aDestVec.begin())+1;
                    m_vColumnPos.push_back(ODatabaseExport::TPositions::value_type(nPos,nPos));
                    m_vColumnTypes.push_back((*aFind)->second->GetType());
                }
                else
                {
                    m_vColumnPos.push_back( ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );
                    m_vColumnTypes.push_back(0);
                }
            }
        }
        else
        {
            ::rtl::OUString sExtraChars = xMetaData->getExtraNameCharacters();
            sal_Int32 nMaxNameLen       = getMaxColumnNameLength();

            ODatabaseExport::TColumnVector::const_iterator aSrcIter = m_vSourceVec.begin();
            for(_rnBreakPos=0;aSrcIter != m_vSourceVec.end() && bRet ;++aSrcIter,++_rnBreakPos)
            {
                OFieldDescription* pField = new OFieldDescription(*(*aSrcIter)->second);
                pField->SetName(convertColumnName(TExportColumnFindFunctor(&m_vDestColumns),(*aSrcIter)->first,sExtraChars,nMaxNameLen));
                TOTypeInfoSP pType = convertType((*aSrcIter)->second->getTypeInfo(),bRet);
                pField->SetType(pType);
                if ( !bPKeyAllowed )
                    pField->SetPrimaryKey(sal_False);

                // now create a column
                insertColumn(m_vDestColumns.size(),pField);
                m_vColumnPos.push_back(ODatabaseExport::TPositions::value_type(m_vDestColumns.size(),m_vDestColumns.size()));
                m_vColumnTypes.push_back((*aSrcIter)->second->GetType());
            }
        }
    }
    return bRet;
}
// -----------------------------------------------------------------------
IMPL_LINK( OCopyTableWizard, ImplOKHdl, OKButton*, EMPTYARG )
{
    m_ePressed = WIZARD_FINISH;
    sal_Bool bFinish = DeactivatePage() != 0;

    if(bFinish)
    {
        WaitObject aWait(this);
        switch(getCreateStyle())
        {
            case WIZARD_DEF_DATA:
            case WIZARD_DEF:
            {
                sal_Bool bOnFirstPage = GetCurLevel() == 0;
                if ( bOnFirstPage )
                {
                    // we came from the first page so we have to clear
                    // all column information already collected
                    clearDestColumns();
                    m_mNameMapping.clear();
                }
                sal_Int32 nBreakPos = 0;
                sal_Bool bCheckOk = CheckColumns(nBreakPos);
                if ( bOnFirstPage && !bCheckOk )
                {
                    showColumnTypeNotSupported(m_vSourceVec[nBreakPos-1]->first);
                    OWizTypeSelect* pPage = static_cast<OWizTypeSelect*>(GetPage(3));
                    if ( pPage )
                    {
                        pPage->setDisplayRow(nBreakPos);
                        ShowPage(3);
                        return 0;
                    }
                }
                if ( m_xConnection.is() )
                {
                    sal_Bool bPKeyAllowed = sal_False;
                    try
                    {
                        Reference< XDatabaseMetaData >  xMetaData(m_xConnection->getMetaData());
                        bPKeyAllowed = xMetaData->supportsCoreSQLGrammar();
                    }
                    catch(const Exception&)
                    {
                        OSL_ENSURE(0,"Exception caught while asking for supportsCoreSQLGrammar!");
                    }
                    if ( bPKeyAllowed )
                    {
                        ODatabaseExport::TColumns::iterator aFind = ::std::find_if(m_vDestColumns.begin(),m_vDestColumns.end()
                            ,::std::compose1(::std::mem_fun(&OFieldDescription::IsPrimaryKey),::std::select2nd<ODatabaseExport::TColumns::value_type>()));
                        if ( aFind == m_vDestColumns.end() )
                        {
                            String sTitle(ModuleRes(STR_TABLEDESIGN_NO_PRIM_KEY_HEAD));
                            String sMsg(ModuleRes(STR_TABLEDESIGN_NO_PRIM_KEY));
                            OSQLMessageBox aBox(this, sTitle,sMsg, WB_YES_NO_CANCEL | WB_DEF_YES);

                            INT16 nReturn = aBox.Execute();

                            switch(nReturn )
                            {
                                case RET_YES:
                                {
                                    OCopyTable* pPage = static_cast<OCopyTable*>(GetPage(0));
                                    m_bCreatePrimaryColumn = true;
                                    m_aKeyName = pPage->GetKeyName();
                                    sal_Int32 nBreakPos2 = 0;
                                    CheckColumns(nBreakPos2);
                                    break;
                                }
                                case RET_CANCEL:
                                    ShowPage(3);
                                    return 0;
                                default:
                                    ;
                            }
                        }
                    }
                }
                break;
            }
            case WIZARD_APPEND_DATA:
            case WIZARD_DEF_VIEW:
                break;
            default:
            {
                OSL_ENSURE(sal_False, "OCopyTableWizard::ImplOKHdl: invalid creation style!");
            }
        }

        EndDialog(RET_OK);
    }
    return bFinish;
}
//------------------------------------------------------------------------
bool OCopyTableWizard::isAutoincrementEnabled() const
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    return m_bCreatePrimaryColumn;
}
// -----------------------------------------------------------------------

IMPL_LINK( OCopyTableWizard, ImplActivateHdl, WizardDialog*, EMPTYARG )
{
    OWizardPage* pCurrent = (OWizardPage*)GetPage(GetCurLevel());
    if(pCurrent)
    {
        sal_Bool bFirstTime = pCurrent->IsFirstTime();
        if(bFirstTime)
            pCurrent->Reset();

        CheckButtons();

        SetText(pCurrent->GetTitle());

        Invalidate();
    }
    return 0;
}
// -----------------------------------------------------------------------
void OCopyTableWizard::CheckButtons()
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    if(GetCurLevel() == 0) // erste Seite hat kein PrevButton
    {
        if(m_nPageCount > 1)
            m_pbNext.Enable(sal_True);
        else
            m_pbNext.Enable(sal_False);

        m_pbPrev.Enable(sal_False);
    }
    else if(GetCurLevel() == m_nPageCount-1) // letzte Seite hat keinen Next Button
    {
        m_pbNext.Enable(sal_False);
        m_pbPrev.Enable(sal_True);
    }
    else
    {
        m_pbPrev.Enable(sal_True);
        // next has already his state
    }
}
// -----------------------------------------------------------------------
void OCopyTableWizard::EnableButton(Wizard_Button_Style eStyle,sal_Bool bEnable)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
//  CheckButtons();
    Button* pButton;
    if(eStyle == WIZARD_NEXT)
        pButton = &m_pbNext;
    else if(eStyle == WIZARD_PREV)
        pButton = &m_pbPrev;
    else
        pButton = &m_pbFinish;
    pButton->Enable(bEnable);

}
// -----------------------------------------------------------------------
long OCopyTableWizard::DeactivatePage()
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    OWizardPage* pPage = (OWizardPage*)GetPage(GetCurLevel());
    return pPage ? pPage->LeavePage() : sal_False;
}
// -----------------------------------------------------------------------
void OCopyTableWizard::AddWizardPage(OWizardPage* pPage)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    AddPage(pPage);
    ++m_nPageCount;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::insertColumn(sal_Int32 _nPos,OFieldDescription* _pField)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    OSL_ENSURE(_pField,"FieldDescrioption is null!");
    if ( _pField )
    {
        ODatabaseExport::TColumns::iterator aFind = m_vDestColumns.find(_pField->GetName());
        if ( aFind != m_vDestColumns.end() )
        {
            delete aFind->second;
            m_vDestColumns.erase(aFind);
        }

        m_aDestVec.insert(m_aDestVec.begin() + _nPos,
            m_vDestColumns.insert(ODatabaseExport::TColumns::value_type(_pField->GetName(),_pField)).first);
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::replaceColumn(sal_Int32 _nPos,OFieldDescription* _pField,const ::rtl::OUString& _sOldName)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    OSL_ENSURE(_pField,"FieldDescrioption is null!");
    if ( _pField )
    {
        m_vDestColumns.erase(_sOldName);
        OSL_ENSURE( m_vDestColumns.find(_pField->GetName()) == m_vDestColumns.end(),"Column with that name already exist!");

        m_aDestVec[_nPos] =
            m_vDestColumns.insert(ODatabaseExport::TColumns::value_type(_pField->GetName(),_pField)).first;
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::loadData(const Reference<XPropertySet>& _xTable,
                                ODatabaseExport::TColumns& _rColumns,
                                ODatabaseExport::TColumnVector& _rColVector)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    ODatabaseExport::TColumns::iterator aIter = _rColumns.begin();

    for(;aIter != _rColumns.end();++aIter)
        delete aIter->second;

    _rColVector.clear();
    _rColumns.clear();

    OSL_ENSURE(m_xConnection.is(),"OCopyTableWizard::CheckColumns: No connection!");
    if(m_xConnection.is())
    {
        //////////////////////////////////////////////////////////////////////
        // Datenstruktur mit Daten aus DatenDefinitionsObjekt fuellen
        if(_xTable.is())
        {
            Reference<XColumnsSupplier> xColSup(_xTable,UNO_QUERY);
            OSL_ENSURE(xColSup.is(),"No XColumnsSupplier!");
            Reference<XNameAccess> xColumns = xColSup->getColumns();
            OFieldDescription* pActFieldDescr = NULL;
            String aType;
            ::rtl::OUString sCreateParam(RTL_CONSTASCII_USTRINGPARAM("x"));
            //////////////////////////////////////////////////////////////////////
            // ReadOnly-Flag
            // Bei Drop darf keine Zeile editierbar sein.
            // Bei Add duerfen nur die leeren Zeilen editierbar sein.
            // Bei Add und Drop koennen alle Zeilen editiert werden.
            Sequence< ::rtl::OUString> aColumns = xColumns->getElementNames();
            const ::rtl::OUString* pIter    = aColumns.getConstArray();
            const ::rtl::OUString* pEnd     = pIter + aColumns.getLength();

            for(;pIter != pEnd;++pIter)
            {
                Reference<XPropertySet> xColumn;
                xColumns->getByName(*pIter) >>= xColumn;

                sal_Int32 nType         = 0;
                sal_Int32 nScale        = 0;
                sal_Int32 nPrecision    = 0;
                sal_Bool bAutoIncrement = sal_False;
                ::rtl::OUString sTypeName;

                // get the properties from the column
                xColumn->getPropertyValue(PROPERTY_TYPENAME)        >>= sTypeName;
                xColumn->getPropertyValue(PROPERTY_TYPE)            >>= nType;
                xColumn->getPropertyValue(PROPERTY_SCALE)           >>= nScale;
                xColumn->getPropertyValue(PROPERTY_PRECISION)       >>= nPrecision;
                xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT) >>= bAutoIncrement;

                pActFieldDescr = new OFieldDescription(xColumn);
                // search for type
                sal_Bool bForce;
                TOTypeInfoSP pTypeInfo = ::dbaui::getTypeInfoFromType(m_aTypeInfo,nType,sTypeName,sCreateParam,nPrecision,nScale,bAutoIncrement,bForce);
                if ( !pTypeInfo.get() )
                    pTypeInfo = m_pTypeInfo;

                pActFieldDescr->FillFromTypeInfo(pTypeInfo,sal_True,sal_False);
                _rColVector.push_back(_rColumns.insert(ODatabaseExport::TColumns::value_type(pActFieldDescr->GetName(),pActFieldDescr)).first);
            }
            // fill the primary  key information
            Reference<XNameAccess> xKeyColumns  = getKeyColumns(_xTable);
            if(xKeyColumns.is())
            {
                Sequence< ::rtl::OUString> aKeyColumns = xKeyColumns->getElementNames();
                const ::rtl::OUString* pKeyBegin    = aKeyColumns.getConstArray();
                const ::rtl::OUString* pKeyEnd      = pKeyBegin + aKeyColumns.getLength();

                for(;pKeyBegin != pKeyEnd;++pKeyBegin)
                {
                    ODatabaseExport::TColumns::iterator keyPos = _rColumns.find(*pKeyBegin);
                    if ( keyPos != _rColumns.end() )
                    {
                        keyPos->second->SetPrimaryKey(sal_True);
                        keyPos->second->SetIsNullable(ColumnValue::NO_NULLS);
                    }
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::clearDestColumns()
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    clearColumns(m_vDestColumns,m_aDestVec);
}
// -----------------------------------------------------------------------------
Reference<XNameAccess> OCopyTableWizard::getKeyColumns(const Reference<XPropertySet>& _xTable) const
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    // use keys and indexes for excat postioning
    // first the keys
    Reference<XKeysSupplier> xKeySup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xKeys;
    if(xKeySup.is())
        xKeys = xKeySup->getKeys();

    Reference<XColumnsSupplier> xKeyColsSup;
    Reference<XNameAccess> xKeyColumns;
    if(xKeys.is())
    {
        Reference<XPropertySet> xProp;
        for(sal_Int32 i=0;i< xKeys->getCount();++i)
        {
            xKeys->getByIndex(i) >>= xProp;
            sal_Int32 nKeyType = 0;
            xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
            if(KeyType::PRIMARY == nKeyType)
            {
                xKeyColsSup.set(xProp,UNO_QUERY);
                OSL_ENSURE(xKeyColsSup.is(),"Columnsupplier is null!");
                xKeyColumns = xKeyColsSup->getColumns();
                break;
            }
        }
    }

    return xKeyColumns;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::appendColumns(Reference<XColumnsSupplier>& _rxColSup,const ODatabaseExport::TColumnVector* _pVec,sal_Bool _bKeyColumns)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    // now append the columns
    OSL_ENSURE(_rxColSup.is(),"No columns supplier");
    if(!_rxColSup.is())
        return;
    Reference<XNameAccess> xColumns = _rxColSup->getColumns();
    OSL_ENSURE(xColumns.is(),"No columns");
    Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);

    Reference<XAppend> xAppend(xColumns,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    ODatabaseExport::TColumnVector::const_iterator aIter = _pVec->begin();
    for(;aIter != _pVec->end();++aIter)
    {
        OFieldDescription* pField = (*aIter)->second;
        if(!pField)
            continue;

        Reference<XPropertySet> xColumn;
        if(pField->IsPrimaryKey() || !_bKeyColumns)
            xColumn = xColumnFactory->createDataDescriptor();
        if(xColumn.is())
        {
            if(!_bKeyColumns)
                dbaui::setColumnProperties(xColumn,pField);
            else
                xColumn->setPropertyValue(PROPERTY_NAME,makeAny(pField->GetName()));

            xAppend->appendByDescriptor(xColumn);
            xColumn = NULL;
            // now only the settings are missing
            if(xColumns->hasByName(pField->GetName()))
            {
                xColumn.set(xColumns->getByName(pField->GetName()),UNO_QUERY);
                OSL_ENSURE(xColumn.is(),"OCopyTableWizard::appendColumns: Column is NULL!");
                if ( xColumn.is() )
                    pField->copyColumnSettingsTo(xColumn);
            }
            else
            {
                OSL_ENSURE(sal_False, "OCopyTableWizard::appendColumns: invalid field name!");
            }

        }
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::appendKey(Reference<XKeysSupplier>& _rxSup,const ODatabaseExport::TColumnVector* _pVec)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    if(!_rxSup.is())
        return; // the database doesn't support keys
    OSL_ENSURE(_rxSup.is(),"No XKeysSupplier!");
    Reference<XDataDescriptorFactory> xKeyFactory(_rxSup->getKeys(),UNO_QUERY);
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
        appendColumns(xColSup,_pVec,sal_True);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        if(xColumns.is() && xColumns->getElementNames().getLength())
            xAppend->appendByDescriptor(xKey);
    }

}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OCopyTableWizard::createView()
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    m_xDestObject = ::dbaui::createView(m_sName,m_xConnection,m_xSourceObject);
    return m_xDestObject;
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OCopyTableWizard::createTable()
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    Reference<XTablesSupplier> xSup(m_xConnection,UNO_QUERY);
    Reference< XNameAccess > xTables;
    if(xSup.is())
        xTables = xSup->getTables();
    if(getCreateStyle() != WIZARD_APPEND_DATA)
    {
        Reference<XDataDescriptorFactory> xFact(xTables,UNO_QUERY);
        OSL_ENSURE(xFact.is(),"No XDataDescriptorFactory available!");
        if(!xFact.is())
            return NULL;

        m_xDestObject = xFact->createDataDescriptor();
        OSL_ENSURE(m_xDestObject.is(),"Could not create a new object!");
        if(!m_xDestObject.is())
            return NULL;

        ::rtl::OUString sCatalog,sSchema,sTable;
        Reference< XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
        ::dbtools::qualifiedNameComponents(xMetaData,
                                            m_sName,
                                            sCatalog,
                                            sSchema,
                                            sTable,
                                            ::dbtools::eInDataManipulation);

        if ( !sCatalog.getLength() && xMetaData->supportsCatalogsInTableDefinitions() )
        {
            sCatalog = m_xConnection->getCatalog();
        }

        if ( !sSchema.getLength() && xMetaData->supportsSchemasInTableDefinitions() )
        {
            sSchema = xMetaData->getUserName();
        }

        m_xDestObject->setPropertyValue(PROPERTY_CATALOGNAME,makeAny(sCatalog));
        m_xDestObject->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(sSchema));
        m_xDestObject->setPropertyValue(PROPERTY_NAME,makeAny(sTable));

        if(m_xSourceObject.is()) // can be null when importing data from html or rtf format
        {
            if ( m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_FONT) )
                m_xDestObject->setPropertyValue(PROPERTY_FONT,m_xSourceObject->getPropertyValue(PROPERTY_FONT));
            if ( m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_ROW_HEIGHT) )
                m_xDestObject->setPropertyValue(PROPERTY_ROW_HEIGHT,m_xSourceObject->getPropertyValue(PROPERTY_ROW_HEIGHT));
            if ( m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_TEXTCOLOR) )
                m_xDestObject->setPropertyValue(PROPERTY_TEXTCOLOR,m_xSourceObject->getPropertyValue(PROPERTY_TEXTCOLOR));
            // can not be copied yet, because the filter or and order clause could the old table name
        }

        Reference< XColumnsSupplier > xSuppDestinationColumns( m_xDestObject, UNO_QUERY );
        // now append the columns
        const ODatabaseExport::TColumnVector* pVec = getDestVector();
        appendColumns( xSuppDestinationColumns, pVec );
        // now append the primary key
        Reference<XKeysSupplier> xKeySup(m_xDestObject,UNO_QUERY);
        appendKey(xKeySup,pVec);

        Reference<XAppend> xAppend(xTables,UNO_QUERY);
        if(xAppend.is())
            xAppend->appendByDescriptor(m_xDestObject);

        //  m_xDestObject = NULL;
        // we need to reget the table because after appending it it is no longer valid
        if(xTables->hasByName(m_sName))
            xTables->getByName(m_sName) >>= m_xDestObject;
        else
        {
            ::rtl::OUString sComposedName(
                ::dbtools::composeTableName( m_xConnection->getMetaData(), m_xDestObject, ::dbtools::eInDataManipulation, false, false, false ) );
            if(xTables->hasByName(sComposedName))
            {
                xTables->getByName(sComposedName) >>= m_xDestObject;
                m_sName = sComposedName;
            }
            else
                m_xDestObject = NULL;
        }
        if(m_xDestObject.is())
        {
            xSuppDestinationColumns.set( m_xDestObject, UNO_QUERY_THROW );
            // insert new table name into table filter
            ::dbaui::appendToFilter(m_xConnection,m_sName,GetFactory(),this);
            // set column mappings
            Reference<XNameAccess> xNameAccess = xSuppDestinationColumns->getColumns();
            Sequence< ::rtl::OUString> aSeq = xNameAccess->getElementNames();
            const ::rtl::OUString* pIter = aSeq.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();

            ::std::vector<int> aAlreadyFound(m_vColumnPos.size(),0);

            for(sal_Int32 nNewPos=1;pIter != pEnd;++pIter,++nNewPos)
            {
                ODatabaseExport::TColumns::iterator aDestIter = m_vDestColumns.find(*pIter);

                if ( aDestIter != m_vDestColumns.end() )
                {
                    ODatabaseExport::TColumnVector::const_iterator aFind = ::std::find(m_aDestVec.begin(),m_aDestVec.end(),aDestIter);
                    sal_Int32 nPos = (aFind - m_aDestVec.begin())+1;

                    ODatabaseExport::TPositions::iterator aPosFind = ::std::find_if(m_vColumnPos.begin(),m_vColumnPos.end(),
                        ::std::compose1(::std::bind2nd(::std::equal_to<sal_Int32>(),nPos),::std::select1st<ODatabaseExport::TPositions::value_type>()));

                    if ( m_vColumnPos.end() != aPosFind )
                    {
                        aPosFind->second = nNewPos;
                        OSL_ENSURE(static_cast<sal_Int32>(m_vColumnTypes.size()) > (aPosFind - m_vColumnPos.begin()),"Invalid index for vector!");
                        m_vColumnTypes[aPosFind - m_vColumnPos.begin()] = (*aFind)->second->GetType();
                    }
                }
/*
                ODatabaseExport::TColumnVector::const_iterator aIter = pVec->begin();
                for(sal_Int32 nOldPos = 1;aIter != pVec->end();++aIter,++nOldPos)
                {
                    if((*aIter)->first == *pBegin)
                    {
                        if(nOldPos != nNewPos)
                        {
                            ::std::vector<int>::iterator aFound = aAlreadyFound.begin();
                            ODatabaseExport::TPositions::iterator aColPos = m_vColumnPos.begin();
                            for(; aColPos != m_vColumnPos.end() && nOldPos;++aColPos,++aFound)
                            {
                                if(aColPos->second != COLUMN_POSITION_NOT_FOUND && !*aFound && nOldPos == aColPos->second)
                                    break;
                            }
                            if(aColPos != m_vColumnPos.end())
                            {
                                *aFound = 1;
                                aColPos->second = nNewPos;
                                m_vColumnTypes[m_vColumnPos.end() - aColPos] = (*aIter)->second->GetType();
                            }
                        }
                        break;
                    }
                }
*/
            }
        }
    }
    else if(xTables.is() && xTables->hasByName(m_sName))
        xTables->getByName(m_sName) >>= m_xDestObject;

    return m_xDestObject;
}
// -----------------------------------------------------------------------------
sal_Bool OCopyTableWizard::supportsPrimaryKey() const
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    sal_Bool bAllowed = sal_False;
    if(m_xConnection.is())
    {
        try
        {
            Reference< XDatabaseMetaData >  xMetaData(m_xConnection->getMetaData());
            bAllowed = xMetaData.is() && xMetaData->supportsCoreSQLGrammar();
        }
        catch(const Exception&)
        {
        }
    }
    return bAllowed;
}
// -----------------------------------------------------------------------------
sal_Int32 OCopyTableWizard::getMaxColumnNameLength() const
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    sal_Int32 nLen = 0;
    if(m_xConnection.is())
    {
        try
        {
            Reference< XDatabaseMetaData >  xMetaData(m_xConnection->getMetaData());
            nLen = xMetaData.is() ? xMetaData->getMaxColumnNameLength() : 0;
        }
        catch(const Exception&)
        {
        }
    }
    return nLen;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::setCreateStyle(const OCopyTableWizard::Wizard_Create_Style& _eStyle)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    m_eCreateStyle = _eStyle;
}
// -----------------------------------------------------------------------------
OCopyTableWizard::Wizard_Create_Style OCopyTableWizard::getCreateStyle() const
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    return m_eCreateStyle;
}
// -----------------------------------------------------------------------------
::rtl::OUString OCopyTableWizard::convertColumnName(const TColumnFindFunctor&   _rCmpFunctor,
                                                    const ::rtl::OUString&  _sColumnName,
                                                    const ::rtl::OUString&  _sExtraChars,
                                                    sal_Int32               _nMaxNameLen)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);

    ::rtl::OUString sAlias = _sColumnName;
    if ( isSQL92CheckEnabled(m_xConnection) )
        sAlias = ::dbtools::convertName2SQLName(_sColumnName,_sExtraChars);
    if((_nMaxNameLen && sAlias.getLength() > _nMaxNameLen) || _rCmpFunctor(sAlias))
    {
        sal_Int32 nDiff = 1;
        do
        {
            ++nDiff;
            if(_nMaxNameLen && sAlias.getLength() >= _nMaxNameLen)
                sAlias = sAlias.copy(0,sAlias.getLength() - (sAlias.getLength()-_nMaxNameLen+nDiff));

            ::rtl::OUString sName(sAlias);
            sal_Int32 nPos = 1;
            sName += ::rtl::OUString::valueOf(nPos);

            while(_rCmpFunctor(sName))
            {
                sName = sAlias;
                sName += ::rtl::OUString::valueOf(++nPos);
            }
            sAlias = sName;
            // we have to check again, it could happen that the name is already to long
        }
        while(_nMaxNameLen && sAlias.getLength() > _nMaxNameLen);
    }
    OSL_ENSURE(m_mNameMapping.find(_sColumnName) == m_mNameMapping.end(),"name doubled!");
    m_mNameMapping[_sColumnName] = sAlias;
    return sAlias;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::removeColumnNameFromNameMap(const ::rtl::OUString& _sName)
{
    m_mNameMapping.erase(_sName);
}
// -----------------------------------------------------------------------------
sal_Bool OCopyTableWizard::supportsType(sal_Int32 _nDataType,sal_Int32& _rNewDataType)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    sal_Bool bRet = m_aDestTypeInfo.find(_nDataType) != m_aDestTypeInfo.end();
    if ( bRet )
        _rNewDataType = _nDataType;
    return bRet;
}
// -----------------------------------------------------------------------------
TOTypeInfoSP OCopyTableWizard::convertType(const TOTypeInfoSP& _pType,sal_Bool& _bNotConvert)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    if ( m_xSourceConnection == m_xConnection )
        return _pType;

    sal_Bool bForce;
    TOTypeInfoSP pType = ::dbaui::getTypeInfoFromType(m_aDestTypeInfo,_pType->nType,_pType->aTypeName,_pType->aCreateParams,_pType->nPrecision,_pType->nMaximumScale,_pType->bAutoIncrement,bForce);
    if ( !pType.get() || bForce )
    { // no type found so we have to find the correct one ourself
        sal_Int32 nDefaultType = DataType::VARCHAR;
        switch(_pType->nType)
        {
            case DataType::TINYINT:
                if(supportsType(DataType::SMALLINT,nDefaultType))
                    break;
                // run through
            case DataType::SMALLINT:
                if(supportsType(DataType::INTEGER,nDefaultType))
                    break;
                // run through
            case DataType::INTEGER:
                if(supportsType(DataType::FLOAT,nDefaultType))
                    break;
                // run through
            case DataType::FLOAT:
                if(supportsType(DataType::REAL,nDefaultType))
                    break;
                // run through
            case DataType::DATE:
            case DataType::TIME:
                if( DataType::DATE == _pType->nType || DataType::TIME == _pType->nType )
                {
                    if(supportsType(DataType::TIMESTAMP,nDefaultType))
                        break;
                }
                // run through
            case DataType::TIMESTAMP:
            case DataType::REAL:
            case DataType::BIGINT:
                if ( supportsType(DataType::DOUBLE,nDefaultType) )
                    break;
                // run through
            case DataType::DOUBLE:
                if ( supportsType(DataType::NUMERIC,nDefaultType) )
                    break;
                // run through
            case DataType::NUMERIC:
                supportsType(DataType::DECIMAL,nDefaultType);
                break;
            case DataType::DECIMAL:
                if ( supportsType(DataType::NUMERIC,nDefaultType) )
                    break;
                if ( supportsType(DataType::DOUBLE,nDefaultType) )
                    break;
                break;
            case DataType::VARCHAR:
                if ( supportsType(DataType::LONGVARCHAR,nDefaultType) )
                    break;
                break;
            default:
                nDefaultType = DataType::VARCHAR;
        }
        pType = ::dbaui::getTypeInfoFromType(m_aDestTypeInfo,nDefaultType,_pType->aTypeName,_pType->aCreateParams,_pType->nPrecision,_pType->nMaximumScale,_pType->bAutoIncrement,bForce);
        if ( !pType.get() )
        {
            _bNotConvert = sal_False;
            ::rtl::OUString sCreate(RTL_CONSTASCII_USTRINGPARAM("x"));
            pType = ::dbaui::getTypeInfoFromType(m_aDestTypeInfo,DataType::VARCHAR,_pType->aTypeName,sCreate,50,0,sal_False,bForce);
            if ( !pType.get() )
                pType = m_pTypeInfo;
        }
        else if ( bForce )
            _bNotConvert = sal_False;
    }
    return pType;
}
// -----------------------------------------------------------------------------
::rtl::OUString OCopyTableWizard::createUniqueName(const ::rtl::OUString& _sName)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    ::rtl::OUString sName = _sName;
    if ( m_xSourceColumns.is() )
        sName = ::dbtools::createUniqueName(m_xSourceColumns,sName,sal_False);
    else
    {
        if ( m_vSourceColumns.find(sName) != m_vSourceColumns.end())
        {
            sal_Int32 nPos = 0;
            while(m_vSourceColumns.find(sName) != m_vSourceColumns.end())
            {
                sName = _sName;
                sName += ::rtl::OUString::valueOf(++nPos);
            }
        }
    }
    return sName;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::fillTypeInfo()
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    ::dbaui::fillTypeInfo(m_xSourceConnection,m_sTypeNames,m_aTypeInfo,m_aTypeInfoIndex);
    ::dbaui::fillTypeInfo(m_xConnection,m_sTypeNames,m_aDestTypeInfo,m_aDestTypeInfoIndex);
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::loadData()
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    loadData(m_xSourceObject,m_vSourceColumns,m_vSourceVec); // create the field description
}
// -----------------------------------------------------------------------------
//=======
void OCopyTableWizard::showColumnTypeNotSupported(const ::rtl::OUString& _rColumnName)
{
    DBG_CHKTHIS(OCopyTableWizard,NULL);
    UniString sTitle(ModuleRes(STR_STAT_WARNING));
    UniString sMessage(ModuleRes(STR_UNKNOWN_TYPE_FOUND));

    sMessage.SearchAndReplaceAscii("#1",_rColumnName);

    OSQLMessageBox aMsg(this,sTitle,sMessage);
    aMsg.Execute();
}
// -----------------------------------------------------------------------------
//------dyf add
void OCopyTableWizard::ResetsName(const ::rtl::OUString & _sName)
{
    if ( _sName.getLength() != 0 )
        m_sName = _sName;

}
//------dyf add end
//-------------------------------------------------------------------------------
