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

#include "selectlabeldialog.hxx"
#include <strings.hrc>
#include <bitmaps.hlst>
#include "formbrowsertools.hxx"
#include "formstrings.hxx"
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <vcl/treelistentry.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;


    // OSelectLabelDialog


    OSelectLabelDialog::OSelectLabelDialog( vcl::Window* pParent, Reference< XPropertySet > const & _xControlModel )
        :ModalDialog(pParent, "LabelSelectionDialog", "modules/spropctrlr/ui/labelselectiondialog.ui")
        ,m_xControlModel(_xControlModel)
        ,m_pInitialSelection(nullptr)
        ,m_pLastSelected(nullptr)
        ,m_bHaveAssignableControl(false)
    {
        get(m_pMainDesc, "label");
        get(m_pControlTree, "control");
        get(m_pNoAssignment, "noassignment");

        // initialize the TreeListBox
        m_pControlTree->SetSelectionMode( SelectionMode::Single );
        m_pControlTree->SetDragDropMode( DragDropMode::NONE );
        m_pControlTree->EnableInplaceEditing( false );
        m_pControlTree->SetStyle(m_pControlTree->GetStyle() | WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HSCROLL);

        m_pControlTree->SetNodeBitmaps(Image(StockImage::Yes, RID_EXTBMP_COLLAPSEDNODE),
                                       Image(StockImage::Yes, RID_EXTBMP_EXPANDEDNODE));
        m_pControlTree->SetSelectHdl(LINK(this, OSelectLabelDialog, OnEntrySelected));
        m_pControlTree->SetDeselectHdl(LINK(this, OSelectLabelDialog, OnEntrySelected));

        // fill the description
        OUString sDescription = m_pMainDesc->GetText();
        sal_Int16 nClassID = FormComponentType::CONTROL;
        if (::comphelper::hasProperty(PROPERTY_CLASSID, m_xControlModel))
            nClassID = ::comphelper::getINT16(m_xControlModel->getPropertyValue(PROPERTY_CLASSID));

        sDescription = sDescription.replaceAll("$controlclass$",
            GetUIHeadlineName(nClassID, makeAny(m_xControlModel)));
        OUString sName = ::comphelper::getString(m_xControlModel->getPropertyValue(PROPERTY_NAME));
        sDescription = sDescription.replaceAll("$controlname$", sName);
        m_pMainDesc->SetText(sDescription);

        // search for the root of the form hierarchy
        Reference< XChild >  xCont(m_xControlModel, UNO_QUERY);
        Reference< XInterface >  xSearch( xCont.is() ? xCont->getParent() : Reference< XInterface > ());
        Reference< XResultSet >  xParentAsResultSet(xSearch, UNO_QUERY);
        while (xParentAsResultSet.is())
        {
            xCont.set(xSearch, UNO_QUERY);
            xSearch = xCont.is() ? xCont->getParent() : Reference< XInterface > ();
            xParentAsResultSet.set(xSearch, UNO_QUERY);
        }

        // and insert all entries below this root into the listbox
        if (xSearch.is())
        {
            // check which service the allowed components must support
            sal_Int16 nClassId = 0;
            try { nClassId = ::comphelper::getINT16(m_xControlModel->getPropertyValue(PROPERTY_CLASSID)); } catch(...) { }
            m_sRequiredService = (FormComponentType::RADIOBUTTON == nClassId) ? OUString(SERVICE_COMPONENT_GROUPBOX) : OUString(SERVICE_COMPONENT_FIXEDTEXT);
            m_aRequiredControlImage = Image(StockImage::Yes, FormComponentType::RADIOBUTTON == nClassId ? OUStringLiteral(RID_EXTBMP_GROUPBOX) : OUStringLiteral(RID_EXTBMP_FIXEDTEXT));

            // calc the currently set label control (so InsertEntries can calc m_pInitialSelection)
            Any aCurrentLabelControl( m_xControlModel->getPropertyValue(PROPERTY_CONTROLLABEL) );
            DBG_ASSERT((aCurrentLabelControl.getValueTypeClass() == TypeClass_INTERFACE) || !aCurrentLabelControl.hasValue(),

                "OSelectLabelDialog::OSelectLabelDialog : invalid ControlLabel property !");
            if (aCurrentLabelControl.hasValue())
                aCurrentLabelControl >>= m_xInitialLabelControl;

            // insert the root
            Image aRootImage(StockImage::Yes, RID_EXTBMP_FORMS);
            SvTreeListEntry* pRoot = m_pControlTree->InsertEntry(PcrRes(RID_STR_FORMS), aRootImage, aRootImage);

            // build the tree
            m_pInitialSelection = nullptr;
            m_bHaveAssignableControl = false;
            InsertEntries(xSearch, pRoot);
            m_pControlTree->Expand(pRoot);
        }

        if (m_pInitialSelection)
        {
            m_pControlTree->MakeVisible(m_pInitialSelection, true);
            m_pControlTree->Select(m_pInitialSelection);
        }
        else
        {
            m_pControlTree->MakeVisible(m_pControlTree->First(), true);
            if (m_pControlTree->FirstSelected())
                m_pControlTree->Select(m_pControlTree->FirstSelected(), false);
            m_pNoAssignment->Check();
        }

        if (!m_bHaveAssignableControl)
        {   // no controls which can be assigned
            m_pNoAssignment->Check();
            m_pNoAssignment->Enable(false);
        }

        m_pNoAssignment->SetClickHdl(LINK(this, OSelectLabelDialog, OnNoAssignmentClicked));
        m_pNoAssignment->GetClickHdl().Call(m_pNoAssignment);
    }

    OSelectLabelDialog::~OSelectLabelDialog()
    {
        disposeOnce();
    }

    void OSelectLabelDialog::dispose()
    {
        // delete the entry datas of the listbox entries
        SvTreeListEntry* pLoop = m_pControlTree->First();
        while (pLoop)
        {
            void* pData = pLoop->GetUserData();
            if (pData)
                delete static_cast<Reference< XPropertySet > *>(pData);
            pLoop = m_pControlTree->Next(pLoop);
        }
        m_pMainDesc.clear();
        m_pControlTree.clear();
        m_pNoAssignment.clear();
        ModalDialog::dispose();
    }

    sal_Int32 OSelectLabelDialog::InsertEntries(const Reference< XInterface > & _xContainer, SvTreeListEntry* pContainerEntry)
    {
        Reference< XIndexAccess >  xContainer(_xContainer, UNO_QUERY);
        if (!xContainer.is())
            return 0;

        sal_Int32 nChildren = 0;
        OUString sName;
        Reference< XPropertySet >  xAsSet;
        for (sal_Int32 i=0; i<xContainer->getCount(); ++i)
        {
            xContainer->getByIndex(i) >>= xAsSet;
            if (!xAsSet.is())
            {
                SAL_INFO("extensions.propctrlr", "OSelectLabelDialog::InsertEntries : strange : a form component which isn't a property set !");
                continue;
            }

            if (!::comphelper::hasProperty(PROPERTY_NAME, xAsSet))
                // we need at least a name for displaying ...
                continue;
            sName = ::comphelper::getString(xAsSet->getPropertyValue(PROPERTY_NAME));

            // we need to check if the control model supports the required service
            Reference< XServiceInfo >  xInfo(xAsSet, UNO_QUERY);
            if (!xInfo.is())
                continue;

            if (!xInfo->supportsService(m_sRequiredService))
            {   // perhaps it is a container
                Reference< XIndexAccess >  xCont(xAsSet, UNO_QUERY);
                if (xCont.is() && xCont->getCount())
                {   // yes -> step down
                    Image aFormImage(StockImage::Yes, RID_EXTBMP_FORM);
                    SvTreeListEntry* pCont = m_pControlTree->InsertEntry(sName, aFormImage, aFormImage, pContainerEntry);
                    sal_Int32 nContChildren = InsertEntries(xCont, pCont);
                    if (nContChildren)
                    {
                        m_pControlTree->Expand(pCont);
                        ++nChildren;
                    }
                    else
                    {   // oops, no valid children -> remove the entry
                        m_pControlTree->ModelIsRemoving(pCont);
                        m_pControlTree->GetModel()->Remove(pCont);
                        m_pControlTree->ModelHasRemoved(pCont);
                    }
                }
                continue;
            }

            // get the label
            if (!::comphelper::hasProperty(PROPERTY_LABEL, xAsSet))
                continue;

            OUString sDisplayName = OUStringBuffer(
                ::comphelper::getString(xAsSet->getPropertyValue(PROPERTY_LABEL))).
                append(" (").append(sName).append(')').
                makeStringAndClear();

            // all requirements met -> insert
            SvTreeListEntry* pCurrent = m_pControlTree->InsertEntry(sDisplayName, m_aRequiredControlImage, m_aRequiredControlImage, pContainerEntry);
            pCurrent->SetUserData(new Reference< XPropertySet > (xAsSet));
            ++nChildren;

            if (m_xInitialLabelControl == xAsSet)
                m_pInitialSelection = pCurrent;

            m_bHaveAssignableControl = true;
        }

        return nChildren;
    }


    IMPL_LINK(OSelectLabelDialog, OnEntrySelected, SvTreeListBox*, pLB, void)
    {
        DBG_ASSERT(pLB == m_pControlTree, "OSelectLabelDialog::OnEntrySelected : where did this come from ?");
        SvTreeListEntry* pSelected = m_pControlTree->FirstSelected();
        void* pData = pSelected ? pSelected->GetUserData() : nullptr;

        if (pData)
            m_xSelectedControl.set(*static_cast<Reference< XPropertySet > *>(pData));

        m_pNoAssignment->SetClickHdl(Link<Button*,void>());
        m_pNoAssignment->Check(pData == nullptr);
        m_pNoAssignment->SetClickHdl(LINK(this, OSelectLabelDialog, OnNoAssignmentClicked));
    }


    IMPL_LINK(OSelectLabelDialog, OnNoAssignmentClicked, Button*, pButton, void)
    {
        DBG_ASSERT(pButton == m_pNoAssignment, "OSelectLabelDialog::OnNoAssignmentClicked : where did this come from ?");

        if (m_pNoAssignment->IsChecked())
            m_pLastSelected = m_pControlTree->FirstSelected();
        else
        {
            DBG_ASSERT(m_bHaveAssignableControl, "OSelectLabelDialog::OnNoAssignmentClicked");
            // search the first assignable entry
            SvTreeListEntry* pSearch = m_pControlTree->First();
            while (pSearch)
            {
                if (pSearch->GetUserData())
                    break;
                pSearch = m_pControlTree->Next(pSearch);
            }
            // and select it
            if (pSearch)
            {
                m_pControlTree->Select(pSearch);
                m_pLastSelected = pSearch;
            }
        }

        if (m_pLastSelected)
        {
            m_pControlTree->SetSelectHdl(Link<SvTreeListBox*,void>());
            m_pControlTree->SetDeselectHdl(Link<SvTreeListBox*,void>());
            m_pControlTree->Select(m_pLastSelected, !m_pNoAssignment->IsChecked());
            m_pControlTree->SetSelectHdl(LINK(this, OSelectLabelDialog, OnEntrySelected));
            m_pControlTree->SetDeselectHdl(LINK(this, OSelectLabelDialog, OnEntrySelected));
        }
    }


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
