/*************************************************************************
 *
 *  $RCSfile: general.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:03:02 $
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

#pragma hdrstop

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif
#ifndef _BIB_GENERAL_HXX
#include "general.hxx"
#endif
#include "sections.hrc"
#ifndef ADRRESID_HXX
#include "bibresid.hxx"
#endif
#ifndef _BIB_DATMAN_HXX
#include "datman.hxx"
#endif
#ifndef _BIBCONFIG_HXX
#include "bibconfig.hxx"
#endif
#ifndef _BIB_FMPROP_HRC
#include "bibprop.hrc"
#endif
#ifndef BIB_HRC
#include "bib.hrc"
#endif
#ifndef BIBMOD_HXX
#include "bibmod.hxx"
#endif
#ifndef BIBTOOLS_HXX
#include "bibtools.hxx"
#endif
#ifndef __EXTENSIONS_INC_EXTENSIO_HRC__
#include "extensio.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <vcl/i18nhelp.hxx>
#endif

#include <algorithm>
#include <functional>
#ifndef _VECTOR_
#include <vector>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::sdb;
using namespace ::rtl;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)

::Point lcl_MovePoint(const FixedText& rFixedText)
{
    ::Point aRet(rFixedText.GetPosPixel());
    aRet.X() += rFixedText.GetSizePixel().Width();
    aRet.X() += 5;
    return aRet;
}

//-----------------------------------------------------------------------------
OUString lcl_GetColumnName( const Mapping* pMapping, sal_uInt16 nIndexPos )
{
    BibConfig* pBibConfig = BibModul::GetConfig();
    OUString sRet = pBibConfig->GetDefColumnName(nIndexPos);
    if(pMapping)
        for(sal_uInt16 i = 0; i < COLUMN_COUNT; i++)
        {
            if(pMapping->aColumnPairs[i].sLogicalColumnName == sRet)
            {
                sRet = pMapping->aColumnPairs[i].sRealColumnName;
                break;
            }
        }
    return sRet;
}
/* -----------------------------04.01.00 10:54--------------------------------

 ---------------------------------------------------------------------------*/
class BibPosListener    :public cppu::WeakImplHelper1 <sdbc::XRowSetListener>
{
    BibGeneralPage*     pParentPage;
public:
    BibPosListener(BibGeneralPage* pParent);

    //XPositioningListener
    virtual void SAL_CALL cursorMoved(const lang::EventObject& event) throw( uno::RuntimeException );
    virtual void SAL_CALL rowChanged(const lang::EventObject& event) throw( uno::RuntimeException ){ /* not interested in */ }
    virtual void SAL_CALL rowSetChanged(const lang::EventObject& event) throw( uno::RuntimeException ){ /* not interested in */ }

    //XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& Source) throw( uno::RuntimeException );

};
/* -----------------------------04.01.00 10:57--------------------------------

 ---------------------------------------------------------------------------*/
BibPosListener::BibPosListener(BibGeneralPage* pParent) :
    pParentPage(pParent)
{
}
/* -----------------------------04.01.00 10:57--------------------------------

 ---------------------------------------------------------------------------*/
void BibPosListener::cursorMoved(const lang::EventObject& /*aEvent*/) throw( uno::RuntimeException )
{
    try
    {
        uno::Reference< form::XBoundComponent >  xLstBox = pParentPage->GetTypeListBoxModel();
        uno::Reference< beans::XPropertySet >  xPropSet(xLstBox, UNO_QUERY);
        if(xPropSet.is())
        {
            BibConfig* pBibConfig = BibModul::GetConfig();
            BibDataManager* pDatMan = pParentPage->GetDataManager();
            BibDBDescriptor aDesc;
            aDesc.sDataSource = pDatMan->getActiveDataSource();
            aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
            aDesc.nCommandType = CommandType::TABLE;

            const Mapping* pMapping = pBibConfig->GetMapping(aDesc);
            OUString sTypeMapping = pBibConfig->GetDefColumnName(AUTHORITYTYPE_POS);
            if(pMapping)
            {
                for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
                {
                    if(pMapping->aColumnPairs[nEntry].sLogicalColumnName == sTypeMapping)
                    {
                        sTypeMapping = pMapping->aColumnPairs[nEntry].sRealColumnName;
                        break;
                    }
                }
            }
            rtl::OUString uTypeMapping = sTypeMapping;

            uno::Reference< form::XForm >  xForm = pDatMan->getForm();
            uno::Reference< sdbcx::XColumnsSupplier >  xSupplyCols(xForm, UNO_QUERY);
            uno::Reference< container::XNameAccess >  xValueAcc;
            if (xSupplyCols.is())
                xValueAcc = xSupplyCols->getColumns();

            sal_Int16 nTempVal = -1;
            if(xValueAcc.is() && xValueAcc->hasByName(uTypeMapping))
            {
                uno::Any aVal = xValueAcc->getByName(uTypeMapping);
                uno::Reference< uno::XInterface >  xInt = *(uno::Reference< uno::XInterface > *)aVal.getValue();
                uno::Reference< sdb::XColumn >  xCol(xInt, UNO_QUERY);
                DBG_ASSERT(xCol.is(), "BibPosListener::positioned : invalid column (no sdb::XColumn) !");
                if (xCol.is())
                {
                    nTempVal = xCol->getShort();
                    // getShort returns zero if the value is not a number
                    if (!nTempVal || xCol->wasNull())
                    {
                        rtl::OUString sTempVal = xCol->getString();
                        if(sTempVal != rtl::OUString('0'))
                            nTempVal = -1;
                    }
                }
            }
            if(nTempVal < 0 || nTempVal >= TYPE_COUNT)
            {
                uno::Any aSel;
                uno::Sequence<sal_Int16> aSelSeq(1);
                sal_Int16* pArr = aSelSeq.getArray();
                pArr[0] = TYPE_COUNT;
                aSel.setValue(&aSelSeq, ::getCppuType((Sequence<sal_Int16>*)0));
                xPropSet->setPropertyValue(C2U("SelectedItems"), aSel);
            }
        }
    }
    catch(Exception& rEx)
    {
        rEx;
        DBG_ERROR("BibPosListener::positioned: something went wrong !");
    }
}
/* -----------------------------04.01.00 11:28--------------------------------

 ---------------------------------------------------------------------------*/
void BibPosListener::disposing(const lang::EventObject& Source) throw( uno::RuntimeException )
{
}

/* -----------------16.11.99 13:06-------------------

 --------------------------------------------------*/
BibGeneralPage::BibGeneralPage(Window* pParent, BibDataManager* pMan):
    BibTabPage(pParent,BibResId(RID_TP_GENERAL)),
    pDatMan(pMan),
    aControlParentWin(this, WB_DIALOGCONTROL),
    aIdentifierFT(&aControlParentWin,   ResId(FT_IDENTIFIER     )),
    aAuthTypeFT(&aControlParentWin,     ResId(FT_AUTHTYPE       )),
    aAuthorFT(&aControlParentWin,       ResId(FT_AUTHOR         )),
    aMonthFT(&aControlParentWin,        ResId(FT_MONTH          )),
    aYearFT(&aControlParentWin,         ResId(FT_YEAR           )),

    aPublisherFT(&aControlParentWin,    ResId(FT_PUBLISHER      )),
    aISBNFT(&aControlParentWin,         ResId(FT_ISBN           )),

    aAddressFT(&aControlParentWin,      ResId(FT_ADDRESS        )),
    aAnnoteFT(&aControlParentWin,       ResId(FT_ANNOTE         )),
    aBooktitleFT(&aControlParentWin,    ResId(FT_BOOKTITLE      )),
    aChapterFT(&aControlParentWin,      ResId(FT_CHAPTER        )),
    aEditionFT(&aControlParentWin,      ResId(FT_EDITION        )),
    aEditorFT(&aControlParentWin,       ResId(FT_EDITOR         )),
    aHowpublishedFT(&aControlParentWin, ResId(FT_HOWPUBLISHED   )),
    aInstitutionFT(&aControlParentWin,  ResId(FT_INSTITUTION    )),
    aJournalFT(&aControlParentWin,      ResId(FT_JOURNAL        )),
    aNoteFT(&aControlParentWin,         ResId(FT_NOTE           )),
    aNumberFT(&aControlParentWin,       ResId(FT_NUMBER         )),
    aOrganizationsFT(&aControlParentWin,ResId(FT_ORGANIZATION   )),
    aPagesFT(&aControlParentWin,        ResId(FT_PAGE           )),
    aSchoolFT(&aControlParentWin,       ResId(FT_SCHOOL         )),
    aSeriesFT(&aControlParentWin,       ResId(FT_SERIES         )),
    aTitleFT(&aControlParentWin,        ResId(FT_TITLE          )),
    aReportTypeFT(&aControlParentWin,   ResId(FT_REPORT         )),
    aVolumeFT(&aControlParentWin,       ResId(FT_VOLUME         )),
    aURLFT(&aControlParentWin,          ResId(FT_URL            )),
    aCustom1FT(&aControlParentWin,      ResId(FT_CUSTOM1        )),
    aCustom2FT(&aControlParentWin,      ResId(FT_CUSTOM2        )),
    aCustom3FT(&aControlParentWin,      ResId(FT_CUSTOM3        )),
    aCustom4FT(&aControlParentWin,      ResId(FT_CUSTOM4        )),
    aCustom5FT(&aControlParentWin,      ResId(FT_CUSTOM5        )),
    aHoriScroll(this, WB_HORZ),
    aVertScroll(this, WB_VERT),
    sErrorPrefix(ResId(ST_ERROR_PREFIX))
{
    aControlParentWin.Show();
    aControlParentWin.SetHelpId(HID_BIB_CONTROL_PARENT);
    aStdSize = GetOutputSizePixel();

    aBibTypeArr[0] = String(ResId(ST_TYPE_ARTICLE));
    aBibTypeArr[1] = String(ResId(ST_TYPE_BOOK));
    aBibTypeArr[2] = String(ResId(ST_TYPE_BOOKLET));
    aBibTypeArr[3] = String(ResId(ST_TYPE_CONFERENCE));
    aBibTypeArr[4] = String(ResId(ST_TYPE_INBOOK    ));
    aBibTypeArr[5] = String(ResId(ST_TYPE_INCOLLECTION));
    aBibTypeArr[6] = String(ResId(ST_TYPE_INPROCEEDINGS));
    aBibTypeArr[7] = String(ResId(ST_TYPE_JOURNAL      ));
    aBibTypeArr[8] = String(ResId(ST_TYPE_MANUAL       ));
    aBibTypeArr[9] = String(ResId(ST_TYPE_MASTERSTHESIS));
    aBibTypeArr[10] = String(ResId(ST_TYPE_MISC         ));
    aBibTypeArr[11] = String(ResId(ST_TYPE_PHDTHESIS    ));
    aBibTypeArr[12] = String(ResId(ST_TYPE_PROCEEDINGS  ));
    aBibTypeArr[13] = String(ResId(ST_TYPE_TECHREPORT   ));
    aBibTypeArr[14] = String(ResId(ST_TYPE_UNPUBLISHED  ));
    aBibTypeArr[15] = String(ResId(ST_TYPE_EMAIL        ));
    aBibTypeArr[16] = String(ResId(ST_TYPE_WWW          ));
    aBibTypeArr[17] = String(ResId(ST_TYPE_CUSTOM1      ));
    aBibTypeArr[18] = String(ResId(ST_TYPE_CUSTOM2      ));
    aBibTypeArr[19] = String(ResId(ST_TYPE_CUSTOM3      ));
    aBibTypeArr[20] = String(ResId(ST_TYPE_CUSTOM4      ));
    aBibTypeArr[21] = String(ResId(ST_TYPE_CUSTOM5      ));

    FreeResource();

    InitFixedTexts();

    aBasePos = aIdentifierFT.GetPosPixel();

    INT16* pMap = nFT2CtrlMap;
    for( USHORT i = 0 ; i < FIELD_COUNT ; ++i, ++pMap )
    {
        aControls[ i ] = 0;
        *pMap = -1;
    }

    AdjustScrollbars();
    Link aScrollLnk(LINK(this, BibGeneralPage, ScrollHdl));
    aHoriScroll.SetScrollHdl( aScrollLnk );
    aVertScroll.SetScrollHdl( aScrollLnk );
    aHoriScroll.SetLineSize(10);
    aVertScroll.SetLineSize(10);
    aHoriScroll.SetPageSize( aIdentifierFT.GetSizePixel().Width());
    aVertScroll.SetPageSize(
        aPublisherFT.GetPosPixel().Y() - aIdentifierFT.GetPosPixel().Y());
    aHoriScroll.Show();
    aVertScroll.Show();

    BibConfig* pBibConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pBibConfig->GetMapping(aDesc);

    xCtrlContnr = VCLUnoHelper::CreateControlContainer(&aControlParentWin);

    xMgr = comphelper::getProcessServiceFactory();

    AddControlWithError( lcl_GetColumnName( pMapping, IDENTIFIER_POS ), lcl_MovePoint( aIdentifierFT ),
        aIdentifierFT.GetSizePixel(), sTableErrorString, aIdentifierFT.GetText(),
        HID_BIB_IDENTIFIER_POS, 0 );

    sTypeColumnName = lcl_GetColumnName(pMapping, AUTHORITYTYPE_POS);

    AddControlWithError( sTypeColumnName, lcl_MovePoint(aAuthTypeFT ), aAuthTypeFT.GetSizePixel(), sTableErrorString,
        aAuthTypeFT.GetText(), HID_BIB_AUTHORITYTYPE_POS, 1 );

    ::Point aYearPos = lcl_MovePoint(aYearFT);
    AddControlWithError( lcl_GetColumnName( pMapping, YEAR_POS ),
        aYearPos, aYearFT.GetSizePixel(), sTableErrorString, aYearFT.GetText(), HID_BIB_YEAR_POS, 4 );

    AddControlWithError( lcl_GetColumnName(pMapping, AUTHOR_POS), lcl_MovePoint(aAuthorFT),
        aAuthorFT.GetSizePixel(), sTableErrorString, aAuthorFT.GetText(), HID_BIB_AUTHOR_POS, 2 );

    ::Point aTitlePos( lcl_MovePoint( aTitleFT ) );
    ::Size aTitleSize = aTitleFT.GetSizePixel();
    aTitleSize.Width() = aYearPos.X() + aYearFT.GetSizePixel().Width() - aTitlePos.X();
    AddControlWithError( lcl_GetColumnName(pMapping, TITLE_POS), aTitlePos, aTitleSize, sTableErrorString,
        aTitleFT.GetText(), HID_BIB_TITLE_POS, 22 );

    AddControlWithError( lcl_GetColumnName( pMapping, PUBLISHER_POS ), lcl_MovePoint( aPublisherFT),
        aPublisherFT.GetSizePixel(), sTableErrorString, aPublisherFT.GetText(), HID_BIB_PUBLISHER_POS, 5 );

    AddControlWithError( lcl_GetColumnName( pMapping, ADDRESS_POS ), lcl_MovePoint( aAddressFT ),
        aAddressFT.GetSizePixel(), sTableErrorString, aAddressFT.GetText(), HID_BIB_ADDRESS_POS, 7 );

    AddControlWithError( lcl_GetColumnName( pMapping, ISBN_POS ), lcl_MovePoint( aISBNFT ),
        aISBNFT.GetSizePixel(), sTableErrorString, aISBNFT.GetText(), HID_BIB_ISBN_POS, 6 );

    AddControlWithError( lcl_GetColumnName( pMapping, CHAPTER_POS ), lcl_MovePoint(aChapterFT),
        aChapterFT.GetSizePixel(), sTableErrorString, aChapterFT.GetText(), HID_BIB_CHAPTER_POS, 10 );

    AddControlWithError( lcl_GetColumnName( pMapping, PAGES_POS ), lcl_MovePoint( aPagesFT ), aPagesFT.GetSizePixel(),
        sTableErrorString, aPagesFT.GetText(), HID_BIB_PAGES_POS, 19 );

    AddControlWithError( lcl_GetColumnName( pMapping, EDITOR_POS ), lcl_MovePoint( aEditorFT ), aEditorFT.GetSizePixel(),
        sTableErrorString, aEditorFT.GetText(), HID_BIB_EDITOR_POS, 12 );

    AddControlWithError( lcl_GetColumnName( pMapping, EDITION_POS ), lcl_MovePoint(aEditionFT), aEditionFT.GetSizePixel(),
        sTableErrorString, aEditionFT.GetText(), HID_BIB_EDITION_POS, 11 );

    AddControlWithError( lcl_GetColumnName(pMapping, BOOKTITLE_POS), lcl_MovePoint(aBooktitleFT),
        aBooktitleFT.GetSizePixel(), sTableErrorString, aBooktitleFT.GetText(), HID_BIB_BOOKTITLE_POS, 9 );

    AddControlWithError( lcl_GetColumnName( pMapping, VOLUME_POS ), lcl_MovePoint( aVolumeFT ), aVolumeFT.GetSizePixel(),
        sTableErrorString, aVolumeFT.GetText(), HID_BIB_VOLUME_POS, 24 );

    AddControlWithError( lcl_GetColumnName( pMapping, HOWPUBLISHED_POS ), lcl_MovePoint( aHowpublishedFT ),
        aHowpublishedFT.GetSizePixel(), sTableErrorString, aHowpublishedFT.GetText(), HID_BIB_HOWPUBLISHED_POS, 13 );

    AddControlWithError( lcl_GetColumnName( pMapping, ORGANIZATIONS_POS ), lcl_MovePoint( aOrganizationsFT ),
        aOrganizationsFT.GetSizePixel(), sTableErrorString, aOrganizationsFT.GetText(), HID_BIB_ORGANIZATIONS_POS, 18 );

    AddControlWithError( lcl_GetColumnName( pMapping, INSTITUTION_POS ), lcl_MovePoint( aInstitutionFT ),
        aInstitutionFT.GetSizePixel(), sTableErrorString, aInstitutionFT.GetText(), HID_BIB_INSTITUTION_POS, 14 );

    AddControlWithError( lcl_GetColumnName( pMapping, SCHOOL_POS ), lcl_MovePoint( aSchoolFT ), aSchoolFT.GetSizePixel(),
        sTableErrorString, aSchoolFT.GetText(), HID_BIB_SCHOOL_POS, 20 );

    AddControlWithError( lcl_GetColumnName( pMapping, REPORTTYPE_POS ), lcl_MovePoint( aReportTypeFT ),
        aReportTypeFT.GetSizePixel(), sTableErrorString, aReportTypeFT.GetText(), HID_BIB_REPORTTYPE_POS, 23 );

    AddControlWithError( lcl_GetColumnName( pMapping, MONTH_POS ), lcl_MovePoint( aMonthFT ), aMonthFT.GetSizePixel(),
        sTableErrorString, aMonthFT.GetText(), HID_BIB_MONTH_POS, 3 );

    AddControlWithError( lcl_GetColumnName( pMapping, JOURNAL_POS ), lcl_MovePoint( aJournalFT ),
        aJournalFT.GetSizePixel(), sTableErrorString, aJournalFT.GetText(), HID_BIB_JOURNAL_POS, 15 );

    AddControlWithError( lcl_GetColumnName( pMapping, NUMBER_POS ), lcl_MovePoint( aNumberFT ), aNumberFT.GetSizePixel(),
        sTableErrorString, aNumberFT.GetText(), HID_BIB_NUMBER_POS, 17 );

    AddControlWithError( lcl_GetColumnName( pMapping, SERIES_POS ), lcl_MovePoint( aSeriesFT ), aSeriesFT.GetSizePixel(),
        sTableErrorString, aSeriesFT.GetText(), HID_BIB_SERIES_POS, 21 );

    AddControlWithError( lcl_GetColumnName( pMapping, ANNOTE_POS ), lcl_MovePoint( aAnnoteFT ), aAnnoteFT.GetSizePixel(),
        sTableErrorString, aAnnoteFT.GetText(), HID_BIB_ANNOTE_POS, 8 );

    AddControlWithError( lcl_GetColumnName( pMapping, NOTE_POS ), lcl_MovePoint( aNoteFT ), aNoteFT.GetSizePixel(),
        sTableErrorString, aNoteFT.GetText(), HID_BIB_NOTE_POS, 16 );

    AddControlWithError( lcl_GetColumnName( pMapping, URL_POS ), lcl_MovePoint( aURLFT ), aURLFT.GetSizePixel(),
        sTableErrorString, aURLFT.GetText(), HID_BIB_URL_POS, 25 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM1_POS ), lcl_MovePoint( aCustom1FT ),
        aCustom1FT.GetSizePixel(), sTableErrorString, aCustom1FT.GetText(), HID_BIB_CUSTOM1_POS, 26 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM2_POS ), lcl_MovePoint( aCustom2FT ), aCustom2FT.GetSizePixel(),
        sTableErrorString, aCustom2FT.GetText(), HID_BIB_CUSTOM2_POS, 27 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM3_POS ), lcl_MovePoint( aCustom3FT ), aCustom3FT.GetSizePixel(),
        sTableErrorString, aCustom3FT.GetText(), HID_BIB_CUSTOM3_POS, 28 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM4_POS ), lcl_MovePoint( aCustom4FT ), aCustom4FT.GetSizePixel(),
        sTableErrorString, aCustom4FT.GetText(), HID_BIB_CUSTOM4_POS, 29 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM5_POS ), lcl_MovePoint( aCustom5FT ), aCustom5FT.GetSizePixel(),
        sTableErrorString, aCustom5FT.GetText(), HID_BIB_CUSTOM5_POS, 30 );

    xPosListener = new BibPosListener(this);
    uno::Reference< sdbc::XRowSet >  xRowSet(pDatMan->getForm(), UNO_QUERY);
    if(xRowSet.is())
        xRowSet->addRowSetListener(xPosListener);
    uno::Reference< form::XFormController > xFormCtrl = pDatMan->GetFormController();
    xFormCtrl->setContainer(xCtrlContnr);
    xFormCtrl->activateTabOrder();

    if(sTableErrorString.Len())
        sTableErrorString.Insert(sErrorPrefix, 0);
}
//-----------------------------------------------------------------------------
BibGeneralPage::~BibGeneralPage()
{
    if (pDatMan && xPosListener.is())
    {
        uno::Reference< sdbc::XRowSet >  xRowSet(pDatMan->getForm(), UNO_QUERY);
        if(xRowSet.is())
            xRowSet->removeRowSetListener(xPosListener);
    }
}
 /* -----------------------------14.04.00 13:11--------------------------------

  ---------------------------------------------------------------------------*/
 void BibGeneralPage::RemoveListeners()
 {
    for(USHORT i = 0; i < FIELD_COUNT; i++)
    {
        if(aControls[i].is())
        {
            uno::Reference< awt::XWindow > xCtrWin(aControls[i], uno::UNO_QUERY );
            xCtrWin->removeFocusListener( this );
            aControls[i] = 0;
        }
    }
 }
/* -----------------------------21.01.00 17:05--------------------------------

 ---------------------------------------------------------------------------*/
void BibGeneralPage::CommitActiveControl()
{
    uno::Reference< form::XFormController > xFormCtrl = pDatMan->GetFormController();
    uno::Reference< awt::XControl >  xCurr = xFormCtrl->getCurrentControl();
    if(xCurr.is())
    {
        uno::Reference< awt::XControlModel >  xModel = xCurr->getModel();
        uno::Reference< form::XBoundComponent >  xBound(xModel, UNO_QUERY);
        if(xBound.is())
            xBound->commit();
    }
}
//-----------------------------------------------------------------------------
void BibGeneralPage::AddControlWithError( const OUString& rColumnName, const ::Point& rPos, const ::Size& rSize,
                                String& rErrorString, String aColumnUIName, sal_uInt16 nHelpId, sal_uInt16 nIndexInFTArray )
{
    // adds also the XControl and creates a map entry in nFT2CtrlMap[] for mapping between control and FT

    INT16                                   nIndex = -1;
    uno::Reference< awt::XControlModel >    xTmp = AddXControl(rColumnName, rPos, rSize, nHelpId, nIndex );
    if( xTmp.is() )
    {
        DBG_ASSERT( nIndexInFTArray < FIELD_COUNT, "*BibGeneralPage::AddControlWithError(): wrong array index!" );
        DBG_ASSERT( nFT2CtrlMap[ nIndexInFTArray ] < 0, "+BibGeneralPage::AddControlWithError(): index already in use!" );

        nFT2CtrlMap[ nIndexInFTArray ] = nIndex;
    }
    else
    {
        if( rErrorString.Len() )
            rErrorString += '\n';

        aColumnUIName.EraseAllChars( '~' );
        rErrorString += aColumnUIName;
    }
}
//-----------------------------------------------------------------------------
uno::Reference< awt::XControlModel >  BibGeneralPage::AddXControl(
        const String& rName,
        ::Point rPos, ::Size rSize, sal_uInt16 nHelpId, INT16& rIndex )
{
    uno::Reference< awt::XControlModel >  xCtrModel;
    try
    {
        sal_Bool bTypeListBox = sTypeColumnName == rName;
        xCtrModel = pDatMan->loadControlModel(rName, bTypeListBox);
        if ( xCtrModel.is() && xMgr.is())
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xCtrModel, UNO_QUERY );

            if( xPropSet.is())
            {
                uno::Reference< beans::XPropertySetInfo >  xPropInfo = xPropSet->getPropertySetInfo();

                uno::Any aAny = xPropSet->getPropertyValue( C2U("DefaultControl") );
                rtl::OUString aControlName;
                aAny >>= aControlName;

                rtl::OUString uProp(C2U("HelpURL"));
                if(xPropInfo->hasPropertyByName(uProp))
                {
                    String sId(C2S("HID:"));
                    sId += String::CreateFromInt32(nHelpId);
                    rtl::OUString uId(sId) ;
                    uno::Any aVal; aVal <<= uId;
                    xPropSet->setPropertyValue(uProp, aVal);
                }

                if(bTypeListBox)
                {
                    //uno::Reference< beans::XPropertySet >  xPropSet(xControl, UNO_QUERY);
                    uno::Any aAny;
                    aAny <<= (sal_Int16)1;
                    xPropSet->setPropertyValue(C2U("BoundColumn"), aAny);
                    ListSourceType eSet = ListSourceType_VALUELIST;
                    aAny.setValue( &eSet, ::getCppuType((const ListSourceType*)0) );
                    xPropSet->setPropertyValue(C2U("ListSourceType"), aAny);

                    uno::Sequence<rtl::OUString> aListSource(TYPE_COUNT);
                    rtl::OUString* pListSourceArr = aListSource.getArray();
                    //pListSourceArr[0] = C2U("select TypeName, TypeIndex from TypeNms");
                    for(sal_uInt16 i = 0; i < TYPE_COUNT; i++)
                        pListSourceArr[i] = String::CreateFromInt32(i);
                    aAny.setValue(&aListSource, ::getCppuType((uno::Sequence<rtl::OUString>*)0));

                    xPropSet->setPropertyValue(C2U("ListSource"), aAny);

                    uno::Sequence<rtl::OUString> aValues(TYPE_COUNT + 1);
                    rtl::OUString* pValuesArr = aValues.getArray();
                    for(sal_uInt16 j = 0; j < TYPE_COUNT; j++)
                        pValuesArr[j]  = aBibTypeArr[j];
                    // empty string if an invalid value no values is set
                    pValuesArr[TYPE_COUNT] = rtl::OUString();

                    aAny.setValue(&aValues, ::getCppuType((uno::Sequence<rtl::OUString>*)0));

                    xPropSet->setPropertyValue(C2U("StringItemList"), aAny);

                    BOOL bTrue = sal_True;
                    aAny.setValue( &bTrue, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( C2U("Dropdown"), aAny );

                    aControlName = C2U("com.sun.star.form.control.ListBox");
                    xLBModel = Reference< form::XBoundComponent >(xCtrModel, UNO_QUERY);

                }

                uno::Reference< awt::XControl >  xControl(xMgr->createInstance( aControlName ), UNO_QUERY );
                if ( xControl.is() )
                {
                    xControl->setModel( xCtrModel);

                    // Peer als Child zu dem FrameWindow
                    xCtrlContnr->addControl(rName, xControl);
                    uno::Reference< awt::XWindow >  xCtrWin(xControl, UNO_QUERY );
                    xCtrWin->addFocusListener( this );
                    rIndex = -1;    // -> implies, that not found
                    for(USHORT i = 0; i < FIELD_COUNT; i++)
                        if(!aControls[i].is())
                        {
                            aControls[i] = xCtrWin;
                            rIndex = INT16( i );
                            break;
                        }
                    xCtrWin->setVisible( sal_True );
                    xControl->setDesignMode( sal_True );
                        // initially switch on the desing mode - switch it off _after_ loading the form
                        // 17.10.2001 - 93107 - frank.schoenheit@sun.com

                    xCtrWin->setPosSize(rPos.X(), rPos.Y(), rSize.Width(),
                        rSize.Height(), awt::PosSize::POSSIZE);
                }
            }
        }
    }
    catch(Exception& rEx)
    {
        rEx;
        DBG_ERROR("BibGeneralPage::AddXControl: something went wrong !");
    }
    return xCtrModel;
}

void BibGeneralPage::AdjustScrollbars()
{
    long nVertScrollWidth = aVertScroll.GetSizePixel().Width();
    long nHoriScrollHeight = aHoriScroll.GetSizePixel().Height();
    ::Size aOutSize(GetOutputSizePixel());
    BOOL bHoriVisible = aOutSize.Width() <= aStdSize.Width();
    BOOL bVertVisible = (aOutSize.Height()-(bHoriVisible ? nHoriScrollHeight : 0)) <= (aStdSize.Height());
    aHoriScroll.Show(bHoriVisible);
    aVertScroll.Show(bVertVisible);

    if(bHoriVisible)
    {
        ::Size aHoriSize(aOutSize.Width() - (bVertVisible ? nVertScrollWidth : 0),
                        nHoriScrollHeight);
        aHoriScroll.SetSizePixel(aHoriSize);
        aHoriScroll.SetRange( Range(0, aStdSize.Width()));
        aHoriScroll.SetVisibleSize( aHoriSize.Width()  - (bVertVisible ? nVertScrollWidth : 0));
    }
    if(bVertVisible)
    {
        ::Size aVertSize(nHoriScrollHeight, aOutSize.Height() -
                                            (bHoriVisible ? nHoriScrollHeight : 0));
        aVertScroll.SetSizePixel(aVertSize);
        aVertScroll.SetRange( Range(0, aStdSize.Height()));
        aVertScroll.SetVisibleSize( aVertSize.Height() );
    }

    ::Size aSize(8, 8);
    aSize = LogicToPixel(aSize, MapMode(MAP_APPFONT));
    ::Size aScrollSize(aOutSize.Width() - aSize.Height(), aSize.Height());
    ::Point aScrollPos(0, aOutSize.Height() - aSize.Height());
    aHoriScroll.SetPosSizePixel(aScrollPos, aScrollSize);

    aScrollPos.X() = aOutSize.Width() - aSize.Width();
    aScrollPos.Y() = 0;
    aScrollSize.Width() = aSize.Width();
    aScrollSize.Height() = aOutSize.Height() - aSize.Height();
    aVertScroll.SetPosSizePixel(aScrollPos, aScrollSize);

    ::Size aControlParentWinSz(aOutSize);
    if(bHoriVisible)
        aControlParentWinSz.Height() -=  aSize.Height();
    if(bVertVisible)
        aControlParentWinSz.Width() -=  aSize.Width();
    aControlParentWin.SetSizePixel(aControlParentWinSz);
}

void BibGeneralPage::Resize()
{
    AdjustScrollbars();
    ScrollHdl(&aVertScroll);
    ScrollHdl(&aHoriScroll);
    Window::Resize();
}

void BibGeneralPage::InitFixedTexts( void )
{
    String      aFixedStrings[ FIELD_COUNT ] =
    {
        String( BibResId( ST_IDENTIFIER     ) ),
        String( BibResId( ST_AUTHTYPE       ) ),
        String( BibResId( ST_AUTHOR         ) ),
        String( BibResId( ST_TITLE          ) ),
        String( BibResId( ST_MONTH          ) ),
        String( BibResId( ST_YEAR           ) ),
        String( BibResId( ST_ISBN           ) ),
        String( BibResId( ST_BOOKTITLE      ) ),
        String( BibResId( ST_CHAPTER        ) ),
        String( BibResId( ST_EDITION        ) ),
        String( BibResId( ST_EDITOR         ) ),
        String( BibResId( ST_HOWPUBLISHED   ) ),
        String( BibResId( ST_INSTITUTION    ) ),
        String( BibResId( ST_JOURNAL        ) ),
        String( BibResId( ST_NOTE           ) ),
        String( BibResId( ST_ANNOTE         ) ),
        String( BibResId( ST_NUMBER         ) ),
        String( BibResId( ST_ORGANIZATION   ) ),
        String( BibResId( ST_PAGE           ) ),
        String( BibResId( ST_PUBLISHER      ) ),
        String( BibResId( ST_ADDRESS        ) ),
        String( BibResId( ST_SCHOOL         ) ),
        String( BibResId( ST_SERIES         ) ),
        String( BibResId( ST_REPORT         ) ),
        String( BibResId( ST_VOLUME         ) ),
        String( BibResId( ST_URL            ) ),
        String( BibResId( ST_CUSTOM1        ) ),
        String( BibResId( ST_CUSTOM2        ) ),
        String( BibResId( ST_CUSTOM3        ) ),
        String( BibResId( ST_CUSTOM4        ) ),
        String( BibResId( ST_CUSTOM5        ) )
    };

    aFixedTexts[0] = &aIdentifierFT;
    aFixedTexts[1] = &aAuthTypeFT;
    aFixedTexts[2] = &aAuthorFT;
    aFixedTexts[3] = &aTitleFT;
    aFixedTexts[4] = &aMonthFT;
    aFixedTexts[5] = &aYearFT;
    aFixedTexts[6] = &aISBNFT;
    aFixedTexts[7] = &aBooktitleFT;
    aFixedTexts[8] = &aChapterFT;
    aFixedTexts[9] = &aEditionFT;
    aFixedTexts[10] = &aEditorFT;
    aFixedTexts[11] = &aHowpublishedFT;
    aFixedTexts[12] = &aInstitutionFT;
    aFixedTexts[13] = &aJournalFT;
    aFixedTexts[14] = &aNoteFT;
    aFixedTexts[15] = &aAnnoteFT;
    aFixedTexts[16] = &aNumberFT;
    aFixedTexts[17] = &aOrganizationsFT;
    aFixedTexts[18] = &aPagesFT;
    aFixedTexts[19] = &aPublisherFT;
    aFixedTexts[20] = &aAddressFT;
    aFixedTexts[21] = &aSchoolFT;
    aFixedTexts[22] = &aSeriesFT;
    aFixedTexts[23] = &aReportTypeFT;
    aFixedTexts[24] = &aVolumeFT;
    aFixedTexts[25] = &aURLFT;
    aFixedTexts[26] = &aCustom1FT;
    aFixedTexts[27] = &aCustom2FT;
    aFixedTexts[28] = &aCustom3FT;
    aFixedTexts[29] = &aCustom4FT;
    aFixedTexts[30] = &aCustom5FT;

    int                 i;

    MnemonicGenerator   aMnemonicGenerator;
    // init mnemonics, first register all strings
    for( i = 0 ; i < FIELD_COUNT ; ++i )
        aMnemonicGenerator.RegisterMnemonic( aFixedStrings[ i ] );

    // ... then get all strings
    for( i = 0 ; i < FIELD_COUNT ; ++i )
        aMnemonicGenerator.CreateMnemonic( aFixedStrings[ i ] );

    // set texts
    for( i = 0 ; i < FIELD_COUNT ; ++i )
        aFixedTexts[ i ]->SetText( aFixedStrings[ i ] );
}

IMPL_LINK(BibGeneralPage, ScrollHdl, ScrollBar*, pScroll)
{
    BOOL bVertical = &aVertScroll == pScroll;
    long nOffset = 0;
    long nCurrentOffset = 0;
    if(bVertical)
        nCurrentOffset = aFixedTexts[0]->GetPosPixel().Y() - aBasePos.Y();
    else
        nCurrentOffset = aFixedTexts[0]->GetPosPixel().X() - aBasePos.X();
    nOffset = pScroll->IsVisible() ? pScroll->GetThumbPos() + nCurrentOffset : nCurrentOffset;;

    for(USHORT i = 0; i < FIELD_COUNT; i++)
    {
        ::Point aPos = aFixedTexts[i]->GetPosPixel();
        if(bVertical)
            aPos.Y() -= nOffset;
        else
            aPos.X() -= nOffset;
        aFixedTexts[i]->SetPosPixel(aPos);
        if(aControls[i].is())
        {
            awt::Rectangle aRect = aControls[i]->getPosSize();
            long nX = aRect.X;
            long nY = aRect.Y;
            if(bVertical)
                nY -= nOffset;
            else
                nX -= nOffset;
            aControls[i]->setPosSize(nX, nY, 0, 0, awt::PosSize::POS);
        }
    }
    return 0;
}

void BibGeneralPage::focusGained(const awt::FocusEvent& rEvent) throw( uno::RuntimeException )
{
    Reference<awt::XWindow> xCtrWin(rEvent.Source, UNO_QUERY );
    if(xCtrWin.is())
    {
        ::Size aOutSize = aControlParentWin.GetOutputSizePixel();
        awt::Rectangle aRect = xCtrWin->getPosSize();
        long nX = aRect.X;
        if(nX < 0)
        {
            // left of the visible area
            aHoriScroll.SetThumbPos(aHoriScroll.GetThumbPos() + nX);
            ScrollHdl(&aHoriScroll);
        }
        else if(nX > aOutSize.Width())
        {
            // right of the visible area
            aHoriScroll.SetThumbPos(aHoriScroll.GetThumbPos() + nX - aOutSize.Width() + aFixedTexts[0]->GetSizePixel().Width());
            ScrollHdl(&aHoriScroll);
        }
        long nY = aRect.Y;
        if(nY < 0)
        {
            // below the visible area
            aVertScroll.SetThumbPos(aVertScroll.GetThumbPos() + nY);
            ScrollHdl(&aVertScroll);
        }
        else if(nY > aOutSize.Height())
        {
            // over the visible area
            aVertScroll.SetThumbPos(aVertScroll.GetThumbPos() + nY - aOutSize.Height()+ aFixedTexts[0]->GetSizePixel().Height());
            ScrollHdl(&aVertScroll);
        }
    }
}

void BibGeneralPage::focusLost(const awt::FocusEvent& ) throw( uno::RuntimeException )
{
    CommitActiveControl();
}

void BibGeneralPage::disposing(const lang::EventObject& Source) throw( uno::RuntimeException )
{
}

void BibGeneralPage::GetFocus()
{
    Reference< awt::XWindow >*  pxControl = aControls;

    for( int i = FIELD_COUNT ; i ; --i, ++pxControl )
    {
        if( pxControl->is() )
        {
            ( *pxControl )->setFocus();
            return;
        }
    }

    // fallback
    aControlParentWin.GrabFocus();
}

BOOL BibGeneralPage::HandleShortCutKey( const KeyEvent& rKeyEvent )
{
    DBG_ASSERT( KEY_MOD2 == rKeyEvent.GetKeyCode().GetModifier(), "+BibGeneralPage::HandleShortCutKey(): this is not for me!" );

    const vcl::I18nHelper&      rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    const xub_Unicode           c = rKeyEvent.GetCharCode();
    BOOL                        bHandled = FALSE;

    sal_Int16                   i;

    typedef std::vector< sal_Int16 >    sal_Int16_vector;

    sal_Int16_vector::size_type nFocused = 0xFFFF;  // index of focused in vector, no one focused initial
    DBG_ASSERT( nFocused > 0, "*BibGeneralPage::HandleShortCutKey(): size_type works not as expected!" );

    sal_Int16_vector            aMatchList;

    for( i = 0 ; i < FIELD_COUNT ; ++i )
    {
        if( rI18nHelper.MatchMnemonic( aFixedTexts[ i ]->GetText(), c ) )
        {
            bHandled = TRUE;
            sal_Int16           nCtrlIndex = nFT2CtrlMap[ i ];

            if( nCtrlIndex >= 0 )
            {   // store index of control
                DBG_ASSERT( aControls[ nCtrlIndex ].is(), "-BibGeneralPage::HandleShortCutKey(): valid index and no control?" );

                uno::Reference< awt::XControl >  xControl( aControls[ nCtrlIndex ], UNO_QUERY );
                DBG_ASSERT( xControl.is(), "-BibGeneralPage::HandleShortCutKey(): a control wich is not a control!" );

                Window*         pWindow = VCLUnoHelper::GetWindow( xControl->getPeer() );

                if( pWindow )
                {
                    aMatchList.push_back( nCtrlIndex );
                    if( pWindow->HasChildPathFocus() )
                    {   // save focused control
                        DBG_ASSERT( nFocused == 0xFFFF, "+BibGeneralPage::HandleShortCutKey(): more than one with focus?!" );
                        DBG_ASSERT( aMatchList.size() > 0, "+BibGeneralPage::HandleShortCutKey(): push_back and no content?!" );
                        nFocused = aMatchList.size() - 1;
                    }
                }
            }
        }
    }

    if( bHandled )
    {
        DBG_ASSERT( aMatchList.size() > 0, "*BibGeneralPage::HandleShortCutKey(): be prepared to crash..." );

        if( nFocused >= ( aMatchList.size() - 1 ) )
            // >=... includes 0xFFFF
            // no one or last focused, take first
            nFocused = 0;
        else
            // take next one
            nFocused++;

        aControls[ aMatchList[ nFocused ] ]->setFocus();
    }

    return bHandled;
}
