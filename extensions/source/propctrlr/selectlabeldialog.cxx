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
    OSelectLabelDialog::OSelectLabelDialog(weld::Window* pParent, Reference< XPropertySet > const & _xControlModel)
        : GenericDialogController(pParent, "modules/spropctrlr/ui/labelselectiondialog.ui", "LabelSelectionDialog")
        , m_xControlModel(_xControlModel)
        , m_bLastSelected(false)
        , m_bHaveAssignableControl(false)
        , m_xMainDesc(m_xBuilder->weld_label("label"))
        , m_xControlTree(m_xBuilder->weld_tree_view("control"))
        , m_xNoAssignment(m_xBuilder->weld_check_button("noassignment"))
    {
        m_xControlTree->connect_changed(LINK(this, OSelectLabelDialog, OnEntrySelected));
        m_xControlTree->set_size_request(-1, m_xControlTree->get_height_rows(8));

        // fill the description
        OUString sDescription = m_xMainDesc->get_label();
        sal_Int16 nClassID = FormComponentType::CONTROL;
        if (::comphelper::hasProperty(PROPERTY_CLASSID, m_xControlModel))
            nClassID = ::comphelper::getINT16(m_xControlModel->getPropertyValue(PROPERTY_CLASSID));

        sDescription = sDescription.replaceAll("$controlclass$",
            GetUIHeadlineName(nClassID, makeAny(m_xControlModel)));
        OUString sName = ::comphelper::getString(m_xControlModel->getPropertyValue(PROPERTY_NAME));
        sDescription = sDescription.replaceAll("$controlname$", sName);
        m_xMainDesc->set_label(sDescription);

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
            m_aRequiredControlImage = (FormComponentType::RADIOBUTTON == nClassId) ? OUString(RID_EXTBMP_GROUPBOX) : OUString(RID_EXTBMP_FIXEDTEXT);

            // calc the currently set label control (so InsertEntries can calc m_xInitialSelection)
            Any aCurrentLabelControl( m_xControlModel->getPropertyValue(PROPERTY_CONTROLLABEL) );
            DBG_ASSERT((aCurrentLabelControl.getValueTypeClass() == TypeClass_INTERFACE) || !aCurrentLabelControl.hasValue(),

                "OSelectLabelDialog::OSelectLabelDialog : invalid ControlLabel property !");
            if (aCurrentLabelControl.hasValue())
                aCurrentLabelControl >>= m_xInitialLabelControl;

            // insert the root
            OUString sRootName(PcrRes(RID_STR_FORMS));
            OUString aFormImage(RID_EXTBMP_FORMS);
            m_xControlTree->insert(nullptr, -1, &sRootName, nullptr,
                                   nullptr, nullptr, &aFormImage, false, nullptr);

            // build the tree
            m_xInitialSelection.reset();
            m_bHaveAssignableControl = false;
            std::unique_ptr<weld::TreeIter> xRoot = m_xControlTree->make_iterator();
            m_xControlTree->get_iter_first(*xRoot);
            InsertEntries(xSearch, *xRoot);
            m_xControlTree->expand_row(*xRoot);
        }

        if (m_xInitialSelection)
        {
            m_xControlTree->scroll_to_row(*m_xInitialSelection);
            m_xControlTree->select(*m_xInitialSelection);
        }
        else
        {
            m_xControlTree->scroll_to_row(0);
            m_xControlTree->unselect_all();
            m_xNoAssignment->set_active(true);
        }

        if (!m_bHaveAssignableControl)
        {   // no controls which can be assigned
            m_xNoAssignment->set_active(true);
            m_xNoAssignment->set_sensitive(false);
        }

        m_xLastSelected = m_xControlTree->make_iterator(nullptr);

        m_xNoAssignment->connect_toggled(LINK(this, OSelectLabelDialog, OnNoAssignmentClicked));
        OnNoAssignmentClicked(*m_xNoAssignment);
    }

    OSelectLabelDialog::~OSelectLabelDialog()
    {
    }

    sal_Int32 OSelectLabelDialog::InsertEntries(const Reference< XInterface > & _xContainer, weld::TreeIter& rContainerEntry)
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
                    OUString aFormImage(RID_EXTBMP_FORM);

                    m_xControlTree->insert(&rContainerEntry, -1, &sName, nullptr,
                                           nullptr, nullptr, &aFormImage, false, nullptr);
                    auto xIter = m_xControlTree->make_iterator(&rContainerEntry);
                    m_xControlTree->iter_nth_child(*xIter, nChildren);
                    sal_Int32 nContChildren = InsertEntries(xCont, *xIter);
                    if (nContChildren)
                    {
                        m_xControlTree->expand_row(*xIter);
                        ++nChildren;
                    }
                    else
                    {   // oops, no valid children -> remove the entry
                        m_xControlTree->remove(*xIter);
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
            m_xUserData.emplace_back(new Reference<XPropertySet>(xAsSet));
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(m_xUserData.back().get())));
            m_xControlTree->insert(&rContainerEntry, -1, &sDisplayName, &sId, nullptr, nullptr, &m_aRequiredControlImage, false, nullptr);

            if (m_xInitialLabelControl == xAsSet)
            {
                m_xInitialSelection = m_xControlTree->make_iterator(&rContainerEntry);
                m_xControlTree->iter_nth_child(*m_xInitialSelection, nChildren);
            }

            ++nChildren;
            m_bHaveAssignableControl = true;
        }

        return nChildren;
    }

    IMPL_LINK(OSelectLabelDialog, OnEntrySelected, weld::TreeView&, rLB, void)
    {
        DBG_ASSERT(&rLB == m_xControlTree.get(), "OSelectLabelDialog::OnEntrySelected : where did this come from ?");
        std::unique_ptr<weld::TreeIter> xIter = m_xControlTree->make_iterator();
        bool bSelected = m_xControlTree->get_selected(xIter.get());
        OUString sData = bSelected ? m_xControlTree->get_id(*xIter) : OUString();
        if (!sData.isEmpty())
            m_xSelectedControl.set(*reinterpret_cast<Reference<XPropertySet>*>(sData.toInt64()));
        m_xNoAssignment->set_active(sData.isEmpty());
    }

    IMPL_LINK(OSelectLabelDialog, OnNoAssignmentClicked, weld::ToggleButton&, rButton, void)
    {
        DBG_ASSERT(&rButton == m_xNoAssignment.get(), "OSelectLabelDialog::OnNoAssignmentClicked : where did this come from ?");

        if (m_xNoAssignment->get_active())
        {
            m_bLastSelected = m_xControlTree->get_selected(m_xLastSelected.get());
        }
        else
        {
            DBG_ASSERT(m_bHaveAssignableControl, "OSelectLabelDialog::OnNoAssignmentClicked");
            // search the first assignable entry
            auto xSearch = m_xControlTree->make_iterator(nullptr);
            bool bSearch = m_xControlTree->get_iter_first(*xSearch);
            while (bSearch)
            {
                if (m_xControlTree->get_id(*xSearch).toInt64())
                    break;
                bSearch = m_xControlTree->iter_next(*xSearch);
            }
            // and select it
            if (bSearch)
            {
                m_xControlTree->copy_iterator(*xSearch, *m_xLastSelected);
                m_xControlTree->select(*m_xLastSelected);
                m_bLastSelected = true;
            }
        }

        if (m_bLastSelected)
        {
            if (!m_xNoAssignment->get_active())
                m_xControlTree->select(*m_xLastSelected);
            else
                m_xControlTree->unselect(*m_xLastSelected);
        }
    }
}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
