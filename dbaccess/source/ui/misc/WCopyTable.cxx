/*************************************************************************
 *
 *  $RCSfile: WCopyTable.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-30 08:44:50 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
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
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;


#define MAX_PAGES   4   // max. Pages die angezeigt werden

DBG_NAME(OCopyTableWizard);
//------------------------------------------------------------------------
OCopyTableWizard::OCopyTableWizard(Window * pParent,
                                   const Reference< XPropertySet >& _xSourceObject,
                                   const Reference< XConnection >& _xConnection,
                                   const Reference< XNumberFormatter >& _xFormatter,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    : WizardDialog( pParent, ModuleRes(WIZ_RTFCOPYTABLE))
    ,m_pbFinish( this , ModuleRes(PB_OK))
    ,m_pbCancel( this , ModuleRes(PB_CANCEL))
    ,m_pbNext( this , ModuleRes(PB_NEXT))
    ,m_pbPrev( this , ModuleRes(PB_PREV))
    ,m_nPageCount(0)
    ,m_xConnection(_xConnection)
    ,m_xSourceObject(_xSourceObject)
    ,m_bCreatePrimaryColumn(sal_False)
    ,m_eCreateStyle(WIZARD_DEF_DATA)
    ,m_mNameMapping(_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers())
    ,m_xFormatter(_xFormatter)
    ,m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_xFactory(_rM)
{
    DBG_CTOR(OCopyTableWizard,NULL);
    construct();
    // Tabellennamen extrahieren
    if(m_xSourceObject.is())
    {
        Reference<XColumnsSupplier> xColSupp(m_xSourceObject,UNO_QUERY);
        if(xColSupp.is())
            m_xSourceColumns = xColSupp->getColumns();

        if(!_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_COMMAND))
        {
            ::rtl::OUString sCatalog,sSchema,sTable;
            m_xSourceObject->getPropertyValue(PROPERTY_CATALOGNAME) >>= sCatalog;
            m_xSourceObject->getPropertyValue(PROPERTY_SCHEMANAME)  >>= sSchema;
            m_xSourceObject->getPropertyValue(PROPERTY_NAME)        >>= sTable;

            ::dbtools::composeTableName(m_xConnection->getMetaData(),sCatalog,sSchema,sTable,m_sName,sal_False);
        }
        else
            _xSourceObject->getPropertyValue(PROPERTY_NAME)         >>= m_sName;
    }
    loadData(); // create the field description

}
// -----------------------------------------------------------------------------
OCopyTableWizard::OCopyTableWizard(Window * pParent,
                                   const ::rtl::OUString& _rDefaultName,
                                   const ODatabaseExport::TColumns& _rSourceColumns,
                                   const ODatabaseExport::TColumnVector& _rSourceColVec,
                                   const Reference< XConnection >& _xConnection,
                                   const Reference< XNumberFormatter >& _xFormatter,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    : WizardDialog( pParent, ModuleRes(WIZ_RTFCOPYTABLE))
    ,m_pbFinish( this , ModuleRes(PB_OK))
    ,m_pbCancel( this , ModuleRes(PB_CANCEL))
    ,m_pbNext( this , ModuleRes(PB_NEXT))
    ,m_pbPrev( this , ModuleRes(PB_PREV))
    ,m_nPageCount(0)
    ,m_xConnection(_xConnection)
    ,m_bCreatePrimaryColumn(sal_False)
    ,m_eCreateStyle(WIZARD_DEF_DATA)
    ,m_mNameMapping(_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers())
    ,m_vSourceColumns(_rSourceColumns)
    ,m_xFormatter(_xFormatter)
    ,m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_xFactory(_rM)
    ,m_sName(_rDefaultName)
    ,m_vSourceVec(_rSourceColVec)
{
    DBG_CTOR(OCopyTableWizard,NULL);
    construct();
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::construct()
{
    AddButton( &m_pbPrev );
    AddButton( &m_pbNext, WIZARDDIALOG_BUTTON_STDOFFSET_X );

    AddButton( &m_pbCancel, WIZARDDIALOG_BUTTON_STDOFFSET_X );
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

    fillTypeInfo();
}
//------------------------------------------------------------------------
OCopyTableWizard::~OCopyTableWizard()
{
    TabPage *pPage=0;
    while(pPage = GetPage(0))
    {
        RemovePage( pPage );
        delete pPage;
    }
    DBG_DTOR(OCopyTableWizard,NULL);
}
// -----------------------------------------------------------------------
IMPL_LINK( OCopyTableWizard, ImplPrevHdl, PushButton*, EMPTYARG )
{
    m_ePressed = WIZARD_PREV;
    if ( GetCurLevel() )
    {
        if(m_eCreateStyle != WIZARD_APPEND_DATA)
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
    m_ePressed = WIZARD_NEXT;
    if ( GetCurLevel() < MAX_PAGES )
    {
        if(m_eCreateStyle != WIZARD_APPEND_DATA)
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
void OCopyTableWizard::CheckColumns()
{
    m_vColumnPos.clear();
    m_vColumnTypes.clear();

    //////////////////////////////////////////////////////////////////////
    // Wenn Datenbank PrimaryKeys verarbeiten kann, PrimaryKey anlegen
    Reference< XDatabaseMetaData >  xMetaData(m_xConnection->getMetaData());
    sal_Bool bPKeyAllowed = xMetaData->supportsCoreSQLGrammar();


    if(m_vDestColumns.size())
    {   // we have dest columns so look for the column matching
        ODatabaseExport::TColumnVector::const_iterator aSrcIter = m_vSourceVec.begin();
        for(;aSrcIter != m_vSourceVec.end();++aSrcIter)
        {
            ODatabaseExport::TColumns::iterator aDestIter = m_vDestColumns.find(m_mNameMapping[(*aSrcIter)->first]);

            if(aDestIter != m_vDestColumns.end())
            {
                ODatabaseExport::TColumnVector::const_iterator aFind = ::std::find(m_aDestVec.begin(),m_aDestVec.end(),aDestIter);
                m_vColumnPos.push_back((aFind - m_aDestVec.begin())+1);
                m_vColumnTypes.push_back((*aFind)->second->GetType());
            }
            else
            {
                m_vColumnPos.push_back(CONTAINER_ENTRY_NOTFOUND);
                m_vColumnTypes.push_back(0);
            }
        }
    }
    else
    {
        //  DlgFieldMatch   dlgMissingFields(this);
        //  ListBox*        pInfoBox = dlgMissingFields.GetInfoBox();
        ::rtl::OUString aColumnName,aOldColName;
        sal_Int32 nMaxNameLen = xMetaData->getMaxColumnNameLength();
        ODatabaseExport::TColumnVector::const_iterator aSrcIter = m_vSourceVec.begin();
        for(;aSrcIter != m_vSourceVec.end();++aSrcIter)
        {
            if(nMaxNameLen && nMaxNameLen < (*aSrcIter)->first.getLength())
            {
                ::rtl::OUString aAlias(::dbtools::convertName2SQLName((*aSrcIter)->first,xMetaData->getExtraNameCharacters()));

                if(nMaxNameLen && aAlias.getLength() > nMaxNameLen)
                    aAlias = aAlias.copy(0,aAlias.getLength() - (aAlias.getLength()-nMaxNameLen-2));

                ::rtl::OUString sName(aAlias);
                sal_Int32 nPos = 1;
                sName += ::rtl::OUString::valueOf(nPos);

                while(m_vDestColumns.find(sName) != m_vDestColumns.end())
                {
                    sName = aAlias;
                    sName += ::rtl::OUString::valueOf(++nPos);
                }
                aAlias = sName;

                m_mNameMapping[(*aSrcIter)->first] = aAlias;
            }
            else
                m_mNameMapping[(*aSrcIter)->first] = (*aSrcIter)->first;

            // now create a column
            insertColumn(m_vDestColumns.size(),new OFieldDescription(*(*aSrcIter)->second));
            m_vColumnPos.push_back(m_vDestColumns.size());
            m_vColumnTypes.push_back((*aSrcIter)->second->GetType());
        }

//      if(pInfoBox->GetEntryCount())
//          dlgMissingFields.Execute();
    }
}
// -----------------------------------------------------------------------
IMPL_LINK( OCopyTableWizard, ImplOKHdl, OKButton*, EMPTYARG )
{
    m_ePressed = WIZARD_FINISH;
    sal_Bool bFinish = DeactivatePage() != 0;

    if(!bFinish)
        return sal_False;

    sal_Bool bWasEmpty = !m_vDestColumns.size();
    if(m_eCreateStyle != WIZARD_DEF_VIEW && m_eCreateStyle != WIZARD_APPEND_DATA )
        CheckColumns();

    switch(m_eCreateStyle)
    {
        case WIZARD_APPEND_DATA:
        {
            break;
        }
        case WIZARD_DEF_DATA:
        case WIZARD_DEF:
        {
            break;
        }
        case WIZARD_DEF_VIEW:
            break;
        default:
        {
            OSL_ASSERT(0);
        }
    }

    if(bFinish)
        EndDialog(sal_True);
    return bFinish;
}
//------------------------------------------------------------------------
sal_Bool OCopyTableWizard::SetAutoincrement() const
{
    return m_bCreatePrimaryColumn;
}
// -----------------------------------------------------------------------

IMPL_LINK( OCopyTableWizard, ImplActivateHdl, WizardDialog*, EMPTYARG )
{
    OWizardPage* pCurrent = (OWizardPage*)GetPage(GetCurLevel());
    if(!pCurrent)
        return 0;
    sal_Bool bFirstTime = pCurrent->IsFirstTime();
    if(bFirstTime)
        pCurrent->Reset();

    CheckButtons();

    SetText(pCurrent->GetTitle());

    Invalidate();
    return 0;
}
// -----------------------------------------------------------------------
void OCopyTableWizard::CheckButtons()
{
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
    OWizardPage* pPage = (OWizardPage*)GetPage(GetCurLevel());
    return pPage ? pPage->LeavePage() : sal_False;
}
// -----------------------------------------------------------------------
void OCopyTableWizard::AddWizardPage(OWizardPage* pPage)
{
    AddPage(pPage);
    ++m_nPageCount;
}
// -----------------------------------------------------------------------
void OCopyTableWizard::RemoveWizardPage(OWizardPage* pPage)
{
    RemovePage(pPage);
    --m_nPageCount;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::fillTypeInfo()
{
    if(!m_xConnection.is())
        return;
    Reference< XResultSet> xRs = m_xConnection->getMetaData()->getTypeInfo();
    Reference< XRow> xRow(xRs,UNO_QUERY);
    // Information for a single SQL type
    ::rtl::OUString aB1 = ::rtl::OUString::createFromAscii(" [ ");
    ::rtl::OUString aB2 = ::rtl::OUString::createFromAscii(" ]");

    // Loop on the result set until we reach end of file
    while (xRs->next())
    {
        OTypeInfo* pInfo = new OTypeInfo();
        pInfo->aTypeName        = xRow->getString (1);
        pInfo->nType            = xRow->getShort (2);
        pInfo->nPrecision       = xRow->getInt (3);
        pInfo->aLiteralPrefix   = xRow->getString (4);
        pInfo->aLiteralSuffix   = xRow->getString (5);
        pInfo->aCreateParams    = xRow->getString (6);
        pInfo->bNullable        = xRow->getInt (7) == ColumnValue::NULLABLE;
        pInfo->bCaseSensitive   = xRow->getBoolean (8);
        pInfo->nSearchType      = xRow->getShort (9);
        pInfo->bUnsigned        = xRow->getBoolean (10);
        pInfo->bCurrency        = xRow->getBoolean (11);
        pInfo->bAutoIncrement   = xRow->getBoolean (12);
        pInfo->aLocalTypeName   = xRow->getString (13);
        pInfo->nMinimumScale    = xRow->getShort (14);
        pInfo->nMaximumScale    = xRow->getShort (15);
        pInfo->nNumPrecRadix    = xRow->getInt (18);

        String aName;
        switch(pInfo->nType)
        {
            case DataType::CHAR:
                aName = m_sTypeNames.GetToken(TYPE_CHAR);
                break;
            case DataType::VARCHAR:
                aName = m_sTypeNames.GetToken(TYPE_TEXT);
                break;
            case DataType::DECIMAL:
                aName = m_sTypeNames.GetToken(TYPE_DECIMAL);
                break;
            case DataType::NUMERIC:
                aName = m_sTypeNames.GetToken(TYPE_NUMERIC);
                break;
            case DataType::BIGINT:
                aName = m_sTypeNames.GetToken(TYPE_BIGINT);
                break;
            case DataType::FLOAT:
                aName = m_sTypeNames.GetToken(TYPE_FLOAT);
                break;
            case DataType::DOUBLE:
                aName = m_sTypeNames.GetToken(TYPE_DOUBLE);
                break;
            case DataType::LONGVARCHAR:
                aName = m_sTypeNames.GetToken(TYPE_MEMO);
                break;
            case DataType::LONGVARBINARY:
                aName = m_sTypeNames.GetToken(TYPE_IMAGE);
                break;
            case DataType::DATE:
                aName = m_sTypeNames.GetToken(TYPE_DATE);
                break;
            case DataType::TIME:
                aName = m_sTypeNames.GetToken(TYPE_TIME);
                break;
            case DataType::TIMESTAMP:
                aName = m_sTypeNames.GetToken(TYPE_DATETIME);
                break;
            case DataType::BIT:
                aName = m_sTypeNames.GetToken(TYPE_BOOL);
                break;
            case DataType::TINYINT:
                aName = m_sTypeNames.GetToken(TYPE_TINYINT);
                break;
            case DataType::SMALLINT:
                aName = m_sTypeNames.GetToken(TYPE_SMALLINT);
                break;
            case DataType::INTEGER:
                aName = m_sTypeNames.GetToken(TYPE_INTEGER);
                break;
            case DataType::REAL:
                aName = m_sTypeNames.GetToken(TYPE_REAL);
                break;
            case DataType::BINARY:
                aName = m_sTypeNames.GetToken(TYPE_BINARY);
                break;
            case DataType::VARBINARY:
                aName = m_sTypeNames.GetToken(TYPE_VARBINARY);
                break;
            case DataType::SQLNULL:
                aName = m_sTypeNames.GetToken(TYPE_SQLNULL);
                break;
            case DataType::OBJECT:
                aName = m_sTypeNames.GetToken(TYPE_OBJECT);
                break;
            case DataType::DISTINCT:
                aName = m_sTypeNames.GetToken(TYPE_DISTINCT);
                break;
            case DataType::STRUCT:
                aName = m_sTypeNames.GetToken(TYPE_STRUCT);
                break;
            case DataType::ARRAY:
                aName = m_sTypeNames.GetToken(TYPE_ARRAY);
                break;
            case DataType::BLOB:
                aName = m_sTypeNames.GetToken(TYPE_BLOB);
                break;
            case DataType::CLOB:
                aName = m_sTypeNames.GetToken(TYPE_CLOB);
                break;
            case DataType::REF:
                aName = m_sTypeNames.GetToken(TYPE_REF);
                break;
            case DataType::OTHER:
                aName = m_sTypeNames.GetToken(TYPE_OTHER);
                break;
            default:
                OSL_ENSURE(0,"Unknown type");
        }
        pInfo->aUIName = aName.GetBuffer();
        pInfo->aUIName += aB1;
        pInfo->aUIName += pInfo->aTypeName;
        pInfo->aUIName += aB2;
        // Now that we have the type info, save it in the multimap
        m_aTypeInfo.insert(OTypeInfoMap::value_type(pInfo->nType,pInfo));
    }
    // for a faster index access
    OTypeInfoMap::iterator aIter = m_aTypeInfo.begin();
    for(;aIter != m_aTypeInfo.end();++aIter)
        m_aTypeInfoIndex.push_back(aIter);

    // Close the result set/statement.

    ::comphelper::disposeComponent(xRs);
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::insertColumn(sal_Int32 _nPos,OFieldDescription* _pField)
{
    m_aDestVec.insert(m_aDestVec.begin() + _nPos,
        m_vDestColumns.insert(ODatabaseExport::TColumns::value_type(_pField->GetName(),_pField)).first);
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::loadData()
{
    ODatabaseExport::TColumns::iterator aIter = m_vSourceColumns.begin();

    for(;aIter != m_vSourceColumns.end();++aIter)
        delete aIter->second;

    m_vSourceVec.clear();
    m_vSourceColumns.clear();

    Reference< XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    //////////////////////////////////////////////////////////////////////
    // Datenstruktur mit Daten aus DatenDefinitionsObjekt fuellen
    if(m_xSourceObject.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xSourceObject,UNO_QUERY);
        OSL_ENSURE(xColSup.is(),"No XColumnsSupplier!");
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        OFieldDescription* pActFieldDescr = NULL;
        String aType;
        //////////////////////////////////////////////////////////////////////
        // ReadOnly-Flag
        // Bei Drop darf keine Zeile editierbar sein.
        // Bei Add duerfen nur die leeren Zeilen editierbar sein.
        // Bei Add und Drop koennen alle Zeilen editiert werden.
        Sequence< ::rtl::OUString> aColumns = xColumns->getElementNames();
        const ::rtl::OUString* pBegin   = aColumns.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aColumns.getLength();

        for(;pBegin != pEnd;++pBegin)
        {
            Reference<XPropertySet> xColumn;
            xColumns->getByName(*pBegin) >>= xColumn;
            sal_Int32 nType         = 0;
            sal_Int32 nScale        = 0;
            sal_Int32 nPrecision    = 0;
            sal_Int32 nNullable     = 0;
            sal_Int32 nFormatKey    = 0;
            sal_Int32 nAlign        = 0;

            sal_Bool bIsAutoIncrement,bIsCurrency;
            ::rtl::OUString sName,sDefaultValue,sDescription,sTypeName;

            // get the properties from the column
            xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
            xColumn->getPropertyValue(PROPERTY_TYPENAME)        >>= sTypeName;
            xColumn->getPropertyValue(PROPERTY_ISNULLABLE)      >>= nNullable;
            bIsAutoIncrement    = ::cppu::any2bool(xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT));
            bIsCurrency         = ::cppu::any2bool(xColumn->getPropertyValue(PROPERTY_ISCURRENCY));
            xColumn->getPropertyValue(PROPERTY_TYPE)            >>= nType;
            xColumn->getPropertyValue(PROPERTY_SCALE)           >>= nScale;
            xColumn->getPropertyValue(PROPERTY_PRECISION)       >>= nPrecision;


            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION))
                xColumn->getPropertyValue(PROPERTY_DESCRIPTION) >>= sDescription;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE))
                xColumn->getPropertyValue(PROPERTY_DEFAULTVALUE)>>= sDefaultValue;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
                xColumn->getPropertyValue(PROPERTY_FORMATKEY)   >>= nFormatKey;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
                xColumn->getPropertyValue(PROPERTY_ALIGN)       >>= nAlign;


            pActFieldDescr = new OFieldDescription();
            // search for type
            ::std::pair<OTypeInfoMap::iterator, OTypeInfoMap::iterator> aPair = m_aTypeInfo.equal_range(nType);
            OTypeInfoMap::iterator aIter = aPair.first;
            if(aIter == m_aTypeInfo.end())
            {   // type not in destination database
                aPair = m_aTypeInfo.equal_range(DataType::VARCHAR);
            }
            for(;aIter != aPair.second;++aIter)
            {
                // search the best matching type
                if( aIter->second->aTypeName == sTypeName   &&
                    aIter->second->nPrecision >= nPrecision &&
                    aIter->second->nMaximumScale >= nScale)
                    break;
            }

            OTypeInfo* pType = aIter->second;
            pActFieldDescr->SetType(pType);
            switch(pType->nType)
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                    pActFieldDescr->SetPrecision(::std::min<sal_Int32>(sal_Int32(50),pType->nPrecision));
                    break;
                default:
                    if(pType->nPrecision && pType->nMaximumScale)
                    {
                        pActFieldDescr->SetPrecision(5);
                        pActFieldDescr->SetScale(0);
                    }
                    else if(pType->nPrecision)
                        pActFieldDescr->SetPrecision(::std::min<sal_Int32>(sal_Int32(16),pType->nPrecision));
            }

            pActFieldDescr->SetName(sName);
            pActFieldDescr->SetFormatKey(nFormatKey);
            pActFieldDescr->SetDescription(sDescription);
            pActFieldDescr->SetAutoIncrement(bIsAutoIncrement);
            pActFieldDescr->SetHorJustify((SvxCellHorJustify)nAlign);
            pActFieldDescr->SetCurrency(bIsCurrency);

            //////////////////////////////////////////////////////////////////////
            // Spezielle Daten
            pActFieldDescr->SetIsNullable(nNullable);
            pActFieldDescr->SetDefaultValue(sDefaultValue);
            pActFieldDescr->SetPrecision(nPrecision);
            pActFieldDescr->SetScale(nScale);
            m_vSourceColumns[sName] = pActFieldDescr;
            m_vSourceVec.push_back(m_vSourceColumns.insert(ODatabaseExport::TColumns::value_type(pActFieldDescr->GetName(),pActFieldDescr)).first);
        }
        // fill the primary  key information
        Reference<XNameAccess> xKeyColumns  = getKeyColumns();
        if(xKeyColumns.is())
        {
            Sequence< ::rtl::OUString> aKeyColumns = xKeyColumns->getElementNames();
            const ::rtl::OUString* pKeyBegin    = aKeyColumns.getConstArray();
            const ::rtl::OUString* pKeyEnd      = pKeyBegin + aKeyColumns.getLength();

            for(;pKeyBegin != pKeyEnd;++pKeyBegin)
            {
                ODatabaseExport::TColumns::iterator aIter = m_vSourceColumns.find(*pKeyBegin);
                if(aIter != m_vSourceColumns.end())
                    aIter->second->SetPrimaryKey(sal_True);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::clearDestColumns()
{
    ODatabaseExport::TColumns::iterator aIter = m_vDestColumns.begin();

    for(;aIter != m_vDestColumns.end();++aIter)
        delete aIter->second;

    m_aDestVec.clear();
    m_vDestColumns.clear();
}
// -----------------------------------------------------------------------------
Reference<XNameAccess> OCopyTableWizard::getKeyColumns() const
{
    // use keys and indexes for excat postioning
    // first the keys
    Reference<XKeysSupplier> xKeySup(m_xSourceObject,UNO_QUERY);
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
                xKeyColsSup = Reference<XColumnsSupplier>(xProp,UNO_QUERY);
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
                setColumnProperties(xColumn,pField);
            else
                xColumn->setPropertyValue(PROPERTY_NAME,makeAny(pField->GetName()));

            xAppend->appendByDescriptor(xColumn);
            xColumn = NULL;
            // now only the settings are missing
            if(xColumns->hasByName(pField->GetName()))
            {
                xColumns->getByName(pField->GetName()) >>= xColumn;
                if(xColumn.is())
                {
                    if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
                        xColumn->setPropertyValue(PROPERTY_FORMATKEY,makeAny(pField->GetFormatKey()));
                    if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
                        xColumn->setPropertyValue(PROPERTY_ALIGN,makeAny((sal_Int32)pField->GetHorJustify()));
                }
            }
            else
            {
                OSL_ASSERT(0);
            }

        }
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::appendKey(Reference<XKeysSupplier>& _rxSup,const ODatabaseExport::TColumnVector* _pVec)
{
    if(!_rxSup.is())
        return; // the database doesn't support keys
    OSL_ENSURE(_rxSup.is(),"No XKeysSupplier!");
    Reference<XDataDescriptorFactory> xKeyFactory(_rxSup->getKeys(),UNO_QUERY);
    OSL_ENSURE(xKeyFactory.is(),"No XDataDescriptorFactory Interface!");
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
        if(xColumns->getElementNames().getLength())
            xAppend->appendByDescriptor(xKey);
    }

}
// -----------------------------------------------------------------------------
void OCopyTableWizard::setColumnProperties(const Reference<XPropertySet>& _rxColumn,const OFieldDescription* _pFieldDesc)
{
    _rxColumn->setPropertyValue(PROPERTY_NAME,makeAny(_pFieldDesc->GetName()));
    _rxColumn->setPropertyValue(PROPERTY_TYPE,makeAny(_pFieldDesc->GetType()));
    _rxColumn->setPropertyValue(PROPERTY_TYPENAME,makeAny(_pFieldDesc->getTypeInfo()->aTypeName));
    _rxColumn->setPropertyValue(PROPERTY_PRECISION,makeAny(_pFieldDesc->GetPrecision()));
    _rxColumn->setPropertyValue(PROPERTY_SCALE,makeAny(_pFieldDesc->GetScale()));
    _rxColumn->setPropertyValue(PROPERTY_ISNULLABLE, makeAny(_pFieldDesc->GetIsNullable()));
    _rxColumn->setPropertyValue(PROPERTY_ISAUTOINCREMENT,::cppu::bool2any(_pFieldDesc->IsAutoIncrement()));
    //  _rxColumn->setPropertyValue(PROPERTY_ISCURRENCY,::cppu::bool2any(_pFieldDesc->IsCurrency()));
    if(_rxColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION))
        _rxColumn->setPropertyValue(PROPERTY_DESCRIPTION,makeAny(_pFieldDesc->GetDescription()));
    if(_rxColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE))
        _rxColumn->setPropertyValue(PROPERTY_DEFAULTVALUE,makeAny(_pFieldDesc->GetDefaultValue()));
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OCopyTableWizard::createView()
{
    Reference<XViewsSupplier> xSup(m_xConnection,UNO_QUERY);
    Reference< XNameAccess > xViews;
    if(xSup.is())
        xViews = xSup->getViews();
    Reference<XDataDescriptorFactory> xFact(xViews,UNO_QUERY);
    OSL_ENSURE(xFact.is(),"No XDataDescriptorFactory available!");
    if(!xFact.is())
        return NULL;

    m_xDestObject = xFact->createDataDescriptor();
    ::rtl::OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(m_xConnection->getMetaData(),
                                        m_sName,
                                        sCatalog,
                                        sSchema,
                                        sTable);

    m_xDestObject->setPropertyValue(PROPERTY_CATALOGNAME,makeAny(sCatalog));
    m_xDestObject->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(sSchema));
    m_xDestObject->setPropertyValue(PROPERTY_NAME,makeAny(sTable));

    ::rtl::OUString sCommand = ::rtl::OUString::createFromAscii("SELECT * FROM ");
    ::rtl::OUString sComposedName;
    ::dbaui::composeTableName(m_xConnection->getMetaData(),m_xSourceObject,sComposedName,sal_True);
    sCommand += sComposedName;
    m_xDestObject->setPropertyValue(PROPERTY_COMMAND,makeAny(sCommand));

    Reference<XAppend> xAppend(xViews,UNO_QUERY);
    if(xAppend.is())
        xAppend->appendByDescriptor(m_xDestObject);

    m_xDestObject = NULL;
    // we need to reget the view because after appending it it is no longer valid
    // but this time it isn't a view object it is a table object with type "VIEW"
    Reference<XTablesSupplier> xTabSup(m_xConnection,UNO_QUERY);
    Reference< XNameAccess > xTables;
    if(xSup.is())
        xTables = xTabSup->getTables();
    if(xTables.is() && xTables->hasByName(m_sName))
        xTables->getByName(m_sName) >>= m_xDestObject;

    return m_xDestObject;
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OCopyTableWizard::createTable()
{
    Reference<XTablesSupplier> xSup(m_xConnection,UNO_QUERY);
    Reference< XNameAccess > xTables;
    if(xSup.is())
        xTables = xSup->getTables();
    Reference<XDataDescriptorFactory> xFact(xTables,UNO_QUERY);
    OSL_ENSURE(xFact.is(),"No XDataDescriptorFactory available!");
    if(!xFact.is())
        return NULL;

    m_xDestObject = xFact->createDataDescriptor();
    OSL_ENSURE(m_xDestObject.is(),"Could not create a new object!");
    if(!m_xDestObject.is())
        return NULL;

    ::rtl::OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(m_xConnection->getMetaData(),
                                        m_sName,
                                        sCatalog,
                                        sSchema,
                                        sTable);

    m_xDestObject->setPropertyValue(PROPERTY_CATALOGNAME,makeAny(sCatalog));
    m_xDestObject->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(sSchema));
    m_xDestObject->setPropertyValue(PROPERTY_NAME,makeAny(sTable));

    if(m_xSourceObject.is()) // can be null when importing data from html or rtf format
    {
        if(m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_FONT))
            m_xDestObject->setPropertyValue(PROPERTY_FONT,m_xSourceObject->getPropertyValue(PROPERTY_FONT));
        if(m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_ROW_HEIGHT))
            m_xDestObject->setPropertyValue(PROPERTY_ROW_HEIGHT,m_xSourceObject->getPropertyValue(PROPERTY_ROW_HEIGHT));
        if(m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_TEXTCOLOR))
            m_xDestObject->setPropertyValue(PROPERTY_TEXTCOLOR,m_xSourceObject->getPropertyValue(PROPERTY_TEXTCOLOR));
        if(m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_ORDER))
            m_xDestObject->setPropertyValue(PROPERTY_ORDER,m_xSourceObject->getPropertyValue(PROPERTY_ORDER));
        if(m_xSourceObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_FILTER))
            m_xDestObject->setPropertyValue(PROPERTY_FILTER,m_xSourceObject->getPropertyValue(PROPERTY_FILTER));
    }
    // now append the columns
    const ODatabaseExport::TColumnVector* pVec = getDestVector();
    Reference<XColumnsSupplier> xColSup(m_xDestObject,UNO_QUERY);
    appendColumns(xColSup,pVec);
    // now append the primary key
    Reference<XKeysSupplier> xKeySup(m_xDestObject,UNO_QUERY);
    appendKey(xKeySup,pVec);

    Reference<XAppend> xAppend(xTables,UNO_QUERY);
    if(xAppend.is())
        xAppend->appendByDescriptor(m_xDestObject);

    m_xDestObject = NULL;
    // we need to reget the table because after appending it it is no longer valid
    if(xTables->hasByName(m_sName))
        xTables->getByName(m_sName) >>= m_xDestObject;

    return m_xDestObject;
}
// -----------------------------------------------------------------------------


