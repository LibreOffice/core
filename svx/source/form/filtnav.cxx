/*************************************************************************
 *
 *  $RCSfile: filtnav.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-20 14:18:56 $
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
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SVX_FMCTRLER_HXX
#include "fmctrler.hxx"
#endif

#ifndef _SVX_FILTNAV_HXX
#include "filtnav.hxx"
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif

#ifndef _SV_EXCHANGE_HXX
#include <vcl/exchange.hxx>
#endif

#ifndef _DTRANS_HXX
#include <so3/dtrans.hxx>
#endif

#ifndef _SVX_FMEXCH_HXX
#include <fmexch.hxx>
#endif

#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif

#ifndef _SVX_DIALMGR_HXX //autogen
#include "dialmgr.hxx"
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVT_SDBPARSE_HXX //autogen wg. SdbSqlParser
#include <svtools/sdbparse.hxx>
#endif

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SVX_FMSHELL_HXX //autogen
#include <fmshell.hxx>
#endif

#ifndef _SVX_FMSHIMP_HXX
#include <fmshimp.hxx>
#endif

#ifndef _SVX_FMSERVS_HXX
#include <fmservs.hxx>
#endif

#ifndef _SVX_FMTOOLS_HXX
#include <fmtools.hxx>
#endif

#ifndef _SVX_DBERRBOX_HXX
#include "dbmsgbox.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _SVX_FMFILTER_HXX
#include "fmfilter.hxx"
#endif
#ifndef _SVX_GRIDCELL_HXX
#include "gridcell.hxx"
#endif

#define SYNC_DELAY                      200
#define DROP_ACTION_TIMER_INITIAL_TICKS     10
    // solange dauert es, bis das Scrollen anspringt
#define DROP_ACTION_TIMER_SCROLL_TICKS      3
    // in diesen Intervallen wird jeweils eine Zeile gescrollt
#define DROP_ACTION_TIMER_TICK_BASE         10
    // das ist die Basis, mit der beide Angaben multipliziert werden (in ms)

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


//========================================================================
SvxFmFilterExch::SvxFmFilterExch(FmFormItem* pFormItem, const ::std::vector<FmFilterItem*>& lstWhich )
                :m_aDraggedEntries(lstWhich)
                ,m_pFormItem(pFormItem)
{
    m_aDataTypeList.Insert( SvDataType(Exchange::RegisterFormatName(SVX_FM_FILTER_FIELDS)) );
}

//------------------------------------------------------------------------
sal_Bool SvxFmFilterExch::GetData( SvData* pData )
{
    return sal_False;
}

//========================================================================
TYPEINIT0(FmFilterData);
Image FmFilterData::GetImage() const {return Image();}

//========================================================================
TYPEINIT1(FmParentData, FmFilterData);
//------------------------------------------------------------------------
FmParentData::~FmParentData()
{
    for (::std::vector<FmFilterData*>::const_iterator i = m_aChilds.begin();
         i != m_aChilds.end(); i++)
        delete (*i);
}

//========================================================================
TYPEINIT1(FmFormItem, FmParentData);
//------------------------------------------------------------------------
Image FmFormItem::GetImage() const
{
    static Image aImage;
    if (!aImage)
    {
        ImageList aNavigatorImages(SVX_RES(RID_SVXIMGLIST_FMEXPL));
        aImage = aNavigatorImages.GetImage( RID_SVXIMG_FORM );
    }
    return aImage;
}

//========================================================================
TYPEINIT1(FmFilterItems, FmParentData);
//------------------------------------------------------------------------
FmFilterItem* FmFilterItems::Find(const Reference< ::com::sun::star::awt::XTextComponent > & _xText) const
{
    for (vector<FmFilterData*>::const_iterator i = m_aChilds.begin();
         i != m_aChilds.end(); i++)
    {
        FmFilterItem* pCond = PTR_CAST(FmFilterItem, *i);
        DBG_ASSERT(pCond, "Wrong element in container");
        if (_xText == pCond->GetTextComponent())
            return pCond;
    }
    return NULL;
}

//------------------------------------------------------------------------
Image FmFilterItems::GetImage() const
{
    static Image aImage;
    if (!aImage)
    {
        ImageList aNavigatorImages(SVX_RES(RID_SVXIMGLIST_FMEXPL));
        aImage = aNavigatorImages.GetImage( RID_SVXIMG_FILTER );
    }
    return aImage;
}

//========================================================================
TYPEINIT1(FmFilterItem, FmFilterData);
//------------------------------------------------------------------------
FmFilterItem::FmFilterItem(FmFilterItems* pParent,
                     const ::rtl::OUString& aFieldName,
                     const ::rtl::OUString& aText,
                     const Reference< ::com::sun::star::awt::XTextComponent > & _xText)
          :FmFilterData(pParent, aText)
          ,m_aFieldName(aFieldName)
          ,m_xText(_xText)
{
}

//------------------------------------------------------------------------
Image FmFilterItem::GetImage() const
{
    static Image aImage;
    if (!aImage)
    {
        ImageList aNavigatorImages(SVX_RES(RID_SVXIMGLIST_FMEXPL));
        aImage = aNavigatorImages.GetImage( RID_SVXIMG_FIELD );
    }
    return aImage;
}

//========================================================================
// Hints for communicatition between model and view
//========================================================================
class FmFilterHint : public SfxHint
{
    FmFilterData*   m_pData;

public:
    TYPEINFO();
    FmFilterHint(FmFilterData* pData):m_pData(pData){}
    FmFilterData* GetData() const { return m_pData; }
};
TYPEINIT1( FmFilterHint, SfxHint );

//========================================================================
class FmFilterInsertedHint : public FmFilterHint
{
    sal_Int32 m_nPos;   // Position relative to the parent of the data

public:
    TYPEINFO();
    FmFilterInsertedHint(FmFilterData* pData, sal_Int32 nRelPos)
        :FmFilterHint(pData)
        ,m_nPos(nRelPos){}

    sal_Int32 GetPos() const { return m_nPos; }
};
TYPEINIT1( FmFilterInsertedHint, FmFilterHint );

//========================================================================
class FmFilterReplacedHint : public FmFilterHint
{
public:
    TYPEINFO();
    FmFilterReplacedHint(FmFilterData* pData)
        :FmFilterHint(pData){}

};
TYPEINIT1( FmFilterReplacedHint, FmFilterHint );

//========================================================================
class FmFilterRemovedHint : public FmFilterHint
{
public:
    TYPEINFO();
    FmFilterRemovedHint(FmFilterData* pData)
        :FmFilterHint(pData){}

};
TYPEINIT1( FmFilterRemovedHint, FmFilterHint );

//========================================================================
class FmFilterTextChangedHint : public FmFilterHint
{
public:
    TYPEINFO();
    FmFilterTextChangedHint(FmFilterData* pData)
        :FmFilterHint(pData){}

};
TYPEINIT1( FmFilterTextChangedHint, FmFilterHint );

//========================================================================
class FmFilterClearedHint : public SfxHint
{
public:
    TYPEINFO();
    FmFilterClearedHint(){}
};
TYPEINIT1( FmFilterClearedHint, SfxHint );

//========================================================================
class FmFilterCurrentChangedHint : public SfxHint
{
public:
    TYPEINFO();
    FmFilterCurrentChangedHint(){}
};
TYPEINIT1( FmFilterCurrentChangedHint, SfxHint );

//========================================================================
// class FmFilterAdapter, Listener an den FilterControls
//========================================================================
class FmFilterAdapter : public ::cppu::WeakImplHelper1< ::com::sun::star::awt::XTextListener >
{
    FmFilterControls        m_aFilterControls;
    FmFilterModel*          m_pModel;

public:
    FmFilterAdapter(FmFilterModel* pModel, const Reference< ::com::sun::star::container::XIndexAccess >& xControllers);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException );

// ::com::sun::star::awt::XTextListener
    virtual void SAL_CALL textChanged(const ::com::sun::star::awt::TextEvent& e);

// helpers
    void dispose() throw( RuntimeException );

    void InsertElements(const Reference< ::com::sun::star::container::XIndexAccess >& xControllers);
    void RemoveElement(const Reference< ::com::sun::star::awt::XTextComponent > & xText);

    Reference< ::com::sun::star::beans::XPropertySet > getField(const Reference< ::com::sun::star::awt::XTextComponent > & xText) const;
    void setText(sal_Int32 nPos,
        const FmFilterItem* pFilterItem,
        const ::rtl::OUString& rText);
    void DeleteItemsByText(::std::vector<FmFilterData*>& rItems, const Reference< ::com::sun::star::awt::XTextComponent > & xText);
    Reference< ::com::sun::star::form::XForm > findForm(const Reference< ::com::sun::star::container::XChild >& xChild);
};

//------------------------------------------------------------------------
FmFilterAdapter::FmFilterAdapter(FmFilterModel* pModel, const Reference< ::com::sun::star::container::XIndexAccess >& xControllers)
                 :m_pModel(pModel)
{
    InsertElements(xControllers);

    // listen on all controls as text listener
    for (FmFilterControls::const_iterator iter = m_aFilterControls.begin();
        iter != m_aFilterControls.end(); iter++)
        (*iter).first->addTextListener(this);
}

//------------------------------------------------------------------------
void FmFilterAdapter::dispose() throw( RuntimeException )
{
    // clear the filter control map
    for (FmFilterControls::const_iterator iter = m_aFilterControls.begin();
         iter != m_aFilterControls.end(); iter++)
         (*iter).first->removeTextListener(this);

    m_aFilterControls.clear();
}

//------------------------------------------------------------------------------
// delete all items relate to the control
void FmFilterAdapter::DeleteItemsByText(::std::vector<FmFilterData*>& rItems,
                                        const Reference< ::com::sun::star::awt::XTextComponent > & xText)
{
    for (::std::vector<FmFilterData*>::reverse_iterator i = rItems.rbegin();
        // link problems with operator ==
        i.base() != rItems.rend().base(); i++)
    {
        FmFilterItems* pFilterItems = PTR_CAST(FmFilterItems, *i);
        if (pFilterItems)
        {
            FmFilterItem* pFilterItem = pFilterItems->Find(xText);
            if (pFilterItem)
            {
                // remove the condition
                ::std::vector<FmFilterData*>& rItems = pFilterItems->GetChilds();
                ::std::vector<FmFilterData*>::iterator j = find(rItems.begin(), rItems.end(), pFilterItem);
                if (j != rItems.end())
                    m_pModel->Remove(j, pFilterItem);
            }
            continue;
        }
        FmFormItem* pFormItem = PTR_CAST(FmFormItem, *i);
        if (pFormItem)
            DeleteItemsByText(pFormItem->GetChilds(), xText);
    }
}

//------------------------------------------------------------------------
void FmFilterAdapter::InsertElements(const Reference< ::com::sun::star::container::XIndexAccess >& xControllers)
{
    for (sal_Int32 i = 0, nLen = xControllers->getCount(); i < nLen; ++i)
    {
        Reference< ::com::sun::star::container::XIndexAccess > xElement;
        xControllers->getByIndex(i) >>= xElement;

        // Insert the Elements of the controller
        InsertElements(xElement);

        // store the filter controls
        FmXFormController* pController = NULL;
        //  ::comphelper::getImplementation(pController, Reference<XUnoTunnel>(xElement, UNO_QUERY));
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(xElement,::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xTunnel.is(), "FmFilterAdapter::InsertElements : xTunnel is invalid!");
        if(xTunnel.is())
        {
            pController = (FmXFormController*)xTunnel->getSomething(FmXFormController::getUnoTunnelImplementationId());
        }

        const FmFilterControls& rControls = pController->getFilterControls();
        for (FmFilterControls::const_iterator iter = rControls.begin(); iter != rControls.end(); ++iter )
            m_aFilterControls.insert(*iter);
    }
}

//------------------------------------------------------------------------------
void FmFilterAdapter::RemoveElement(const Reference< ::com::sun::star::awt::XTextComponent > & xText)
{
    if (xText.is())
    {
        // alle Level durchlaufen und eintraege entfernen
        if (m_pModel)
            DeleteItemsByText(m_pModel->GetChilds(), xText);

        FmFilterControls::iterator iter = m_aFilterControls.find(xText);
        if (iter != m_aFilterControls.end())
            m_aFilterControls.erase(iter);
    }
}

//------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySet >  FmFilterAdapter::getField(const Reference< ::com::sun::star::awt::XTextComponent > & xText) const
{
    Reference< ::com::sun::star::beans::XPropertySet >  xField;
    FmFilterControls::const_iterator i = m_aFilterControls.find(xText);
    if (i != m_aFilterControls.end())
        xField = (*i).second;

    return xField;
}

//------------------------------------------------------------------------
void FmFilterAdapter::setText(sal_Int32 nRowPos,
                              const FmFilterItem* pFilterItem,
                              const ::rtl::OUString& rText)
{
    // set the text for the text component
    Reference< ::com::sun::star::awt::XTextComponent > xText(pFilterItem->GetTextComponent());
    xText->setText(rText);

    // get the controller of the text component and its filter rows
    FmFormItem* pFormItem = PTR_CAST(FmFormItem,pFilterItem->GetParent()->GetParent());
    FmXFormController* pController = NULL;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(pFormItem->GetController(),::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xTunnel.is(), "FmFilterAdapter::InsertElements : xTunnel is invalid!");
    if(xTunnel.is())
    {
        pController = (FmXFormController*)xTunnel->getSomething(FmXFormController::getUnoTunnelImplementationId());
    }
    //  ::comphelper::getImplementation(pController, Reference<XUnoTunnel>(pFormItem->GetController(),UNO_QUERY));
    FmFilterRows& rRows = pController->getFilterRows();

    DBG_ASSERT(nRowPos < rRows.size(), "wrong row pos");
    // Suchen der aktuellen Row
    FmFilterRow& rRow = rRows[nRowPos];

    // do we have a new filter
    if (rText.len())
        rRow[xText] = rText;
    else
    {
        // do we have the control in the row
        FmFilterRow::iterator iter = rRow.find(xText);
        // erase the entry out of the row
        if (iter != rRow.end())
            rRow.erase(iter);
    }
}


// ::com::sun::star::lang::XEventListener
//------------------------------------------------------------------------
void SAL_CALL FmFilterAdapter::disposing(const ::com::sun::star::lang::EventObject& e) throw( RuntimeException )
{
    Reference< ::com::sun::star::awt::XTextComponent >  xText(e.Source,UNO_QUERY);
    if (xText.is())
        RemoveElement(xText);
}

// XTextListener
//------------------------------------------------------------------------
Reference< ::com::sun::star::form::XForm > FmFilterAdapter::findForm(const Reference< ::com::sun::star::container::XChild >& xChild)
{
    Reference< ::com::sun::star::form::XForm > xForm;
    if (xChild.is())
    {
        xForm = Reference< ::com::sun::star::form::XForm >(xChild->getParent(), UNO_QUERY);
        if (!xForm.is())
            xForm = findForm(Reference< ::com::sun::star::container::XChild >(xChild->getParent(), UNO_QUERY));
    }
    return xForm;
}

// XTextListener
//------------------------------------------------------------------------
void FmFilterAdapter::textChanged(const ::com::sun::star::awt::TextEvent& e)
{
    // Find the according formitem in the
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xControl(e.Source, UNO_QUERY);
    if (!m_pModel || !xControl.is())
        return;

    Reference< ::com::sun::star::form::XForm > xForm(findForm(Reference< ::com::sun::star::container::XChild >(xControl->getModel(), UNO_QUERY)));
    if (!xForm.is())
        return;

    FmFormItem* pFormItem = m_pModel->Find(m_pModel->m_aChilds, xForm);
    if (pFormItem)
    {
        Reference< ::com::sun::star::awt::XTextComponent > xText(e.Source, UNO_QUERY);
        FmFilterItems* pFilter = PTR_CAST(FmFilterItems, pFormItem->GetChilds()[pFormItem->GetCurrentPosition()]);
        FmFilterItem* pFilterItem = pFilter->Find(xText);
        if (pFilterItem)
        {
            if (xText->getText().len())
            {
                pFilterItem->SetText(xText->getText());
                // UI benachrichtigen
                FmFilterTextChangedHint aChangeHint(pFilterItem);
                m_pModel->Broadcast( aChangeHint );
            }
            else
            {
                // no text anymore so remove the condition
                m_pModel->Remove(pFilterItem);
            }
        }
        else
        {
            // searching the component by field name
            ::rtl::OUString aFieldName = getLabelName(Reference< ::com::sun::star::beans::XPropertySet > (Reference< ::com::sun::star::awt::XControl > (xText, UNO_QUERY)->getModel(),UNO_QUERY));

            pFilterItem = new FmFilterItem(pFilter, aFieldName, xText->getText(), xText);
            m_pModel->Insert(pFilter->GetChilds().end(), pFilterItem);
        }
        m_pModel->CheckIntegrity(pFormItem);
    }
}

//========================================================================
// class FmFilterModel
//========================================================================
TYPEINIT1(FmFilterModel, FmParentData);
//------------------------------------------------------------------------
FmFilterModel::FmFilterModel()
              :FmParentData(NULL, ::rtl::OUString())
              ,m_pAdapter(NULL)
              ,m_pCurrentItems(NULL)
{
}

//------------------------------------------------------------------------
FmFilterModel::~FmFilterModel()
{
    Clear();
}

//------------------------------------------------------------------------
void FmFilterModel::Clear()
{
    if (m_pAdapter)
    {
        m_pAdapter->dispose();
        m_pAdapter->release();
        m_pAdapter= NULL;
    }

    m_pCurrentItems  = NULL;
    m_xController    = NULL;
    m_xControllers   = NULL;

    for (::std::vector<FmFilterData*>::const_iterator i = m_aChilds.begin();
         i != m_aChilds.end(); i++)
        delete (*i);

    m_aChilds.clear();

    // UI benachrichtigen
    FmFilterClearedHint aClearedHint;
    Broadcast( aClearedHint );
}

//------------------------------------------------------------------------
void FmFilterModel::Update(const Reference< ::com::sun::star::container::XIndexAccess > & xControllers, const Reference< ::com::sun::star::form::XFormController > & xCurrent)
{
    if ((::com::sun::star::form::XFormController*) xCurrent.get() == (::com::sun::star::form::XFormController*) m_xController.get())
        return;

    if (!xControllers.is())
    {
        Clear();
        return;
    }

    // there is only a new current controller
    if ((::com::sun::star::container::XIndexAccess*)m_xControllers.get() != (::com::sun::star::container::XIndexAccess*)xControllers.get())
    {
        Clear();

        m_xControllers = xControllers;
        Update(m_xControllers, this);

        DBG_ASSERT(xCurrent.is(), "FmFilterModel::Update(...) no current controller");

        // Listening for TextChanges
        m_pAdapter = new FmFilterAdapter(this, xControllers);
        m_pAdapter->acquire();

        SetCurrentController(xCurrent);
        CheckIntegrity(this);
    }
    else
        SetCurrentController(xCurrent);
}

//------------------------------------------------------------------------
void FmFilterModel::Update(const Reference< ::com::sun::star::container::XIndexAccess > & xControllers, FmParentData* pParent)
{
    sal_Int32 nCount = xControllers->getCount();
    for (sal_Int32 i = 0; i < nCount; i++)
    {
        Reference< ::com::sun::star::form::XFormController >  xController(*(Reference< ::com::sun::star::form::XFormController > *)xControllers->getByIndex(i).getValue());
        Reference< ::com::sun::star::beans::XPropertySet >  xModelAsSet(xController->getModel(), UNO_QUERY);
        ::rtl::OUString aName = ::comphelper::getString(xModelAsSet->getPropertyValue(FM_PROP_NAME));

        // Insert a new ::com::sun::star::form
        FmFormItem* pFormItem = new FmFormItem(pParent, xController, aName);
        Insert(pParent->GetChilds().end(), pFormItem);

        // And now insert the filters for the form
        FmXFormController* pController = NULL;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(pFormItem->GetController(),::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xTunnel.is(), "FmFilterAdapter::InsertElements : xTunnel is invalid!");
        if(xTunnel.is())
        {
            pController = (FmXFormController*)xTunnel->getSomething(FmXFormController::getUnoTunnelImplementationId());
        }
        //  ::comphelper::getImplementation(pController, Reference<XUnoTunnel>(pFormItem->GetController(),UNO_QUERY));

        INT32 nPos = pController->getCurrentFilterPosition();
        pFormItem->SetCurrentPosition(nPos);

        String aTitle(SVX_RES(RID_STR_FILTER_FILTER_FOR));
        const FmFilterRows& rRows = pController->getFilterRows();
        for (FmFilterRows::const_iterator iter = rRows.begin(); iter != rRows.end(); ++iter)
        {
            const FmFilterRow& rRow = *iter;
            // now add the filter rows
            // One Row always exists

            FmFilterItems* pFilterItems = new FmFilterItems(pFormItem, aTitle);
            Insert(pFormItem->GetChilds().end(), pFilterItems);
            for (FmFilterRow::const_iterator iter1 = rRow.begin(); iter1 != rRow.end(); ++iter1)
            {
                // insert new and conditons
                ::rtl::OUString aFieldName = getLabelName(Reference< ::com::sun::star::beans::XPropertySet > (Reference< ::com::sun::star::awt::XControl > ((*iter1).first, UNO_QUERY)->getModel(),UNO_QUERY));
                FmFilterItem* pANDCondition = new FmFilterItem(pFilterItems, aFieldName, (*iter1).second, (*iter1).first);
                Insert(pFilterItems->GetChilds().end(), pANDCondition);
            }
            // title for the next conditions
            aTitle = SVX_RES(RID_STR_FILTER_FILTER_OR);
        }

        // now add dependent controllers
        Reference< ::com::sun::star::container::XIndexAccess >  xControllerAsIndex(xController, UNO_QUERY);
        Update(xControllerAsIndex, pFormItem);
    }
}

//------------------------------------------------------------------------
FmFormItem* FmFilterModel::Find(const ::std::vector<FmFilterData*>& rItems, const Reference< ::com::sun::star::form::XFormController > & xController) const
{
    for (::std::vector<FmFilterData*>::const_iterator i = rItems.begin();
         i != rItems.end(); i++)
    {
        FmFormItem* pForm = PTR_CAST(FmFormItem,*i);
        if (pForm)
        {
            if ((::com::sun::star::form::XFormController*)xController.get() == (::com::sun::star::form::XFormController*)pForm->GetController().get())
                return pForm;
            else
            {
                pForm = Find(pForm->GetChilds(), xController);
                if (pForm)
                    return pForm;
            }
        }
    }
    return NULL;
}

//------------------------------------------------------------------------
FmFormItem* FmFilterModel::Find(const ::std::vector<FmFilterData*>& rItems, const Reference< ::com::sun::star::form::XForm >& xForm) const
{
    for (::std::vector<FmFilterData*>::const_iterator i = rItems.begin();
         i != rItems.end(); i++)
    {
        FmFormItem* pForm = PTR_CAST(FmFormItem,*i);
        if (pForm)
        {
            if (xForm == pForm->GetController()->getModel())
                return pForm;
            else
            {
                pForm = Find(pForm->GetChilds(), xForm);
                if (pForm)
                    return pForm;
            }
        }
    }
    return NULL;
}

//------------------------------------------------------------------------
void FmFilterModel::SetCurrentController(const Reference< ::com::sun::star::form::XFormController > & xCurrent)
{
    if ((::com::sun::star::form::XFormController*) xCurrent.get() == (::com::sun::star::form::XFormController*) m_xController.get())
        return;

    m_xController = xCurrent;

    FmFormItem* pItem = Find(m_aChilds, xCurrent);
    if (pItem)
        SetCurrentItems((FmFilterItems*)pItem->GetChilds()[pItem->GetCurrentPosition()]);
}

//------------------------------------------------------------------------
void FmFilterModel::AppendFilterItems(FmFormItem* pFormItem)
{
    DBG_ASSERT(pFormItem, "AppendFilterItems(): no form item present");

    FmFilterItems* pFilterItems = new FmFilterItems(pFormItem, ::rtl::OUString(SVX_RES(RID_STR_FILTER_FILTER_OR)));
    // insert the condition behind the last filter items
    ::std::vector<FmFilterData*>::reverse_iterator iter;
    for (iter = pFormItem->GetChilds().rbegin();
        // link problems with operator ==
        iter.base() != pFormItem->GetChilds().rend().base(); iter++)
    {
        if ((*iter)->ISA(FmFilterItems))
            break;
    }
    sal_Int32 nInsertPos = iter.base() - pFormItem->GetChilds().rend().base();
    ::std::vector<FmFilterData*>::iterator i = pFormItem->GetChilds().begin() + nInsertPos;

    Insert(i, pFilterItems);

    // do we need a new row
    FmXFormController* pController = NULL;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(pFormItem->GetController(),::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xTunnel.is(), "FmFilterAdapter::InsertElements : xTunnel is invalid!");
    if(xTunnel.is())
    {
        pController = (FmXFormController*)xTunnel->getSomething(FmXFormController::getUnoTunnelImplementationId());
    }
    //  ::comphelper::getImplementation(pController, Reference<XUnoTunnel>(pFormItem->GetController(),UNO_QUERY));
    FmFilterRows& rRows = pController->getFilterRows();

    // determine the filter position
    if (nInsertPos >= rRows.size())
        rRows.push_back(FmFilterRow());
}

//------------------------------------------------------------------------
void FmFilterModel::Insert(const ::std::vector<FmFilterData*>::iterator& rPos, FmFilterData* pData)
{
    ::std::vector<FmFilterData*>& rItems = pData->GetParent()->GetChilds();
    sal_Int32 nPos = rPos == rItems.end() ? LIST_APPEND : rPos - rItems.begin();
    rItems.insert(rPos, pData);

    // UI benachrichtigen
    FmFilterInsertedHint aInsertedHint(pData, nPos);
    Broadcast( aInsertedHint );
}

//------------------------------------------------------------------------
void FmFilterModel::Remove(FmFilterData* pData)
{
    FmParentData* pParent = pData->GetParent();
    ::std::vector<FmFilterData*>& rItems = pParent->GetChilds();

    // erase the item from the model
    ::std::vector<FmFilterData*>::iterator i = find(rItems.begin(), rItems.end(), pData);
    DBG_ASSERT(i != rItems.end(), "FmFilterModel::Remove(): unknown Item");
    // position within the parent
    sal_Int32 nPos = i - rItems.begin();
    if (pData->ISA(FmFilterItems))
    {
        FmFormItem* pFormItem = (FmFormItem*)pParent;
        FmXFormController* pController = NULL;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(pFormItem->GetController(),::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xTunnel.is(), "FmFilterAdapter::InsertElements : xTunnel is invalid!");
        if(xTunnel.is())
        {
            pController = (FmXFormController*)xTunnel->getSomething(FmXFormController::getUnoTunnelImplementationId());
        }
        //  ::comphelper::getImplementation(pController, Reference<XUnoTunnel>(pFormItem->GetController(),UNO_QUERY));
        FmFilterRows& rRows = pController->getFilterRows();

        // how many entries do we have
        // it's the last row than we just empty it
        if (nPos == (rRows.size() - 1))
        {
            // remove all childs and stay current
            ::std::vector<FmFilterData*>& rChilds = ((FmFilterItems*)pData)->GetChilds();
            while (!rChilds.empty())
            {
                ::std::vector<FmFilterData*>::iterator j = rChilds.end();
                j--;

                // we stay on the level so delete each item explizit to clean the controls
                sal_Int32 nParentPos = j - rChilds.begin();
                // EmptyText removes the filter
                FmFilterItem* pFilterItem = PTR_CAST(FmFilterItem, *j);
                m_pAdapter->setText(nParentPos, pFilterItem, ::rtl::OUString());
                Remove(j, pFilterItem);
            }
        }
        else // delete the row
        {
            // if the row is on the current position we have to away from that position.
            // than we can delete it
            if (nPos == pFormItem->GetCurrentPosition())
            {
                ::std::vector<FmFilterData*>::iterator j = i;

                // give a new current postion
                if (nPos < (rRows.size() - 1))
                    // set it to the next row
                    ++j;
                else
                    // set it to the previous row
                    --j;

                // if necessary we have the formItem for the current controller
                // than we have to adjust the data displayed in the form
                pFormItem->SetCurrentPosition(j - rItems.begin());
                pController->setCurrentFilterPosition(j - rItems.begin());

                // Keep the view consistent and force and new painting
                FmFilterTextChangedHint aChangeHint(*j);
                Broadcast( aChangeHint );
            }

            // now delete the entry
            // before deleting we have to shift the current position of the form if necessary
            if (nPos < pFormItem->GetCurrentPosition())
            {
                pFormItem->SetCurrentPosition(pFormItem->GetCurrentPosition() - 1);
                pController->decrementCurrentFilterPosition();

                // is it the first row, than the nex row has to recieve a different name
                if (nPos == 0)
                {
                    // ensure that the text labels are consistent
                    rItems[1]->SetText(String(SVX_RES(RID_STR_FILTER_FILTER_FOR)));
                    FmFilterTextChangedHint aChangeHint(rItems[1]);
                    Broadcast( aChangeHint );
                }
            }

            // delete it
            rRows.erase(rRows.begin() + nPos);

            // and keep the controller consistent
            DBG_ASSERT(rRows.size() != 0, "wrong row size");

            // and remove it from the model
            Remove(i, pData);
        }
    }
    else // FormItems can not be deleted
    {
        FmFilterItem* pFilterItem = PTR_CAST(FmFilterItem, pData);

        // if its the last condition remove the parent
        if (rItems.size() == 1)
            Remove(pFilterItem->GetParent());
        else
        {
            // find the position of the father within his father
            ::std::vector<FmFilterData*>& rItems = pData->GetParent()->GetParent()->GetChilds();
            ::std::vector<FmFilterData*>::iterator j = find(rItems.begin(), rItems.end(), pFilterItem->GetParent());
            DBG_ASSERT(j != rItems.end(), "FmFilterModel::Remove(): unknown Item");
            sal_Int32 nParentPos = j - rItems.begin();

            // EmptyText removes the filter
            m_pAdapter->setText(nParentPos, pFilterItem, ::rtl::OUString());
            Remove(i, pData);
        }
    }
}

//------------------------------------------------------------------------
void FmFilterModel::Remove(const ::std::vector<FmFilterData*>::iterator& rPos, FmFilterData* pData)
{
    ::std::vector<FmFilterData*>& rItems = pData->GetParent()->GetChilds();
    rItems.erase(rPos);

    // UI benachrichtigen
    FmFilterRemovedHint aRemoveHint( pData );
    Broadcast( aRemoveHint );

    delete pData;
}

//------------------------------------------------------------------------
sal_Bool FmFilterModel::ValidateText(FmFilterItem* pItem, UniString& rText, UniString& rErrorMsg) const
{
    // check the input
    Reference< ::com::sun::star::beans::XPropertySet >  xField(m_pAdapter->getField(pItem->GetTextComponent()));
    Reference< ::com::sun::star::sdbc::XConnection >  xConnection(::dbtools::getConnection(Reference< ::com::sun::star::sdbc::XRowSet > (m_xController->getModel(), UNO_QUERY)));
    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xFormatSupplier = ::dbtools::getNumberFormats(xConnection, sal_True);
    Reference< ::com::sun::star::util::XNumberFormatter >  xFormatter(::comphelper::getProcessServiceFactory()
                        ->createInstance(FM_NUMBER_FORMATTER), UNO_QUERY);
        xFormatter->attachNumberFormatsSupplier(xFormatSupplier);

    String aErr,aTxt;
    SdbSqlParseNode* pParseNode = getSQLParser().PredicateTree(aErr, aTxt, xFormatter, Application::GetAppInternational(), xField);
    rErrorMsg = aErr;
    rText = aTxt;
    if (pParseNode)
    {
        String aPreparedText;
        pParseNode->ParseNodeToPredicateStr(aPreparedText,
                                   xConnection->getMetaData(),
                                   xFormatter,
                                   Application::GetAppInternational(),
                                   xField);
        rText = aPreparedText;
        delete pParseNode;
        return sal_True;
    }
    else
        return sal_False;
}

//------------------------------------------------------------------------
void FmFilterModel::Append(FmFilterItems* pItems, FmFilterItem* pFilterItem)
{
    ::std::vector<FmFilterData*>& rParentItems = pItems->GetParent()->GetChilds();
    ::std::vector<FmFilterData*>::iterator i = find(rParentItems.begin(), rParentItems.end(), pItems);
    sal_Int32 nParentPos = i - rParentItems.begin();

    Insert(pItems->GetChilds().end(), pFilterItem);
}

//------------------------------------------------------------------------
void FmFilterModel::SetText(FmFilterItem* pItem, const ::rtl::OUString& rText)
{
    ::std::vector<FmFilterData*>& rItems = pItem->GetParent()->GetParent()->GetChilds();
    ::std::vector<FmFilterData*>::iterator i = find(rItems.begin(), rItems.end(), pItem->GetParent());
    sal_Int32 nParentPos = i - rItems.begin();

    m_pAdapter->setText(nParentPos, pItem, rText);

    if (!rText)
        Remove(pItem);
    else
    {
        // Change the text
        pItem->SetText(rText);
        FmFilterTextChangedHint aChangeHint(pItem);
        Broadcast( aChangeHint );
    }
}

//------------------------------------------------------------------------
void FmFilterModel::SetCurrentItems(FmFilterItems* pCurrent)
{
    if (m_pCurrentItems == pCurrent)
        return;

    // search for the condition
    if (pCurrent)
    {
        FmFormItem* pFormItem = (FmFormItem*)pCurrent->GetParent();
        ::std::vector<FmFilterData*>& rItems = pFormItem->GetChilds();
        ::std::vector<FmFilterData*>::const_iterator i = find(rItems.begin(), rItems.end(), pCurrent);

        if (i != rItems.end())
        {
            // determine the filter position
            sal_Int32 nPos = i - rItems.begin();
            FmXFormController* pController = NULL;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(pFormItem->GetController(),::com::sun::star::uno::UNO_QUERY);
            DBG_ASSERT(xTunnel.is(), "FmFilterAdapter::InsertElements : xTunnel is invalid!");
            if(xTunnel.is())
            {
                pController = (FmXFormController*)xTunnel->getSomething(FmXFormController::getUnoTunnelImplementationId());
            }
            //  ::comphelper::getImplementation(pController, Reference<XUnoTunnel>(pFormItem->GetController(),UNO_QUERY));
            pController->setCurrentFilterPosition(nPos);
            pFormItem->SetCurrentPosition(nPos);

            if ((::com::sun::star::form::XFormController*)m_xController.get() != (::com::sun::star::form::XFormController*)pFormItem->GetController().get())
                // calls SetCurrentItems again
                SetCurrentController(pFormItem->GetController());
            else
                m_pCurrentItems = pCurrent;
        }
        else
            m_pCurrentItems = NULL;
    }
    else
        m_pCurrentItems = NULL;


    // UI benachrichtigen
    FmFilterCurrentChangedHint aHint;
    Broadcast( aHint );
}

//------------------------------------------------------------------------
void FmFilterModel::CheckIntegrity(FmParentData* pItem)
{
    // checks whether for each form there's one free level for input

    ::std::vector<FmFilterData*>& rItems = pItem->GetChilds();
    sal_Bool bAppendLevel = sal_False;

    for (::std::vector<FmFilterData*>::iterator i = rItems.begin();
         i != rItems.end(); i++)
    {
        FmFilterItems* pItems = PTR_CAST(FmFilterItems, *i);
        if (pItems)
        {
            bAppendLevel = !pItems->GetChilds().empty();
            continue;
        }

        FmFormItem* pFormItem = PTR_CAST(FmFormItem, *i);
        if (pFormItem)
        {
            CheckIntegrity(pFormItem);
            continue;
        }
    }
    if (bAppendLevel)
        AppendFilterItems((FmFormItem*)pItem);
}

//========================================================================
// class FmFilterItemsString
//========================================================================
class FmFilterItemsString : public SvLBoxString
{
public:
    FmFilterItemsString( SvLBoxEntry* pEntry, sal_uInt16 nFlags,    const XubString& rStr )
        :SvLBoxString(pEntry,nFlags,rStr){}

    virtual void Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
    virtual void InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData);
};

const int nxDBmp = 12;
//------------------------------------------------------------------------
void FmFilterItemsString::Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
{
    FmFilterItems* pRow = (FmFilterItems*)pEntry->GetUserData();
    FmFormItem* pForm = (FmFormItem*)pRow->GetParent();
    // current filter is significant painted
    if (pForm->GetChilds()[pForm->GetCurrentPosition()] == pRow)
    {
        Color aLineColor(rDev.GetLineColor());
        Rectangle aRect(rPos, GetSize(&rDev, pEntry ));
        Point aFirst(rPos.X(), aRect.Bottom() - 6);
        Point aSecond(aFirst.X() + 2, aFirst.Y() + 3);

        rDev.SetLineColor(rDev.GetTextColor());
        rDev.DrawLine(aFirst, aSecond);

        aFirst = aSecond;
        aFirst.X() += 1;
        aSecond.X() += 6;
        aSecond.Y() -= 5;

        rDev.DrawLine(aFirst, aSecond);
        rDev.SetLineColor( aLineColor );
    }
    rDev.DrawText( Point(rPos.X() + nxDBmp, rPos.Y()), GetText() );
}

//------------------------------------------------------------------------
void FmFilterItemsString::InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    Size aSize(pView->GetTextWidth(GetText()), pView->GetTextHeight());
    aSize.Width() += nxDBmp;
    pViewData->aSize = aSize;
}

//========================================================================
// class FmFilterString
//========================================================================
class FmFilterString : public SvLBoxString
{
    UniString m_aName;

public:
    FmFilterString( SvLBoxEntry* pEntry, sal_uInt16 nFlags, const XubString& rStr, const UniString& aName)
        :SvLBoxString(pEntry,nFlags,rStr)
        ,m_aName(aName)
    {
        m_aName.AppendAscii(": ");
    }

    virtual void Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
    virtual void InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData);
};

const int nxD = 4;

//------------------------------------------------------------------------
void FmFilterString::InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    Font aOldFont( pView->GetFont());
    Font aFont( aOldFont );
    aFont.SetWeight(WEIGHT_BOLD);
    pView->SetFont( aFont );

    Size aSize(pView->GetTextWidth(m_aName), pView->GetTextHeight());
    pView->SetFont( aOldFont );
    aSize.Width() += pView->GetTextWidth(GetText()) + nxD;
    pViewData->aSize = aSize;
}

//------------------------------------------------------------------------
void FmFilterString::Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
{
    Font aOldFont( rDev.GetFont());
    Font aFont( aOldFont );
    aFont.SetWeight(WEIGHT_BOLD);
    rDev.SetFont( aFont );

    Point aPos(rPos);
    rDev.DrawText( aPos, m_aName );

    // position for the second text
    aPos.X() += rDev.GetTextWidth(m_aName) + nxD;
    rDev.SetFont( aOldFont );
    rDev.DrawText( aPos, GetText() );
}

//========================================================================
// class FmFilterNavigator
//========================================================================
FmFilterNavigator::FmFilterNavigator( Window* pParent )
                  :SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_BORDER|WB_HASBUTTONSATROOT )
{
    SetHelpId( HID_FILTER_NAVIGATOR );

    ImageList aNavigatorImages(SVX_RES(RID_SVXIMGLIST_FMEXPL));
    Image aCollapsedNodeImg = aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE );
    Image aExpandedNodeImg = aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE );
    SetNodeBitmaps( aCollapsedNodeImg, aExpandedNodeImg );

    m_pModel = new FmFilterModel();
    StartListening( *m_pModel );

    EnableInplaceEditing( sal_True );
    SetSelectionMode(MULTIPLE_SELECTION);

    EnableDrop();
    SetDragDropMode(0xFFFF);

    m_aSelectTimer.SetTimeoutHdl(LINK(this, FmFilterNavigator, OnSelect));
    m_aDropActionTimer.SetTimeoutHdl(LINK(this, FmFilterNavigator, OnDropActionTimer));
}

//------------------------------------------------------------------------
FmFilterNavigator::~FmFilterNavigator()
{
    if (m_aSelectTimer.IsActive())
        m_aSelectTimer.Stop();

    EndListening( *m_pModel );
    delete m_pModel;
}

//------------------------------------------------------------------------
void FmFilterNavigator::Clear()
{
    if (m_aSelectTimer.IsActive())
        m_aSelectTimer.Stop();

    m_pModel->Clear();
}

//------------------------------------------------------------------------
void FmFilterNavigator::Update(const Reference< ::com::sun::star::container::XIndexAccess > & xControllers, const Reference< ::com::sun::star::form::XFormController > & xCurrent)
{
    if (xCurrent == m_pModel->GetCurrentController())
        return;

    m_pModel->Update(xControllers, xCurrent);

    // expand the filters for the current controller
    SvLBoxEntry* pEntry = FindEntry(m_pModel->GetCurrentForm());
    if (pEntry && !IsExpanded(pEntry))
    {
        SelectAll(sal_False);

        if (!IsExpanded(pEntry))
            Expand(pEntry);

        pEntry = FindEntry(m_pModel->GetCurrentItems());
        if (pEntry)
        {
            if (!IsExpanded(pEntry))
                Expand(pEntry);
            Select(pEntry, sal_True);
        }
    }
}

//------------------------------------------------------------------------
sal_Bool FmFilterNavigator::EditingEntry( SvLBoxEntry* pEntry, Selection& rSelection )
{
    if (!SvTreeListBox::EditingEntry( pEntry, rSelection ))
        return sal_False;

    return pEntry && ((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem);
}

//------------------------------------------------------------------------
sal_Bool FmFilterNavigator::EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText )
{
    if (EditingCanceled())
        return sal_True;

    DBG_ASSERT(((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem),
                    "FmFilterNavigator::EditedEntry() wrong entry");

    UniString aText(rNewText);
    aText.EraseTrailingChars();
    aText.EraseLeadingChars();
    if (aText.Len() != 0)
    {
        // deleting the entry asynchron
        sal_uInt32 nEvent;
        PostUserEvent(nEvent, LINK(this, FmFilterNavigator, OnRemove), pEntry);
    }
    else
    {
        UniString aErrorMsg;

        if (m_pModel->ValidateText((FmFilterItem*)pEntry->GetUserData(), aText, aErrorMsg))
        {
            GrabFocus();
            m_pModel->SetText((FmFilterItem*)pEntry->GetUserData(), aText);
            SetCursor(pEntry, sal_True);
            SetEntryText(pEntry, aText);

            // settting the text asynchron
            sal_uInt32 nEvent;
            PostUserEvent(nEvent, LINK(this, FmFilterNavigator, OnEdited), pEntry);
        }
        else
        {
            // display the error and return sal_False
            ::vos::OGuard aGuard(Application::GetSolarMutex());
            ::rtl::OUString aTitle(SVX_RES(RID_STR_SYNTAXERROR));
            SvxDBMsgBox aDlg(this, aTitle, aErrorMsg, WB_OK | WB_DEF_OK,
                             SvxDBMsgBox::Info);
            aDlg.Execute();
            return sal_False;
        }
    }
    return sal_True;
}

//------------------------------------------------------------------------
IMPL_LINK( FmFilterNavigator, OnEdited, SvLBoxEntry*, pEntry )
{
    // invalidate the entry to see the correct text
    SetEntryText( pEntry, ((FmFilterItem*)pEntry->GetUserData())->GetText());
    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK( FmFilterNavigator, OnRemove, SvLBoxEntry*, pEntry )
{
    // now remove the entry
    m_pModel->Remove((FmFilterData*) pEntry->GetUserData());
    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK( FmFilterNavigator, OnSelect, void*, EMPTYTAG )
{
    // now activate the controller to be consistent
    sal_Bool bHadFocus = HasFocus();
    Reference< ::com::sun::star::awt::XWindow >  xWindow(m_pModel->GetCurrentController(),  UNO_QUERY);
    if (xWindow.is())
        xWindow->setFocus();
    else
        m_pModel->GetCurrentController()->activateFirst();

    // now grab the focus again
    if (bHadFocus)
        GrabFocus();
    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK( FmFilterNavigator, OnDropActionTimer, void*, EMPTYARG )
{
    if (--m_aTimerCounter > 0)
        return 0L;

    if (m_aDropActionType == DA_EXPANDNODE)
    {
        SvLBoxEntry* pToExpand = GetEntry(m_aTimerTriggered);
        if (pToExpand && (GetChildCount(pToExpand) > 0) &&  !IsExpanded(pToExpand))
            // tja, eigentlich muesste ich noch testen, ob die Node nicht schon expandiert ist, aber ich
            // habe dazu weder in den Basisklassen noch im Model eine Methode gefunden ...
            // aber ich denke, die BK sollte es auch so vertragen
            Expand(pToExpand);

        // nach dem Expand habe ich im Gegensatz zum Scrollen natuerlich nix mehr zu tun
        m_aDropActionTimer.Stop();
    }
    else
    {
        switch (m_aDropActionType)
        {
            case DA_SCROLLUP :
                ScrollOutputArea(1);
                break;
            case DA_SCROLLDOWN :
                ScrollOutputArea(-1);
                break;
        }
        m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
    }
    return 0L;
}


//------------------------------------------------------------------------
sal_Bool FmFilterNavigator::QueryDrop( DropEvent& rDEvt )
{
    Point aDropPos = rDEvt.GetPosPixel();

    // kuemmern wir uns erst mal um moeglich DropActions (Scrollen und Aufklappen)
    if (rDEvt.IsLeaveWindow())
    {
        if (m_aDropActionTimer.IsActive())
            m_aDropActionTimer.Stop();
    }
    else
    {
        sal_Bool bNeedTrigger = sal_False;
        // auf dem ersten Eintrag ?
        if ((aDropPos.Y() >= 0) && (aDropPos.Y() < GetEntryHeight()))
        {
            m_aDropActionType = DA_SCROLLUP;
            bNeedTrigger = sal_True;
        }
        else
        {
            // auf dem letzten (bzw. in dem Bereich, den ein Eintrag einnehmen wuerde, wenn er unten genau buendig
            // abschliessen wuerde) ?
            if ((aDropPos.Y() < GetSizePixel().Height()) && (aDropPos.Y() >= GetSizePixel().Height() - GetEntryHeight()))
            {
                m_aDropActionType = DA_SCROLLDOWN;
                bNeedTrigger = sal_True;
            }
            else
            {   // auf einem Entry mit Childs, der nicht aufgeklappt ist ?
                SvLBoxEntry* pDropppedOn = GetEntry(aDropPos);
                if (pDropppedOn && (GetChildCount(pDropppedOn) > 0) && !IsExpanded(pDropppedOn))
                {
                    // -> aufklappen
                    m_aDropActionType = DA_EXPANDNODE;
                    bNeedTrigger = sal_True;
                }
            }
        }
        if (bNeedTrigger && (m_aTimerTriggered != aDropPos))
        {
            // neu anfangen zu zaehlen
            m_aTimerCounter = DROP_ACTION_TIMER_INITIAL_TICKS;
            // die Pos merken, da ich auch QueryDrops bekomme, wenn sich die Maus gar nicht bewegt hat
            m_aTimerTriggered = aDropPos;
            // und den Timer los
            if (!m_aDropActionTimer.IsActive()) // gibt es den Timer schon ?
            {
                m_aDropActionTimer.SetTimeout(DROP_ACTION_TIMER_TICK_BASE);
                m_aDropActionTimer.Start();
            }
        }
        else if (!bNeedTrigger)
            m_aDropActionTimer.Stop();
    }


    // Hat das Object das richtige Format?
    SvDataObjectRef xDataObj = SvDataObject::PasteDragServer( rDEvt );

    const SvDataTypeList& rTypeList = xDataObj->GetTypeList();
    if (!rTypeList.Get(Exchange::RegisterFormatName(SVX_FM_FILTER_FIELDS)))
        return sal_False;

    SvxFmFilterExchRef xExch = (SvxFmFilterExch*)&xDataObj;
    // do we conain the formitem?
    if (!FindEntry(xExch->GetFormItem()))
        return sal_False;

    SvLBoxEntry* pDropTarget = GetEntry(aDropPos);
    if (!pDropTarget)
        return sal_False;

    FmFilterData* pData = (FmFilterData*)pDropTarget->GetUserData();
    FmFormItem* pForm = NULL;
    if (pData->ISA(FmFilterItem))
    {
        pForm = PTR_CAST(FmFormItem,pData->GetParent()->GetParent());
        if (pForm != xExch->GetFormItem())
            return sal_False;
    }
    else if (pData->ISA(FmFilterItems))
    {
        pForm = PTR_CAST(FmFormItem,pData->GetParent());
        if (pForm != xExch->GetFormItem())
            return sal_False;
    }
    else
        return sal_False;

    return sal_True;
}

//------------------------------------------------------------------------
sal_Bool FmFilterNavigator::Drop( const DropEvent& rDEvt )
{
    // ware schlecht, wenn nach dem Droppen noch gescrollt wird ...
    if (m_aDropActionTimer.IsActive())
        m_aDropActionTimer.Stop();

    // Format-Ueberpruefung
    SvDataObjectRef xDataObj = SvDataObject::PasteDragServer( rDEvt );
    SvxFmFilterExchRef xExch = (SvxFmFilterExch*)&xDataObj;

    // das Ziel des Drop sowie einige Daten darueber
    Point aDropPos = rDEvt.GetPosPixel();
    SvLBoxEntry* pDropTarget = GetEntry( aDropPos );
    if (!pDropTarget)
        return sal_False;

    // search the container where to add the items
    FmFilterData*   pData = (FmFilterData*)pDropTarget->GetUserData();
    FmFilterItems*  pTargetItems = pData->ISA(FmFilterItems) ? (FmFilterItems*)pData
                                                       : (FmFilterItems*)((FmFilterItem*)pData)->GetParent();
    SelectAll(sal_False);
    SvLBoxEntry* pEntry = FindEntry(pTargetItems);
    Select(pEntry, sal_True);
    SetCurEntry(pEntry);

    sal_Bool bCopy = rDEvt.GetAction() == DROP_COPY;
    ::std::vector<FmFilterItem*> aItemList = xExch->GetDraggedEntries();

    for (::std::vector<FmFilterItem*>::const_iterator i = aItemList.begin(); i != aItemList.end(); i++)
    {
        if ((*i)->GetParent() == pTargetItems)
            continue;
        else
        {
            FmFilterItem* pFilterItem = pTargetItems->Find((*i)->GetTextComponent());
            String aText = (*i)->GetText();
            if (!pFilterItem)
            {
                pFilterItem = new FmFilterItem(pTargetItems, (*i)->GetFieldName(), aText, (*i)->GetTextComponent());
                m_pModel->Append(pTargetItems, pFilterItem);
            }

            if (!bCopy)
                m_pModel->Remove(*i);

            // now set the text for the new dragged item
            m_pModel->SetText(pFilterItem, aText);
        }
    }
    m_pModel->CheckIntegrity((FmFormItem*)pTargetItems->GetParent());
    return sal_True;
}

//------------------------------------------------------------------------
void FmFilterNavigator::InitEntry(SvLBoxEntry* pEntry,
                                  const XubString& rStr,
                                  const Image& rImg1,
                                  const Image& rImg2)
{
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2 );
    SvLBoxString* pString = NULL;

    if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem))
        pString = new FmFilterString(pEntry, 0, rStr, ((FmFilterItem*)pEntry->GetUserData())->GetFieldName());
    else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItems))
        pString = new FmFilterItemsString(pEntry, 0, rStr );

    if (pString)
        pEntry->ReplaceItem( pString, 1 );
}

//------------------------------------------------------------------------
sal_Bool FmFilterNavigator::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    if (bSelect == IsSelected(pEntry))  // das passiert manchmal, ich glaube, die Basisklasse geht zu sehr auf Nummer sicher ;)
        return sal_True;

    if (SvTreeListBox::Select(pEntry, bSelect))
    {
        if (bSelect)
        {
            sal_Bool bSyncController = m_aSelectTimer.IsActive();
            if (bSyncController)
                m_aSelectTimer.Stop();

            FmFormItem* pFormItem = NULL;
            if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem))
                pFormItem = (FmFormItem*)((FmFilterItem*)pEntry->GetUserData())->GetParent()->GetParent();
            else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItems))
                pFormItem = (FmFormItem*)((FmFilterItem*)pEntry->GetUserData())->GetParent()->GetParent();
            else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFormItem))
                pFormItem = (FmFormItem*)pEntry->GetUserData();

            if (pFormItem)
            {
                // will the controller be exchanged?
                bSyncController = bSyncController || (::com::sun::star::form::XFormController*)m_pModel->GetCurrentController().get() != (::com::sun::star::form::XFormController*)pFormItem->GetController().get();
                if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem))
                    m_pModel->SetCurrentItems((FmFilterItems*)((FmFilterItem*)pEntry->GetUserData())->GetParent());
                else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItems))
                    m_pModel->SetCurrentItems((FmFilterItems*)pEntry->GetUserData());
                else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFormItem))
                    m_pModel->SetCurrentController(((FmFormItem*)pEntry->GetUserData())->GetController());
            }
            if (bSyncController)
            {
                m_aSelectTimer.SetTimeout(SYNC_DELAY);
                m_aSelectTimer.Start();
            }
        }
        return sal_True;
    }
    else
        return sal_False;
}

//------------------------------------------------------------------------
void FmFilterNavigator::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA(FmFilterInsertedHint))
    {
        FmFilterInsertedHint* pHint = (FmFilterInsertedHint*)&rHint;
        Insert(pHint->GetData(), pHint->GetPos());
    }
    else if( rHint.ISA(FmFilterClearedHint) )
    {
        SvTreeListBox::Clear();
    }
    else if( rHint.ISA(FmFilterRemovedHint) )
    {
        FmFilterRemovedHint* pHint = (FmFilterRemovedHint*)&rHint;
        Remove(pHint->GetData());
    }
    else if( rHint.ISA(FmFilterTextChangedHint) )
    {
        FmFilterTextChangedHint* pHint = (FmFilterTextChangedHint*)&rHint;
        SvLBoxEntry* pEntry = FindEntry(pHint->GetData());
        if (pEntry)
            SetEntryText( pEntry, pHint->GetData()->GetText());
    }
    else if( rHint.ISA(FmFilterCurrentChangedHint) )
    {
        // invalidate the entries
        for (SvLBoxEntry* pEntry = First(); pEntry != NULL;
             pEntry = Next(pEntry))
            GetModel()->InvalidateEntry( pEntry );
    }
}

//------------------------------------------------------------------------
SvLBoxEntry* FmFilterNavigator::FindEntry(FmFilterData* pItem)
{
    SvLBoxEntry* pEntry = NULL;
    if (pItem)
    {
        for (pEntry = First(); pEntry != NULL; pEntry = Next( pEntry ))
        {
            FmFilterData* pEntryItem = (FmFilterData*)pEntry->GetUserData();
            if (pEntryItem == pItem)
                break;
        }
    }
    return pEntry;
}

//------------------------------------------------------------------------
void FmFilterNavigator::Insert(FmFilterData* pItem, sal_Int32 nPos)
{
    FmParentData* pParent = pItem->GetParent() ? pItem->GetParent() : GetFilterModel();

    // insert the item
    SvLBoxEntry* pParentEntry = FindEntry(pParent);
    InsertEntry(pItem->GetText(), pItem->GetImage(), pItem->GetImage(), pParentEntry, sal_False, nPos, pItem );
}

//------------------------------------------------------------------------
void FmFilterNavigator::Remove(FmFilterData* pItem)
{
    // der Entry zu den Daten
    SvLBoxEntry* pEntry = FindEntry(pItem);
    if (pEntry)
        GetModel()->Remove( pEntry );
}

//------------------------------------------------------------------------------
void FmFilterNavigator::Command( const CommandEvent& rEvt )
{
    sal_Bool bHandled = sal_False;
    switch (rEvt.GetCommand())
    {
        case COMMAND_STARTDRAG:
        {
            EndSelection();
            Pointer aMovePtr( POINTER_MOVEDATA ),
                    aCopyPtr( POINTER_COPYDATA ),
                    aLinkPtr( POINTER_COPYDATA );

            // be sure that the data is only used within a only one form!
            ::std::vector<FmFilterItem*> aSelectList;
            FmFormItem* pFirstItem = NULL;
            SvLBoxEntry* pCurEntry = GetCurEntry();

            bHandled = sal_True;
            for (SvLBoxEntry* pEntry = FirstSelected();
                 bHandled && pEntry != NULL;
                 pEntry = NextSelected(pEntry))
            {
                FmFilterItem* pFilter = PTR_CAST(FmFilterItem, (FmFilterData*)pEntry->GetUserData());
                if (pFilter)
                {
                    FmFormItem* pForm = PTR_CAST(FmFormItem,pFilter->GetParent()->GetParent());
                    if (!pForm)
                        bHandled = sal_False;
                    else if (!pFirstItem)
                        pFirstItem = pForm;
                    else if (pFirstItem != pForm)
                        bHandled = sal_False;

                    if (bHandled)
                        aSelectList.push_back(pFilter);
                }
            }
            if (!bHandled || aSelectList.empty())
                break;

            SvxFmFilterExch* pExch = new SvxFmFilterExch(pFirstItem, aSelectList);
            short nDragResult = pExch->ExecuteDrag( this, aMovePtr, aCopyPtr, aLinkPtr, DRAG_MOVEABLE | DRAG_COPYABLE );
            if (nDragResult == DROP_CANCEL)
            {
                SetCursor(pCurEntry, sal_True);
                MakeVisible(pCurEntry);
            }
            bHandled = sal_True;
            break;
        }
        case COMMAND_CONTEXTMENU:
        {
            // die Stelle, an der geklickt wurde
            Point aWhere;
            SvLBoxEntry* pClicked = NULL;
            if (rEvt.IsMouseEvent())
            {
                aWhere = rEvt.GetMousePosPixel();
                pClicked = GetEntry(aWhere);
                if (pClicked == NULL)
                    break;

                if (!IsSelected(pClicked))
                {
                    SelectAll(sal_False);
                    Select(pClicked, sal_True);
                    SetCurEntry(pClicked);
                }
            }
            else
            {
                pClicked = GetCurEntry();
                if (!pClicked)
                    break;
                aWhere = GetEntryPos(pClicked);
            }

            ::std::vector<FmFilterData*> aSelectList;
            for (SvLBoxEntry* pEntry = FirstSelected();
                 pEntry != NULL;
                 pEntry = NextSelected(pEntry))
            {
                // don't delete forms
                FmFormItem* pForm = PTR_CAST(FmFormItem, (FmFilterData*)pEntry->GetUserData());
                if (!pForm)
                    aSelectList.push_back((FmFilterData*)pEntry->GetUserData());
            }
            if (aSelectList.size() == 1)
            {
                // don't delete the only empty row of a form
                FmFilterItems* pFilterItems = PTR_CAST(FmFilterItems, aSelectList[0]);
                if (pFilterItems && pFilterItems->GetChilds().empty()
                    && pFilterItems->GetParent()->GetChilds().size() == 1)
                    aSelectList.clear();
            }

            PopupMenu aContextMenu(SVX_RES(RID_FM_FILTER_MENU));

            // every condition could be deleted except the first one if its the only one
            aContextMenu.EnableItem( SID_FM_DELETE, !aSelectList.empty() );

            //
            sal_Bool bEdit = PTR_CAST(FmFilterItem, (FmFilterData*)pClicked->GetUserData()) != NULL &&
                IsSelected(pClicked) && GetSelectionCount() == 1;

            aContextMenu.EnableItem( SID_FM_FILTER_EDIT,
                bEdit );
            aContextMenu.EnableItem( SID_FM_FILTER_IS_NULL,
                bEdit );
            aContextMenu.EnableItem( SID_FM_FILTER_IS_NOT_NULL,
                bEdit );

            aContextMenu.RemoveDisabledEntries(sal_True, sal_True);
            sal_uInt16 nSlotId = aContextMenu.Execute( this, aWhere );
            switch( nSlotId )
            {
                case SID_FM_FILTER_EDIT:
                {
                    EditEntry( pClicked );
                }   break;
                case SID_FM_FILTER_IS_NULL:
                case SID_FM_FILTER_IS_NOT_NULL:
                {
                    UniString aErrorMsg;
                    UniString aText;
                    if (nSlotId == SID_FM_FILTER_IS_NULL)
                        aText.AssignAscii("IS NULL");
                    else
                        aText.AssignAscii("IS NOT NULL");

                    m_pModel->ValidateText((FmFilterItem*)pClicked->GetUserData(),
                                            aText, aErrorMsg);
                    m_pModel->SetText((FmFilterItem*)pClicked->GetUserData(), aText);
                }   break;
                case SID_FM_DELETE:
                {
                    DeleteSelection();
                }   break;
            }
            bHandled = sal_True;
        } break;
    }

    if (!bHandled)
        SvTreeListBox::Command( rEvt );
}

//------------------------------------------------------------------------
void FmFilterNavigator::KeyInput(const KeyEvent& rKEvt)
{
    // delete ?
    if (rKEvt.GetKeyCode().GetCode() == KEY_DELETE && !rKEvt.GetKeyCode().GetModifier())
    {
        if (!IsSelected(First()) || GetEntryCount() > 1)
            DeleteSelection();
        return;
    }
    SvTreeListBox::KeyInput(rKEvt);
}

//------------------------------------------------------------------------------
void FmFilterNavigator::DeleteSelection()
{
    // to avoid the deletion of an entry twice (e.g. deletion of a parent and afterward
    // the deletion of it's child, i have to shrink the selecton list
    ::std::vector<SvLBoxEntry*> aEntryList;
    for (SvLBoxEntry* pEntry = FirstSelected();
         pEntry != NULL;
         pEntry = NextSelected(pEntry))
    {
        FmFilterItem* pFilterItem = PTR_CAST(FmFilterItem, (FmFilterData*)pEntry->GetUserData());
        if (pFilterItem && IsSelected(GetParent(pEntry)))
            continue;

        FmFormItem* pForm = PTR_CAST(FmFormItem, (FmFilterData*)pEntry->GetUserData());
        if (!pForm)
            aEntryList.push_back(pEntry);
    }

    // Remove the selection
    SelectAll(FALSE);

    for (::std::vector<SvLBoxEntry*>::reverse_iterator i = aEntryList.rbegin();
        // link problems with operator ==
        i.base() != aEntryList.rend().base(); i++)
    {
        m_pModel->Remove((FmFilterData*)(*i)->GetUserData());
    }

    // now check if we need to insert new items
    m_pModel->CheckIntegrity(m_pModel);
}

//========================================================================
// class FmFilterNavigatorWin
//========================================================================
FmFilterNavigatorWin::FmFilterNavigatorWin( SfxBindings *pBindings, SfxChildWindow *pMgr,
                              Window* pParent )
                     :SfxDockingWindow( pBindings, pMgr, pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_ROLLABLE|WB_3DLOOK|WB_DOCKABLE) )
                     ,SfxControllerItem( SID_FM_FILTER_NAVIGATOR_CONTROL, *pBindings )
{
    SetHelpId( HID_FILTER_NAVIGATOR_WIN );

    m_pNavigator = new FmFilterNavigator( this );
    m_pNavigator->Show();
    SetText( SVX_RES(RID_STR_FILTER_NAVIGATOR) );
    SetSizePixel( Size(200,200) );
}

//------------------------------------------------------------------------
FmFilterNavigatorWin::~FmFilterNavigatorWin()
{
    delete m_pNavigator;
}

//-----------------------------------------------------------------------
void FmFilterNavigatorWin::Update(FmFormShell* pFormShell)
{
    if (!pFormShell)
        m_pNavigator->Update(Reference< ::com::sun::star::container::XIndexAccess > (), Reference< ::com::sun::star::form::XFormController > ());
    else
    {
        Reference< ::com::sun::star::form::XFormController >  xController(pFormShell->GetImpl()->getActiveInternalController());
        Reference< ::com::sun::star::container::XIndexAccess >  xContainer;
        if (xController.is())
        {
            Reference< ::com::sun::star::container::XChild >  xChild(xController, UNO_QUERY);
            for (Reference< XInterface >  xParent(xChild->getParent());
                 xParent.is();
                 xParent = xChild.is() ? xChild->getParent() : Reference< XInterface > ())
            {
                xContainer = Reference< ::com::sun::star::container::XIndexAccess > (xParent, UNO_QUERY);
                xChild = Reference< ::com::sun::star::container::XChild > (xParent, UNO_QUERY);
            }
        }
        m_pNavigator->Update(xContainer, xController);
    }
}

//-----------------------------------------------------------------------
void FmFilterNavigatorWin::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if( !pState  || SID_FM_FILTER_NAVIGATOR_CONTROL != nSID )
        return;

    if( eState >= SFX_ITEM_AVAILABLE )
    {
        FmFormShell* pShell = PTR_CAST( FmFormShell,((SfxObjectItem*)pState)->GetShell() );
        Update( pShell );
    }
    else
        Update( NULL );
}

//-----------------------------------------------------------------------
sal_Bool FmFilterNavigatorWin::Close()
{
    Update( NULL );
    return SfxDockingWindow::Close();
}

//-----------------------------------------------------------------------
void FmFilterNavigatorWin::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = sal_False;
}

//-----------------------------------------------------------------------
Size FmFilterNavigatorWin::CalcDockingSize( SfxChildAlignment eAlign )
{
    Size aSize = SfxDockingWindow::CalcDockingSize( eAlign );

    switch( eAlign )
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_BOTTOM:
            return Size();
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
            break;
    }

    return aSize;
}

//-----------------------------------------------------------------------
SfxChildAlignment FmFilterNavigatorWin::CheckAlignment( SfxChildAlignment eActAlign, SfxChildAlignment eAlign )
{
    switch (eAlign)
    {
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_NOALIGNMENT:
            return (eAlign);
    }

    return (eActAlign);
}

//------------------------------------------------------------------------
void FmFilterNavigatorWin::Resize()
{
    SfxDockingWindow::Resize();

    Size aLogOutputSize = PixelToLogic( GetOutputSizePixel(), MAP_APPFONT );
    Size aLogExplSize = aLogOutputSize;
    aLogExplSize.Width() -= 6;
    aLogExplSize.Height() -= 6;

    Point aExplPos = LogicToPixel( Point(3,3), MAP_APPFONT );
    Size aExplSize = LogicToPixel( aLogExplSize, MAP_APPFONT );

    m_pNavigator->SetPosSizePixel( aExplPos, aExplSize );
}


//========================================================================
// class FmFilterNavigatorWinMgr
//========================================================================
SFX_IMPL_DOCKINGWINDOW( FmFilterNavigatorWinMgr, SID_FM_FILTER_NAVIGATOR )

//-----------------------------------------------------------------------
FmFilterNavigatorWinMgr::FmFilterNavigatorWinMgr( Window *pParent, sal_uInt16 nId,
                                    SfxBindings *pBindings, SfxChildWinInfo* pInfo )
                 :SfxChildWindow( pParent, nId )
{
    pWindow = new FmFilterNavigatorWin( pBindings, this, pParent );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ((SfxDockingWindow*)pWindow)->Initialize( pInfo );
}

