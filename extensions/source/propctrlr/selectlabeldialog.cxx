/*************************************************************************
 *
 *  $RCSfile: selectlabeldialog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:03:58 $
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

#ifndef _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_
#include "selectlabeldialog.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;

    //========================================================================
    // OSelectLabelDialog
    //========================================================================
    DBG_NAME(OSelectLabelDialog)
    //------------------------------------------------------------------------
    OSelectLabelDialog::OSelectLabelDialog( Window* pParent, Reference< XPropertySet >  _xControlModel )
        :ModalDialog(pParent, ModuleRes(RID_DLG_SELECTLABELCONTROL))
        ,m_aMainDesc(this, ResId(1))
        ,m_aControlTree(this, ResId(1))
        ,m_aNoAssignment(this, ResId(1))
        ,m_aSeparator(this, ResId(1))
        ,m_aOk(this, ResId(1))
        ,m_aCancel(this, ResId(1))
        ,m_xControlModel(_xControlModel)
        ,m_pInitialSelection(NULL)
        ,m_aModelImages(ModuleRes(RID_IL_FORMEXPLORER))
        ,m_pLastSelected(NULL)
        ,m_bHaveAssignableControl(sal_False)
    {
        DBG_CTOR(OSelectLabelDialog,NULL);

        // initialize the TreeListBox
        m_aControlTree.SetSelectionMode( SINGLE_SELECTION );
        m_aControlTree.SetDragDropMode( 0 );
        m_aControlTree.EnableInplaceEditing( sal_False );
        m_aControlTree.SetWindowBits(WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HSCROLL);

        m_aControlTree.SetNodeBitmaps( m_aModelImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ), m_aModelImages.GetImage( RID_SVXIMG_EXPANDEDNODE ) );
        m_aControlTree.SetSelectHdl(LINK(this, OSelectLabelDialog, OnEntrySelected));
        m_aControlTree.SetDeselectHdl(LINK(this, OSelectLabelDialog, OnEntrySelected));

        // fill the description
        UniString sDescription = m_aMainDesc.GetText();
        sal_Int16 nClassID = FormComponentType::CONTROL;
        if (::comphelper::hasProperty(PROPERTY_CLASSID, m_xControlModel))
            nClassID = ::comphelper::getINT16(m_xControlModel->getPropertyValue(PROPERTY_CLASSID));

        sDescription.SearchAndReplace(String::CreateFromAscii("$control_class$"), GetUIHeadlineName(nClassID, makeAny(m_xControlModel)));
        UniString sName = ::comphelper::getString(m_xControlModel->getPropertyValue(PROPERTY_NAME)).getStr();
        sDescription.SearchAndReplace(String::CreateFromAscii("$control_name$"), sName);
        m_aMainDesc.SetText(sDescription);

        // search for the root of the form hierarchy
        Reference< XChild >  xCont(m_xControlModel, UNO_QUERY);
        Reference< XInterface >  xSearch( xCont.is() ? xCont->getParent() : Reference< XInterface > ());
        Reference< XResultSet >  xParentAsResultSet(xSearch, UNO_QUERY);
        while (xParentAsResultSet.is())
        {
            xCont = Reference< XChild > (xSearch, UNO_QUERY);
            xSearch = xCont.is() ? xCont->getParent() : Reference< XInterface > ();
            xParentAsResultSet = Reference< XResultSet > (xSearch, UNO_QUERY);
        }

        // and insert all entries below this root into the listbox
        if (xSearch.is())
        {
            // check wich service the allowed components must suppport
            sal_Int16 nClassId = 0;
            try { nClassId = ::comphelper::getINT16(m_xControlModel->getPropertyValue(PROPERTY_CLASSID)); } catch(...) { }
            m_sRequiredService = (FormComponentType::RADIOBUTTON == nClassId) ? SERVICE_COMPONENT_GROUPBOX : SERVICE_COMPONENT_FIXEDTEXT;
            m_aRequiredControlImage = m_aModelImages.GetImage((FormComponentType::RADIOBUTTON == nClassId) ? RID_SVXIMG_GROUPBOX : RID_SVXIMG_FIXEDTEXT);

            // calc the currently set label control (so InsertEntries can calc m_pInitialSelection)
            Any aCurrentLabelControl( m_xControlModel->getPropertyValue(PROPERTY_CONTROLLABEL) );
            DBG_ASSERT((aCurrentLabelControl.getValueTypeClass() == TypeClass_INTERFACE) || !aCurrentLabelControl.hasValue(),

                "OSelectLabelDialog::OSelectLabelDialog : invalid ControlLabel property !");
            if (aCurrentLabelControl.hasValue())
                aCurrentLabelControl >>= m_xInitialLabelControl;

            // insert the root
            Image aRootImage = m_aModelImages.GetImage(RID_SVXIMG_FORMS);
            SvLBoxEntry* pRoot = m_aControlTree.InsertEntry(ModuleRes(RID_STR_FORMS), aRootImage, aRootImage);

            // build the tree
            m_pInitialSelection = NULL;
            m_bHaveAssignableControl = sal_False;
            InsertEntries(xSearch, pRoot);
            m_aControlTree.Expand(pRoot);
        }

        if (m_pInitialSelection)
        {
            m_aControlTree.MakeVisible(m_pInitialSelection, sal_True);
            m_aControlTree.Select(m_pInitialSelection, sal_True);
        }
        else
        {
            m_aControlTree.MakeVisible(m_aControlTree.First(), sal_True);
            if (m_aControlTree.FirstSelected())
                m_aControlTree.Select(m_aControlTree.FirstSelected(), sal_False);
            m_aNoAssignment.Check(sal_True);
        }

        if (!m_bHaveAssignableControl)
        {   // no controls which can be assigned
            m_aNoAssignment.Check(sal_True);
            m_aNoAssignment.Enable(sal_False);
        }

        m_aNoAssignment.SetClickHdl(LINK(this, OSelectLabelDialog, OnNoAssignmentClicked));
        m_aNoAssignment.GetClickHdl().Call(&m_aNoAssignment);

        FreeResource();
    }

    //------------------------------------------------------------------------
    OSelectLabelDialog::~OSelectLabelDialog()
    {
        // delete the entry datas of the listbox entries
        SvLBoxEntry* pLoop = m_aControlTree.First();
        while (pLoop)
        {
            void* pData = pLoop->GetUserData();
            if (pData)
                delete (Reference< XPropertySet > *)pData;
            pLoop = m_aControlTree.Next(pLoop);
        }

        DBG_DTOR(OSelectLabelDialog,NULL);
    }

    //------------------------------------------------------------------------
    sal_Int32 OSelectLabelDialog::InsertEntries(const Reference< XInterface > & _xContainer, SvLBoxEntry* pContainerEntry)
    {
        Reference< XIndexAccess >  xContainer(_xContainer, UNO_QUERY);
        if (!xContainer.is())
            return 0;

        sal_Int32 nChildren = 0;
        UniString sName,sDisplayName;
        Reference< XPropertySet >  xAsSet;
        for (sal_Int32 i=0; i<xContainer->getCount(); ++i)
        {
            xContainer->getByIndex(i) >>= xAsSet;
            if (!xAsSet.is())
            {
                DBG_WARNING("OSelectLabelDialog::InsertEntries : strange : a form component which isn't a property set !");
                continue;
            }

            if (!::comphelper::hasProperty(PROPERTY_NAME, xAsSet))
                // we need at least a name for displaying ...
                continue;
            sName = ::comphelper::getString(xAsSet->getPropertyValue(PROPERTY_NAME)).getStr();

            // we need to check if the control model supports the required service
            Reference< XServiceInfo >  xInfo(xAsSet, UNO_QUERY);
            if (!xInfo.is())
                continue;

            if (!xInfo->supportsService(m_sRequiredService))
            {   // perhaps it is a container
                Reference< XIndexAccess >  xCont(xAsSet, UNO_QUERY);
                if (xCont.is() && xCont->getCount())
                {   // yes -> step down
                    Image aFormImage = m_aModelImages.GetImage( RID_SVXIMG_FORM );
                    SvLBoxEntry* pCont = m_aControlTree.InsertEntry(sName, aFormImage, aFormImage, pContainerEntry);
                    sal_Int32 nContChildren = InsertEntries(xCont, pCont);
                    if (nContChildren)
                    {
                        m_aControlTree.Expand(pCont);
                        ++nChildren;
                    }
                    else
                    {   // oops, no valid childs -> remove the entry
                        m_aControlTree.ModelIsRemoving(pCont);
                        m_aControlTree.GetModel()->Remove(pCont);
                        m_aControlTree.ModelHasRemoved(pCont);
                    }
                }
                continue;
            }

            // get the label
            if (!::comphelper::hasProperty(PROPERTY_LABEL, xAsSet))
                continue;
            sDisplayName = ::comphelper::getString(xAsSet->getPropertyValue(PROPERTY_LABEL)).getStr();
            sDisplayName += String::CreateFromAscii(" (");
            sDisplayName += sName;
            sDisplayName += ')';

            // all requirements met -> insert
            SvLBoxEntry* pCurrent = m_aControlTree.InsertEntry(sDisplayName, m_aRequiredControlImage, m_aRequiredControlImage, pContainerEntry);
            pCurrent->SetUserData(new Reference< XPropertySet > (xAsSet));
            ++nChildren;

            if (m_xInitialLabelControl == xAsSet)
                m_pInitialSelection = pCurrent;

            m_bHaveAssignableControl = sal_True;
        }

        return nChildren;
    }

    //------------------------------------------------------------------------
    IMPL_LINK(OSelectLabelDialog, OnEntrySelected, SvTreeListBox*, pLB)
    {
        DBG_ASSERT(pLB == &m_aControlTree, "OSelectLabelDialog::OnEntrySelected : where did this come from ?");
        SvLBoxEntry* pSelected = m_aControlTree.FirstSelected();
        void* pData = pSelected ? pSelected->GetUserData() : NULL;

        if (pData)
            m_xSelectedControl = Reference< XPropertySet > (*(Reference< XPropertySet > *)pData);

        m_aNoAssignment.SetClickHdl(Link());
        m_aNoAssignment.Check(pData == NULL);
        m_aNoAssignment.SetClickHdl(LINK(this, OSelectLabelDialog, OnNoAssignmentClicked));

        return 0L;
    }

    //------------------------------------------------------------------------
    IMPL_LINK(OSelectLabelDialog, OnNoAssignmentClicked, Button*, pButton)
    {
        DBG_ASSERT(pButton == &m_aNoAssignment, "OSelectLabelDialog::OnNoAssignmentClicked : where did this come from ?");

        if (m_aNoAssignment.IsChecked())
            m_pLastSelected = m_aControlTree.FirstSelected();
        else
        {
            DBG_ASSERT(m_bHaveAssignableControl, "OSelectLabelDialog::OnNoAssignmentClicked");
            // search the first assignable entry
            SvLBoxEntry* pSearch = m_aControlTree.First();
            while (pSearch)
            {
                if (pSearch->GetUserData())
                    break;
                pSearch = m_aControlTree.Next(pSearch);
            }
            // and select it
            if (pSearch)
            {
                m_aControlTree.Select(pSearch);
                m_pLastSelected = pSearch;
            }
        }

        if (m_pLastSelected)
        {
            m_aControlTree.SetSelectHdl(Link());
            m_aControlTree.SetDeselectHdl(Link());
            m_aControlTree.Select(m_pLastSelected, !m_aNoAssignment.IsChecked());
            m_aControlTree.SetSelectHdl(LINK(this, OSelectLabelDialog, OnEntrySelected));
            m_aControlTree.SetDeselectHdl(LINK(this, OSelectLabelDialog, OnEntrySelected));
        }

        return 0L;
    }

//............................................................................
}   // namespace pcr
//............................................................................

