/*************************************************************************
 *
 *  $RCSfile: groupboxwiz.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-28 09:18:30 $
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

#ifndef _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_
#include "groupboxwiz.hxx"
#endif
#ifndef _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_
#include "commonpagesdbp.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _EXTENSIONS_DBP_OPTIONGROUPLAYOUTER_HXX_
#include "optiongrouplayouter.hxx"
#endif


//#define GBW_STATE_DATASELECTION       0
#define GBW_STATE_OPTIONLIST        0
#define GBW_STATE_DEFAULTOPTION     1
#define GBW_STATE_OPTIONVALUES      2
#define GBW_STATE_DBFIELD           3
#define GBW_STATE_FINALIZE          4

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::svt;

    //=====================================================================
    //= OGroupBoxWizard
    //=====================================================================
    //---------------------------------------------------------------------
    OGroupBoxWizard::OGroupBoxWizard( Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XMultiServiceFactory >& _rxORB )
        :OControlWizard(_pParent, ModuleRes(RID_DLG_GROUPBOXWIZARD), _rxObjectModel, _rxORB)
        ,m_bVisitedDefault(sal_False)
        ,m_bVisitedDB(sal_False)
    {
        initControlSettings(&m_aSettings);
    }

    //---------------------------------------------------------------------
    sal_Bool OGroupBoxWizard::approveControlType(sal_Int16 _nClassId)
    {
        return FormComponentType::GROUPBOX == _nClassId;
    }

    //---------------------------------------------------------------------
    OWizardPage* OGroupBoxWizard::createPage(sal_uInt16 _nState)
    {
        switch (_nState)
        {
//          case GBW_STATE_DATASELECTION:
//              return new OTableSelectionPage(this);
//
            case GBW_STATE_OPTIONLIST:
                return new ORadioSelectionPage(this);

            case GBW_STATE_DEFAULTOPTION:
                return new ODefaultFieldSelectionPage(this);

            case GBW_STATE_OPTIONVALUES:
                return new OOptionValuesPage(this);

            case GBW_STATE_DBFIELD:
                return new OOptionDBFieldPage(this);

            case GBW_STATE_FINALIZE:
                return new OFinalizeGBWPage(this);
        }

        return NULL;
    }

    //---------------------------------------------------------------------
    sal_uInt16 OGroupBoxWizard::determineNextState(sal_uInt16 _nCurrentState)
    {
        switch (_nCurrentState)
        {
//          case GBW_STATE_DATASELECTION:
//              return GBW_STATE_OPTIONLIST;
//
            case GBW_STATE_OPTIONLIST:
                return GBW_STATE_DEFAULTOPTION;

            case GBW_STATE_DEFAULTOPTION:
                return GBW_STATE_OPTIONVALUES;

            case GBW_STATE_OPTIONVALUES:
                if (getContext().aFieldNames.getLength())
                    return GBW_STATE_DBFIELD;
                else
                    return GBW_STATE_FINALIZE;

            case GBW_STATE_DBFIELD:
                return GBW_STATE_FINALIZE;
        }

        return WZS_INVALID_STATE;
    }

    //---------------------------------------------------------------------
    void OGroupBoxWizard::enterState(sal_uInt16 _nState)
    {
        // some stuff to do before calling the base class (modifying our settings)
        switch (_nState)
        {
            case GBW_STATE_DEFAULTOPTION:
                if (!m_bVisitedDefault)
                {   // assume that the first of the radio buttons should be selected
                    DBG_ASSERT(m_aSettings.aLabels.size(), "OGroupBoxWizard::enterState: should never have reached this state!");
                    m_aSettings.sDefaultField = m_aSettings.aLabels[0];
                }
                m_bVisitedDefault = sal_True;
                break;

            case GBW_STATE_DBFIELD:
                if (!m_bVisitedDB)
                {   // try to generate a default for the DB field
                    // (simply use the first field in the DB names collection)
                    if (getContext().aFieldNames.getLength())
                        m_aSettings.sDBField = getContext().aFieldNames[0];
                }
                m_bVisitedDB = sal_True;
                break;
        }

        // setting the def button .... to be done before the base class is called, too, 'cause the base class
        // calls the pages, which are allowed to override our def button behaviour
        defaultButton(GBW_STATE_FINALIZE == _nState ? WZB_FINISH : WZB_NEXT);

        // allow "finish" on the last page only
        enableButtons(WZB_FINISH, GBW_STATE_FINALIZE == _nState);
        // allow previous on all pages but the first one
        enableButtons(WZB_PREVIOUS, GBW_STATE_OPTIONLIST != _nState);
        // allow next on all pages but the last one
        enableButtons(WZB_NEXT, GBW_STATE_FINALIZE != _nState);

        OControlWizard::enterState(_nState);
    }

    //---------------------------------------------------------------------
    sal_Bool OGroupBoxWizard::leaveState(sal_uInt16 _nState)
    {
        if (!OControlWizard::leaveState(_nState))
            return sal_False;

        return sal_True;
    }

    //---------------------------------------------------------------------
    void OGroupBoxWizard::createRadios()
    {
        try
        {
            OOptionGroupLayouter aLayouter(getServiceFactory());
            aLayouter.doLayout(getContext(), getSettings());
        }
        catch(Exception&)
        {
            DBG_ERROR("OGroupBoxWizard::createRadios: caught an exception while creating the radio shapes!");
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OGroupBoxWizard::onFinish(sal_Int32 _nResult)
    {
        if (RET_OK != _nResult)
            return OControlWizard::onFinish(_nResult);

        // commit the basic control setttings
        commitControlSettings(&m_aSettings);

        // create the radio buttons
        createRadios();

        return OControlWizard::onFinish(_nResult);
    }

    //=====================================================================
    //= ORadioSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    ORadioSelectionPage::ORadioSelectionPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, ModuleRes(RID_PAGE_GROUPRADIOSELECTION))
        ,m_aFrame               (this, ResId(FL_DATA))
        ,m_aRadioNameLabel      (this, ResId(FT_RADIOLABELS))
        ,m_aRadioName           (this, ResId(ET_RADIOLABELS))
        ,m_aMoveRight           (this, ResId(PB_MOVETORIGHT))
        ,m_aMoveLeft            (this, ResId(PB_MOVETOLEFT))
        ,m_aExistingRadiosLabel (this, ResId(FT_RADIOBUTTONS))
        ,m_aExistingRadios      (this, ResId(LB_RADIOBUTTONS))
    {
        FreeResource();

        m_aMoveLeft.SetClickHdl(LINK(this, ORadioSelectionPage, OnMoveEntry));
        m_aMoveRight.SetClickHdl(LINK(this, ORadioSelectionPage, OnMoveEntry));
        m_aRadioName.SetModifyHdl(LINK(this, ORadioSelectionPage, OnNameModified));
        m_aExistingRadios.SetSelectHdl(LINK(this, ORadioSelectionPage, OnEntrySelected));

        implCheckMoveButtons();
        m_aExistingRadios.EnableMultiSelection(sal_True);

        getDialog()->defaultButton(&m_aMoveRight);
    }

    //---------------------------------------------------------------------
    void ORadioSelectionPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_aRadioName.GrabFocus();
    }

    //---------------------------------------------------------------------
    void ORadioSelectionPage::initializePage()
    {
        OGBWPage::initializePage();

        m_aRadioName.SetText(String());

        // no need to initialize the list of radios here
        // (we're the only one affecting this special setting, so it will be in the same state as last time this
        // page was commited)

        implCheckMoveButtons();
    }

    //---------------------------------------------------------------------
    sal_Bool ORadioSelectionPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!OGBWPage::commitPage(_eReason))
            return sal_False;

        // copy the names of the radio buttons to be inserted
        // and initialize the values
        OOptionGroupSettings& rSettings = getSettings();
        rSettings.aLabels.clear();
        rSettings.aLabels.reserve(m_aExistingRadios.GetEntryCount());
        rSettings.aValues.reserve(m_aExistingRadios.GetEntryCount());
        for (sal_uInt16 i=0; i<m_aExistingRadios.GetEntryCount(); ++i)
        {
            rSettings.aLabels.push_back(m_aExistingRadios.GetEntry(i));
            rSettings.aValues.push_back(String::CreateFromInt32((sal_Int32)(i + 1)));
        }

        return sal_True;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( ORadioSelectionPage, OnMoveEntry, PushButton*, _pButton )
    {
        if (&m_aMoveLeft == _pButton)
        {
            while (m_aExistingRadios.GetSelectEntryCount())
                m_aExistingRadios.RemoveEntry(m_aExistingRadios.GetSelectEntryPos(0));
        }
        else
        {
            m_aExistingRadios.InsertEntry(m_aRadioName.GetText());
            m_aRadioName.SetText(String());
        }
        implCheckMoveButtons();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( ORadioSelectionPage, OnEntrySelected, ListBox*, _pList )
    {
        implCheckMoveButtons();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( ORadioSelectionPage, OnNameModified, Edit*, _pList )
    {
        implCheckMoveButtons();
        return 0L;
    }

    //---------------------------------------------------------------------
    void ORadioSelectionPage::implCheckMoveButtons()
    {
        sal_Bool bHaveSome = (0 != m_aExistingRadios.GetEntryCount());
        sal_Bool bSelectedSome = (0 != m_aExistingRadios.GetSelectEntryCount());
        sal_Bool bUnfinishedInput = (0 != m_aRadioName.GetText().Len());

        m_aMoveLeft.Enable(bSelectedSome);
        m_aMoveRight.Enable(bUnfinishedInput);

        getDialog()->enableButtons(WZB_NEXT, bHaveSome);

        if (bUnfinishedInput)
        {
            if (0 == (m_aMoveRight.GetStyle() & WB_DEFBUTTON))
                getDialog()->defaultButton(&m_aMoveRight);
        }
        else
        {
            if (WB_DEFBUTTON == (m_aMoveRight.GetStyle() & WB_DEFBUTTON))
                getDialog()->defaultButton(WZB_NEXT);
        }
    }

    //=====================================================================
    //= ODefaultFieldSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    ODefaultFieldSelectionPage::ODefaultFieldSelectionPage( OControlWizard* _pParent )
        :OMaybeListSelectionPage(_pParent, ModuleRes(RID_PAGE_DEFAULTFIELDSELECTION))
        ,m_aFrame                   (this, ResId(FL_DEFAULTSELECTION))
        ,m_aDefaultSelectionLabel   (this, ResId(FT_DEFAULTSELECTION))
        ,m_aDefSelYes               (this, ResId(RB_DEFSELECTION_YES))
        ,m_aDefSelection            (this, ResId(LB_DEFSELECTIONFIELD))
        ,m_aDefSelNo                (this, ResId(RB_DEFSELECTION_NO))
    {
        FreeResource();

        announceControls(m_aDefSelYes, m_aDefSelNo, m_aDefSelection);
        m_aDefSelection.SetDropDownLineCount(10);
    }

    //---------------------------------------------------------------------
    void ODefaultFieldSelectionPage::initializePage()
    {
        OMaybeListSelectionPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();

        // fill the listbox
        m_aDefSelection.Clear();
        for (   ConstStringArrayIterator aLoop = rSettings.aLabels.begin();
                aLoop != rSettings.aLabels.end();
                ++aLoop
            )
            m_aDefSelection.InsertEntry(*aLoop);


        implInitialize(rSettings.sDefaultField);
    }

    //---------------------------------------------------------------------
    sal_Bool ODefaultFieldSelectionPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!OMaybeListSelectionPage::commitPage(_eReason))
            return sal_False;

        OOptionGroupSettings& rSettings = getSettings();
        implCommit(rSettings.sDefaultField);

        return sal_True;
    }

    //=====================================================================
    //= OOptionValuesPage
    //=====================================================================
    //---------------------------------------------------------------------
    OOptionValuesPage::OOptionValuesPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, ModuleRes(RID_PAGE_OPTIONVALUES))
        ,m_aFrame               (this, ResId(FL_OPTIONVALUES))
        ,m_aDescription         (this, ResId(FT_OPTIONVALUES_EXPL))
        ,m_aValueLabel          (this, ResId(FT_OPTIONVALUES))
        ,m_aValue               (this, ResId(ET_OPTIONVALUE))
        ,m_aOptionsLabel        (this, ResId(FT_RADIOBUTTONS))
        ,m_aOptions             (this, ResId(LB_RADIOBUTTONS))
        ,m_nLastSelection((sal_uInt16)-1)
    {
        FreeResource();

        m_aOptions.SetSelectHdl(LINK(this, OOptionValuesPage, OnOptionSelected));
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OOptionValuesPage, OnOptionSelected, ListBox*, NOTINTERESTEDIN )
    {
        implTraveledOptions();
        return 0L;
    }

    //---------------------------------------------------------------------
    void OOptionValuesPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_aValue.GrabFocus();
    }

    //---------------------------------------------------------------------
    void OOptionValuesPage::implTraveledOptions()
    {
        if ((sal_uInt16)-1 != m_nLastSelection)
        {
            // save the value for the last option
            DBG_ASSERT(m_nLastSelection < m_aUncommittedValues.size(), "OOptionValuesPage::implTraveledOptions: invalid previous selection index!");
            m_aUncommittedValues[m_nLastSelection] = m_aValue.GetText();
        }

        m_nLastSelection = m_aOptions.GetSelectEntryPos();
        DBG_ASSERT(m_nLastSelection < m_aUncommittedValues.size(), "OOptionValuesPage::implTraveledOptions: invalid new selection index!");
        m_aValue.SetText(m_aUncommittedValues[m_nLastSelection]);
    }

    //---------------------------------------------------------------------
    void OOptionValuesPage::initializePage()
    {
        OGBWPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();
        DBG_ASSERT(rSettings.aLabels.size(), "OOptionValuesPage::initializePage: no options!!");
        DBG_ASSERT(rSettings.aLabels.size() == rSettings.aValues.size(), "OOptionValuesPage::initializePage: inconsistent data!");

        // fill the list with all available options
        m_aOptions.Clear();
        m_nLastSelection = -1;
        for (   ConstStringArrayIterator aLoop = rSettings.aLabels.begin();
                aLoop != rSettings.aLabels.end();
                ++aLoop
            )
            m_aOptions.InsertEntry(*aLoop);

        // remember the values ... can't set them directly in the settings without the explicit commit call
        // so we need have a copy of the values
        m_aUncommittedValues = rSettings.aValues;

        // select the first entry
        m_aOptions.SelectEntryPos(0);
        implTraveledOptions();
    }

    //---------------------------------------------------------------------
    sal_Bool OOptionValuesPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!OGBWPage::commitPage(_eReason))
            return sal_False;

        OOptionGroupSettings& rSettings = getSettings();

        // commit the current value
        implTraveledOptions();
        // copy the uncommitted values
        rSettings.aValues = m_aUncommittedValues;

        return sal_True;
    }

    //=====================================================================
    //= OOptionDBFieldPage
    //=====================================================================
    //---------------------------------------------------------------------
    OOptionDBFieldPage::OOptionDBFieldPage( OControlWizard* _pParent )
        :ODBFieldPage(_pParent)
    {
        setDescriptionText(String(ModuleRes(RID_STR_GROUPWIZ_DBFIELD)));
    }

    //---------------------------------------------------------------------
    String& OOptionDBFieldPage::getDBFieldSetting()
    {
        return getSettings().sDBField;
    }

    //=====================================================================
    //= OFinalizeGBWPage
    //=====================================================================
    //---------------------------------------------------------------------
    OFinalizeGBWPage::OFinalizeGBWPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, ModuleRes(RID_PAGE_OPTIONS_FINAL))
        ,m_aFrame           (this, ResId(FL_NAMEIT))
        ,m_aNameLabel       (this, ResId(FT_NAMEIT))
        ,m_aName            (this, ResId(ET_NAMEIT))
        ,m_aThatsAll        (this, ResId(FT_THATSALL))
    {
        FreeResource();
    }

    //---------------------------------------------------------------------
    void OFinalizeGBWPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_aName.GrabFocus();
    }

    //---------------------------------------------------------------------
    void OFinalizeGBWPage::initializePage()
    {
        OGBWPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();
        m_aName.SetText(rSettings.sControlLabel);
    }

    //---------------------------------------------------------------------
    sal_Bool OFinalizeGBWPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!OGBWPage::commitPage(_eReason))
            return sal_False;

        getSettings().sControlLabel = m_aName.GetText();

        return sal_True;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2001/02/23 15:19:08  fs
 *  some changes / centralizations - added the list-/combobox wizard
 *
 *  Revision 1.1  2001/02/21 09:23:55  fs
 *  initial checkin - form control auto pilots
 *
 *
 *  Revision 1.0 14.02.01 10:41:21  fs
 ************************************************************************/

