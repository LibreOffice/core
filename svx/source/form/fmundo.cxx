/*************************************************************************
 *
 *  $RCSfile: fmundo.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-24 07:01:26 $
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

#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif

#include <com/sun/star/beans/PropertyAttribute.hpp>
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif

#ifndef _FM_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

#ifndef _SVX_FMPAGE_HXX //autogen
#include <fmpage.hxx>
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif

#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif

#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#ifndef _SB_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif

#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
#endif

#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif

#ifndef _OSL_MUTEX_HXX_ //autogen
#include <osl/mutex.hxx>
#endif

#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _SVX_FMSTL_HXX
#include "fmstl.hxx"
#endif

using namespace ::com::sun::star::uno;
//------------------------------------------------------------------------------
// some helper structs for caching property infos
//------------------------------------------------------------------------------
struct PropertyInfo
{
    BOOL    bIsTransientOrReadOnly      : 1;    // the property is transient or read-only, thus we need no undo action for it
    BOOL    bIsControlSourceProperty    : 1;    // the property is the special control source property, thus it may be handled
                                                // as if it's transient or persistent
};

struct PropertySetInfo
{
    DECLARE_STL_USTRINGACCESS_MAP(PropertyInfo, AllProperties);

    AllProperties   aProps;                 // all properties of this set which we know so far
    BOOL            bHasEmptyControlSource; // sal_True -> the set has a DataField property, and the current value is an empty string
                                            // sal_False -> the set has _no_ such property or it's value isn't empty
};

BOOL operator < (const Reference< ::com::sun::star::beans::XPropertySet >& lhs,
                 const Reference< ::com::sun::star::beans::XPropertySet >& rhs)
{
    return lhs.get() < rhs.get();
}

DECLARE_STL_STDKEY_MAP(Reference< ::com::sun::star::beans::XPropertySet >, PropertySetInfo, PropertySetInfoCache);

//------------------------------------------------------------------------------

XubString static_STR_UNDO_PROPERTY;
//------------------------------------------------------------------------------
DBG_NAME(FmXUndoEnvironment);
//------------------------------------------------------------------------------
FmXUndoEnvironment::FmXUndoEnvironment(FmFormModel& _rModel)
                   :rModel(_rModel)
                   ,nLocks(0)
                   ,bReadOnly(sal_False)
                   ,m_pPropertySetCache(NULL)
{
    DBG_CTOR(FmXUndoEnvironment,NULL);
}

//------------------------------------------------------------------------------
FmXUndoEnvironment::~FmXUndoEnvironment()
{
    DBG_DTOR(FmXUndoEnvironment,NULL);
    if (m_pPropertySetCache)
        delete static_cast<PropertySetInfoCache*>(m_pPropertySetCache);
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::Clear()
{
    Lock();
    sal_uInt16 nCount = rModel.GetPageCount();
    sal_uInt16 i;
    for (i = 0; i < nCount; i++)
    {
        Reference< XInterface >  xInt(((FmFormPage*)rModel.GetPage(i))->GetForms());
        RemoveElement(xInt);
    }

    nCount = rModel.GetMasterPageCount();
    for (i = 0; i < nCount; i++)
    {
        Reference< XInterface >  xInt(((FmFormPage*)rModel.GetMasterPage(i))->GetForms());
        RemoveElement(xInt);
    }
    UnLock();

    EndListening(*rModel.GetObjectShell());
    if (IsListening(rModel))
        EndListening(rModel);
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::ModeChanged()
{
    if (bReadOnly != (rModel.GetObjectShell()->IsReadOnly() || rModel.GetObjectShell()->IsReadOnlyUI()))
    {
        bReadOnly = !bReadOnly;

        sal_uInt16 nCount = rModel.GetPageCount();
        sal_uInt16 i;
        for (i = 0; i < nCount; i++)
        {
            Reference< XInterface >  xInt(((FmFormPage*)rModel.GetPage(i))->GetForms());
            AlterPropertyListening(xInt);
        }

        nCount = rModel.GetMasterPageCount();
        for (i = 0; i < nCount; i++)
        {
            Reference< XInterface >  xInt(((FmFormPage*)rModel.GetMasterPage(i))->GetForms());
            AlterPropertyListening(xInt);
        }

        if (!bReadOnly)
            StartListening(rModel);
        else
            EndListening(rModel);
    }
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA(SdrHint))
    {
        SdrHint* pSdrHint = (SdrHint*)&rHint;
        switch( pSdrHint->GetKind() )
        {
            case HINT_OBJINSERTED:
            {
                SdrObject* pSdrObj = (SdrObject*)pSdrHint->GetObject();
                Inserted( pSdrObj );
            }   break;
            case HINT_OBJREMOVED:
            {
                SdrObject* pSdrObj = (SdrObject*)pSdrHint->GetObject();
                Removed( pSdrObj );
            }
            break;
        }
    }
    else if (rHint.ISA(SfxSimpleHint))
    {
        switch ( ((SfxSimpleHint&)rHint).GetId() )
        {
            case SFX_HINT_DYING:
                Clear();
                break;
            case SFX_HINT_MODECHANGED:
                ModeChanged();
                break;
        }
    }
    else if (rHint.ISA(SfxEventHint))
    {
        switch (((SfxEventHint&)rHint).GetEventId())
        {
        case SFX_EVENT_CREATEDOC:
            case SFX_EVENT_OPENDOC:
                ModeChanged();
                break;
        }
    }

}

//------------------------------------------------------------------
void FmXUndoEnvironment::Inserted(SdrObject* pObj)
{
    if (bReadOnly)
        return;

    if (pObj->GetObjInventor() == FmFormInventor)
    {
        FmFormObj* pFormObj = PTR_CAST(FmFormObj, pObj);
        Inserted(pFormObj);
    }
    else if (pObj->IsGroupObject())
    {
        SdrObjListIter aIter(*pObj->GetSubList());
        while (aIter.IsMore())
        {
            SdrObject* pObj = aIter.Next();
            Inserted(pObj);
        }
    }
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::Inserted(FmFormObj* pObj)
{
    // ist das Control noch einer ::com::sun::star::form::Form zugeordnet
    Reference< XInterface >  xModel = pObj->GetUnoControlModel();
    Reference< ::com::sun::star::form::XFormComponent >  xContent(xModel, UNO_QUERY);
    if (xContent.is() && pObj->GetPage())
    {
        // Komponente gehoert noch keiner ::com::sun::star::form::Form an
        if (!xContent->getParent().is())
        {
            // Einfuegen in den Parent falls noetig
            Reference< ::com::sun::star::container::XIndexContainer >  xParent = pObj->GetParent();
            // Suchen des ::com::sun::star::form::Form in der aktuellen Page
            Reference< ::com::sun::star::container::XIndexContainer >  xForm;
            Reference< XInterface >  xIface(xParent, UNO_QUERY);
            Reference< ::com::sun::star::container::XIndexAccess >  xForms(((FmFormPage*)pObj->GetPage())->GetForms(), UNO_QUERY);;

            if (searchElement(xForms, xIface))
                xForm = xParent;
            else
            {
                Reference< ::com::sun::star::form::XForm >  xTemp = ((FmFormPage*)pObj->GetPage())->GetImpl()->SetDefaults(xContent);
                xForm = Reference< ::com::sun::star::container::XIndexContainer > (xTemp, UNO_QUERY);
            }

            // Position des Elements
            sal_Int32 nPos = xForm->getCount();
            if ((::com::sun::star::container::XIndexContainer*)xForm.get() == (::com::sun::star::container::XIndexContainer*)xParent.get())
            {
                if (nPos > pObj->GetPos())
                    nPos = xForm->getCount();
            }

            xForm->insertByIndex(nPos, makeAny(xContent));

            Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xForm, UNO_QUERY);
            if (xManager.is())
                xManager->registerScriptEvents(nPos, pObj->GetEvents());
        }

        // FormObject zuruecksetzen
        pObj->SetObjEnv(Reference< ::com::sun::star::container::XIndexContainer > ());
    }
}

//------------------------------------------------------------------
void FmXUndoEnvironment::Removed(SdrObject* pObj)
{
    if (bReadOnly)
        return;

    if (pObj->GetObjInventor() == FmFormInventor)
    {
        FmFormObj* pFormObj = PTR_CAST(FmFormObj, pObj);
        Removed(pFormObj);
    }
    else if (pObj->IsGroupObject())
    {
        SdrObjListIter aIter(*pObj->GetSubList());
        while (aIter.IsMore())
        {
            SdrObject* pObj = aIter.Next();
            Removed(pObj);
        }
    }
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::Removed(FmFormObj* pObj)
{
    // ist das Control noch einer ::com::sun::star::form::Form zugeordnet
    Reference< XInterface >  xModel = pObj->GetUnoControlModel();
    Reference< ::com::sun::star::form::XFormComponent >  xContent(xModel, UNO_QUERY);
    if (xContent.is())
    {
        // das Object wird aus einer Liste herausgenommen
        // existiert ein Vater wird das Object beim beim Vater entfernt und
        // am FormObject gemerkt!

        // wird das Object wieder eingefuegt und ein Parent existiert, so wird dieser
        // Parent wiederum gesetzt
        Reference< ::com::sun::star::container::XIndexContainer >  xForm(xContent->getParent(), UNO_QUERY);
        if (xForm.is())
        {
            Reference< ::com::sun::star::container::XIndexAccess >  xIndexAccess((::com::sun::star::container::XIndexContainer*)xForm.get());
            // Feststellen an welcher Position sich das Kind befunden hat
            sal_Int32 nPos = getElementPos(xIndexAccess, xContent);
            if (nPos >= 0)
            {
                Sequence< ::com::sun::star::script::ScriptEventDescriptor > aEvts;
                Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xForm, UNO_QUERY);
                if (xManager.is())
                    aEvts = xManager->getScriptEvents(nPos);

                try
                {
                    pObj->SetObjEnv(xForm, nPos, aEvts);
                    xForm->removeByIndex(nPos);
                }
                catch(Exception&)
                {
                }

            }
        }
    }
}

//  ::com::sun::star::lang::XEventListener
//------------------------------------------------------------------------------
void SAL_CALL FmXUndoEnvironment::disposing(const ::com::sun::star::lang::EventObject& e) throw( RuntimeException )
{
    // check if it's an object we have cached informations about
    if (m_pPropertySetCache)
    {
        Reference< ::com::sun::star::beans::XPropertySet > xSourceSet(e.Source, UNO_QUERY);
        if (xSourceSet.is())
        {
            PropertySetInfoCache* pCache = static_cast<PropertySetInfoCache*>(m_pPropertySetCache);
            PropertySetInfoCacheIterator aSetPos = pCache->find(xSourceSet);
            if (aSetPos != pCache->end())
                pCache->erase(aSetPos);
        }
    }
}

// ::com::sun::star::beans::XPropertyChangeListener
//------------------------------------------------------------------------------
void SAL_CALL FmXUndoEnvironment::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
{
    if (!IsLocked())
    {
        // kein Undo fuer transiente und readonly properties
        Reference< ::com::sun::star::beans::XPropertySet >  xSet(evt.Source, UNO_QUERY);
        if (xSet.is())
        {
            if (!m_pPropertySetCache)
                m_pPropertySetCache = new PropertySetInfoCache;
            PropertySetInfoCache* pCache = static_cast<PropertySetInfoCache*>(m_pPropertySetCache);

            // let's see if we know something about the set
            PropertySetInfoCacheIterator aSetPos = pCache->find(xSet);
            if (aSetPos == pCache->end())
            {
                PropertySetInfo aNewEntry;
                if (!::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xSet))
                {
                    aNewEntry.bHasEmptyControlSource = sal_False;
                }
                else
                {
                    try
                    {
                        Any aCurrentControlSource = xSet->getPropertyValue(FM_PROP_CONTROLSOURCE);
                        aNewEntry.bHasEmptyControlSource = !aCurrentControlSource.hasValue() || (::comphelper::getString(aCurrentControlSource).getLength() == 0);
                    }
                    catch(Exception&)
                    {
                    }
                }
                (*pCache)[xSet] = aNewEntry;
                aSetPos = pCache->find(xSet);
                DBG_ASSERT(aSetPos != pCache->end(), "FmXUndoEnvironment::propertyChange : just inserted it ... why it's not there ?");
            }
            else
            {   // is it the DataField property ?
                if (evt.PropertyName.equals(FM_PROP_CONTROLSOURCE))
                {
                    aSetPos->second.bHasEmptyControlSource = !evt.NewValue.hasValue() || (::comphelper::getString(evt.NewValue).getLength() == 0);
                }
            }

            // now we have access to the cached info about the set
            // let's see what we know about the property
            PropertySetInfo::AllProperties& rPropInfos = aSetPos->second.aProps;
            PropertySetInfo::AllPropertiesIterator aPropertyPos = rPropInfos.find(evt.PropertyName);
            if (aPropertyPos == rPropInfos.end())
            {   // nothing 'til now ... have to change this ....
                PropertyInfo aNewEntry;

                // the attributes
                INT32 nAttributes = xSet->getPropertySetInfo()->getPropertyByName(evt.PropertyName).Attributes;
                aNewEntry.bIsTransientOrReadOnly = ((nAttributes & ::com::sun::star::beans::PropertyAttribute::READONLY) != 0) || ((nAttributes & ::com::sun::star::beans::PropertyAttribute::TRANSIENT) != 0);

                // check if it is the special "DataFieldProperty"
                aNewEntry.bIsControlSourceProperty = sal_False;
                try
                {
                    if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCEPROPERTY, xSet))
                    {
                        Any aControlSourceProperty = xSet->getPropertyValue(FM_PROP_CONTROLSOURCEPROPERTY);
                        ::rtl::OUString sControlSourceProperty;
                        aControlSourceProperty >>= sControlSourceProperty;

                        aNewEntry.bIsControlSourceProperty = (sControlSourceProperty.equals(evt.PropertyName));
                    }
                }
                catch(Exception&)
                {
                }

                // insert the new entry
                rPropInfos[evt.PropertyName] = aNewEntry;
                aPropertyPos = rPropInfos.find(evt.PropertyName);
                DBG_ASSERT(aPropertyPos != rPropInfos.end(), "FmXUndoEnvironment::propertyChange : just inserted it ... why it's not there ?");
            }

            // now we have access to the cached info about the property affected
            // and are able to decide wether or not we need an undo action

            if (!aPropertyPos->second.bIsTransientOrReadOnly)
            {   // normally we would generate an undo action for all non-readonly and non-transient properties, but ...

                // check if it is a special control property which is required for data field connectivity, these
                // special properties may be handled as though they were transient ...
                if (!aPropertyPos->second.bIsControlSourceProperty || aSetPos->second.bHasEmptyControlSource)
                    rModel.AddUndo(new FmUndoPropertyAction(rModel, evt));
            }
        }
    }
    else
    {
        // if it's the DataField property we may have to adjust our cache
        if (m_pPropertySetCache && evt.PropertyName.equals(FM_PROP_CONTROLSOURCE))
        {
            Reference< ::com::sun::star::beans::XPropertySet >  xSet(evt.Source, UNO_QUERY);
            PropertySetInfoCache* pCache = static_cast<PropertySetInfoCache*>(m_pPropertySetCache);
            PropertySetInfo& rSetInfo = (*pCache)[xSet];
            rSetInfo.bHasEmptyControlSource = !evt.NewValue.hasValue() || (::comphelper::getString(evt.NewValue).getLength() == 0);
        }
    }
}

// ::com::sun::star::beans::XVetoableChangeListener
//------------------------------------------------------------------------------
void SAL_CALL FmXUndoEnvironment::vetoableChange(const ::com::sun::star::beans::PropertyChangeEvent& aEvent) throw( ::com::sun::star::beans::PropertyVetoException, RuntimeException )
{
    if (aEvent.PropertyName == FM_PROP_DATASOURCE)
    {
        // if the database form belongs to a connection
        // it is not possible to change the connection
        if (findConnection(aEvent.Source).is())
        {
            ::rtl::OUString aMessage = ::rtl::OUString(SVX_RES(RID_STR_VETO_DATASOURCE));
            throw(::com::sun::star::beans::PropertyVetoException(aMessage, (::com::sun::star::beans::XVetoableChangeListener*)this));
        }
    }
}

// ::com::sun::star::container::XContainerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXUndoEnvironment::elementInserted(const ::com::sun::star::container::ContainerEvent& evt)
{
    // neues Object zum lauschen
    Reference< XInterface >  xIface;
    evt.Element >>= xIface;
    AddElement(xIface);

    if (!IsLocked() && rModel.GetObjectShell())
    {
        rModel.GetObjectShell()->SetModified(sal_True);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXUndoEnvironment::elementReplaced(const ::com::sun::star::container::ContainerEvent& evt)
{
    Reference< XInterface >  xIface;
    evt.ReplacedElement >>= xIface;
    RemoveElement(xIface);

    evt.Element >>= xIface;
    AddElement(xIface);

    if (!IsLocked() && rModel.GetObjectShell())
    {
        rModel.GetObjectShell()->SetModified(sal_True);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXUndoEnvironment::elementRemoved(const ::com::sun::star::container::ContainerEvent& evt)
{
    Reference< XInterface >  xIface;
    evt.ReplacedElement >>= xIface;
    RemoveElement(xIface);

    if (!IsLocked() && rModel.GetObjectShell())
    {
        rModel.GetObjectShell()->SetModified(sal_True);
    }
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::AddForms(const Reference< ::com::sun::star::container::XNameContainer > & rForms)
{
    Lock();
    Reference< XInterface >  xInt = rForms;
    AddElement(xInt);
    UnLock();
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::RemoveForms(const Reference< ::com::sun::star::container::XNameContainer > & rForms)
{
    Lock();
    Reference< XInterface >  xInt = rForms;
    RemoveElement(xInt);
    UnLock();
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::AlterPropertyListening(const Reference< XInterface > & Element)
{
    // am ::com::sun::star::sdbcx::Container horchen
    Reference< ::com::sun::star::container::XIndexContainer >  xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        sal_uInt32 nCount = xContainer->getCount();
        Reference< XInterface >  xIface;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            xContainer->getByIndex(i) >>= xIface;
            AlterPropertyListening(xIface);
        }
    }

    Reference< ::com::sun::star::beans::XPropertySet >  xSet(Element, UNO_QUERY);
    if (xSet.is())
    {
        if (!bReadOnly)
            xSet->addPropertyChangeListener(::rtl::OUString(), (::com::sun::star::beans::XPropertyChangeListener*)this);
        else
            xSet->removePropertyChangeListener(::rtl::OUString(), (::com::sun::star::beans::XPropertyChangeListener*)this);
    }
}


//------------------------------------------------------------------------------
void FmXUndoEnvironment::AddElement(const Reference< XInterface > & Element)
{
    // am ::com::sun::star::sdbcx::Container horchen
    Reference< ::com::sun::star::container::XIndexContainer >  xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        // Wenn der ::com::sun::star::sdbcx::Container ein EventAttachManager ist, mussen wir uns
        // auch noch als ScriptListener anmelden.
        Reference< ::com::sun::star::script::XEventAttacherManager >  xEAManager(Element, UNO_QUERY);
        if( xEAManager.is() )
            xEAManager->addScriptListener( (::com::sun::star::script::XScriptListener*)this );

        sal_uInt32 nCount = xContainer->getCount();
        Reference< XInterface >  xIface;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            xContainer->getByIndex(i) >>= xIface;
            AddElement(xIface);
        }

        Reference< ::com::sun::star::container::XContainer >  xCont(Element, UNO_QUERY);
        if (xCont.is())
            xCont->addContainerListener((::com::sun::star::container::XContainerListener*)this);
    }

    if (!bReadOnly)
    {
        // auf Properties horchen
        Reference< ::com::sun::star::beans::XPropertySet >  xSet(Element, UNO_QUERY);
        if (xSet.is())
        {
            xSet->addPropertyChangeListener(::rtl::OUString(), (::com::sun::star::beans::XPropertyChangeListener*)this);
            Reference< ::com::sun::star::form::XForm >  xForm(xSet, UNO_QUERY);
            if (xForm.is())
                xSet->addVetoableChangeListener(FM_PROP_DATASOURCE, (::com::sun::star::beans::XVetoableChangeListener*)this);
        }
    }
}

//------------------------------------------------------------------------------
void FmXUndoEnvironment::RemoveElement(const Reference< XInterface > & Element)
{
    if (!bReadOnly)
    {
        // Verbindung zu PropertySet aufheben
        Reference< ::com::sun::star::beans::XPropertySet >  xSet(Element, UNO_QUERY);
        if (xSet.is())
        {
            xSet->removePropertyChangeListener(::rtl::OUString(), (::com::sun::star::beans::XPropertyChangeListener*)this);

            Reference< ::com::sun::star::form::XForm >  xForm(xSet, UNO_QUERY);
            if (xForm.is())
                xSet->removeVetoableChangeListener(FM_PROP_DATASOURCE, (::com::sun::star::beans::XVetoableChangeListener*)this);
        }
    }

    // Verbindung zu Kindern aufheben
    Reference< ::com::sun::star::container::XIndexContainer >  xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        Reference< ::com::sun::star::container::XContainer >  xCont(Element, UNO_QUERY);
        if (xCont.is())
            xCont->removeContainerListener((::com::sun::star::container::XContainerListener*)this);

        // Wenn der ::com::sun::star::sdbcx::Container ein EventAttachManager ist, mussen wir uns
        // auch noch als ScriptListener anmelden.
        Reference< ::com::sun::star::script::XEventAttacherManager >  xEAManager(Element, UNO_QUERY);
        if( xEAManager.is() )
            xEAManager->removeScriptListener( (::com::sun::star::script::XScriptListener*)this );

        sal_uInt32 nCount = xContainer->getCount();
        Reference< XInterface >  xIface;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            xContainer->getByIndex(i) >>= xIface;
            RemoveElement(xIface);
        }
    }
}


// ::com::sun::star::script::XScriptListener
void FmXUndoEnvironment::firing_Impl( const ::com::sun::star::script::ScriptEvent& evt, Any *pSyncRet )
{
    ::vos::OClearableGuard aGuard( Application::GetSolarMutex() );

    SfxObjectShellRef xObjSh = rModel.GetObjectShell();
    if( !xObjSh.Is() )
        return;

    {
        Reference< XInterface >  xThis;
        if (evt.Helper.getValueType() ==
            ::getCppuType((const Reference< ::com::sun::star::form::XFormController>*)0))
        {
            Reference< ::com::sun::star::form::XFormController >  xController;
            evt.Helper >>= xController;
            xThis = Reference< XInterface > (xController, UNO_QUERY);
        }
        else if (evt.Helper.getValueType() ==
            ::getCppuType((const Reference< ::com::sun::star::beans::XPropertySet>*)0))

        {
            Reference< ::com::sun::star::beans::XPropertySet >  xSet;
            evt.Helper >>= xSet;
            Reference< ::com::sun::star::form::XForm >  xForm(xSet, UNO_QUERY);

            if ( xForm.is())
            {
                // these events can't be called form a database ::com::sun::star::form::Form
                if (evt.MethodName == ::rtl::OUString::createFromAscii("errorOccured"))
                    return;
                else if (evt.MethodName == ::rtl::OUString::createFromAscii("approveCursorMove") ||
                         evt.MethodName == ::rtl::OUString::createFromAscii("approveRowChange") ||
                         evt.MethodName == ::rtl::OUString::createFromAscii("approveRowSetChange") ||
                         evt.MethodName == ::rtl::OUString::createFromAscii("approveParameter"))
                {
                    sal_Bool bB = sal_True;
                    pSyncRet->setValue(&bB,::getBooleanCppuType());
                    return;
                }
            }
            xThis = Reference< XInterface > (xSet, UNO_QUERY);
        }
        else if( evt.Helper.getValueType() ==
            ::getCppuType((const Reference< ::com::sun::star::awt::XControl>*)0) )

        {
            Reference< ::com::sun::star::awt::XControl >  xControl;
            evt.Helper >>= xControl;
            xThis = Reference< XInterface > (xControl, UNO_QUERY);
        }

        aGuard.clear();
        if (xThis.is())
            xObjSh->CallScript( evt.ScriptType, evt.ScriptCode,
                                xThis, (void *)&evt.Arguments,
                                pSyncRet );

    }

    // Objectshells are not thread safe, so guard the destruction
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        xObjSh = NULL;
    }
}

void SAL_CALL FmXUndoEnvironment::firing(const ::com::sun::star::script::ScriptEvent& evt)
{
    firing_Impl( evt );
}

//------------------------------------------------------------------------------
Any SAL_CALL FmXUndoEnvironment::approveFiring(const ::com::sun::star::script::ScriptEvent& evt)
{
    Any aRet;
    firing_Impl( evt, &aRet );
    return aRet;
}

//------------------------------------------------------------------------------
FmUndoPropertyAction::FmUndoPropertyAction(FmFormModel& rNewMod, const ::com::sun::star::beans::PropertyChangeEvent& evt)
                     :SdrUndoAction(rNewMod)
                     ,aPropertyName(evt.PropertyName)
                     ,aNewValue(evt.NewValue)
                     ,aOldValue(evt.OldValue)
                     ,xObj(evt.Source, UNO_QUERY)
{
    if (rNewMod.GetObjectShell())
        rNewMod.GetObjectShell()->SetModified(sal_True);
    if(static_STR_UNDO_PROPERTY.Len() != 0)
        static_STR_UNDO_PROPERTY = SVX_RES(RID_STR_UNDO_PROPERTY);
}


//------------------------------------------------------------------------------
void FmUndoPropertyAction::Undo()
{
    FmXUndoEnvironment& rEnv = ((FmFormModel&)rMod).GetUndoEnv();

    if (xObj.is() && !rEnv.IsLocked())
    {
        // Locking damit keine neue UndoAction entsteht
        rEnv.Lock();
        xObj->setPropertyValue(aPropertyName, aOldValue);
        rEnv.UnLock();
    }
}

//------------------------------------------------------------------------------
void FmUndoPropertyAction::Redo()
{
    FmXUndoEnvironment& rEnv = ((FmFormModel&)rMod).GetUndoEnv();

    if (xObj.is() && !rEnv.IsLocked())
    {
        rEnv.Lock();
        xObj->setPropertyValue(aPropertyName, aNewValue);
        rEnv.UnLock();
    }
}

//------------------------------------------------------------------------------
XubString FmUndoPropertyAction::GetComment() const
{
    XubString aStr(static_STR_UNDO_PROPERTY);
    sal_uInt16 nId = FmPropertyInfoService::getPropertyId(aPropertyName);
    if (nId)
        aStr.SearchAndReplace('#', FmPropertyInfoService::getPropertyTranslation(nId));
    else
        aStr.SearchAndReplace('#', aPropertyName);
    return aStr;
}


DBG_NAME(FmUndoContainerAction);
//------------------------------------------------------------------------------
FmUndoContainerAction::FmUndoContainerAction(FmFormModel& rMod,
                                             Action _eAction,
                                             const Reference< ::com::sun::star::container::XIndexContainer > & xCont,
                                             const Reference< XInterface > & xElem,
                                             sal_Int32 nIdx)
                      :SdrUndoAction(rMod)
                      ,eAction(_eAction)
                      ,xContainer(xCont)
                      ,nIndex(nIdx)
{
    DBG_CTOR(FmUndoContainerAction,NULL);
    if (xCont.is() && xElem.is())
    {
        // den Richtigen IFacePointer
        ::comphelper::query_interface(xElem, xElement);
        if (eAction == Removed)
        {
            if (nIndex < 0)
            {
                // Feststellen an welcher Position sich das Kind befunden hat
                Reference< ::com::sun::star::container::XIndexAccess >  xInd(xContainer,UNO_QUERY);
                nIndex = getElementPos(xInd, xElement);
            }

            if (nIndex >= 0)
            {
                Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xCont, UNO_QUERY);
                if (xManager.is())
                    aEvts = xManager->getScriptEvents(nIndex);
            }
            else
                xElement = NULL;

            xOwnElement = xElement;
        }
        else
        {
            if (nIndex < 0)
                nIndex = xContainer->getCount();
        }
    }
}

//------------------------------------------------------------------------------
FmUndoContainerAction::~FmUndoContainerAction()
{
    Reference< ::com::sun::star::lang::XComponent >  xComp(xOwnElement, UNO_QUERY);
    if (xComp.is())
    {
        Reference< ::com::sun::star::container::XChild >  xChild(xOwnElement, UNO_QUERY);
        // nur wenn das Objekt frei schwebt
        if (xChild.is() && !xChild->getParent().is())
            xComp->dispose();
    }
    DBG_DTOR(FmUndoContainerAction,NULL);
}

//------------------------------------------------------------------------------
void FmUndoContainerAction::Undo()
{
    FmXUndoEnvironment& rEnv = ((FmFormModel&)rMod).GetUndoEnv();
    if (xContainer.is() && !rEnv.IsLocked() && xElement.is())
    {
        rEnv.Lock();
        switch (eAction)
        {
            case Inserted:
            {
                Reference< XInterface >  xObj,xIface;;
                xContainer->getByIndex(nIndex) >>= xObj;


                ::comphelper::query_interface(xObj, xIface);
                if ((XInterface *)xElement.get() == (XInterface *)xIface.get())
                {
                    Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xContainer, UNO_QUERY);
                    if (xManager.is())
                        aEvts = xManager->getScriptEvents(nIndex);
                    xContainer->removeByIndex(nIndex);
                    xOwnElement = xElement;
                }
            }   break;
            case Removed:
                if (xContainer->getCount() >= nIndex)
                {
                    Any aVal;
                    if (xContainer->getElementType() == ::getCppuType((const Reference< ::com::sun::star::form::XFormComponent>*)0))

                    {
                        Reference< ::com::sun::star::form::XFormComponent >  xFmcomp(xElement, UNO_QUERY);
                        aVal <<= xFmcomp;
                    }
                    else
                    {
                        Reference< ::com::sun::star::form::XForm >  xForm(xElement, UNO_QUERY);
                        aVal <<= xForm;
                    }

                    xContainer->insertByIndex(nIndex, aVal);
                    Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xContainer, UNO_QUERY);
                    if (xManager.is())
                        xManager->registerScriptEvents(nIndex, aEvts);
                    xOwnElement = NULL;
                }   break;
        }
        rEnv.UnLock();
    }
}

//------------------------------------------------------------------------------
void FmUndoContainerAction::Redo()
{
    FmXUndoEnvironment& rEnv = ((FmFormModel&)rMod).GetUndoEnv();
    if (xContainer.is() && !rEnv.IsLocked() && xElement.is())
    {
        rEnv.Lock();
        switch (eAction)
        {
            case Inserted:
            {
                if (xContainer->getCount() >= nIndex)
                {
                    Any aVal;
                    if (xContainer->getElementType() ==
                        ::getCppuType((const Reference< ::com::sun::star::form::XFormComponent>*)0))

                    {
                        Reference< ::com::sun::star::form::XFormComponent >  xFmcomp(xElement, UNO_QUERY);
                        aVal <<= xFmcomp;
                    }
                    else
                    {
                        Reference< ::com::sun::star::form::XForm >  xForm(xElement, UNO_QUERY);
                        aVal <<= xForm;
                    }

                    xContainer->insertByIndex(nIndex, aVal);

                    Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xContainer, UNO_QUERY);
                    if (xManager.is())
                        xManager->registerScriptEvents(nIndex, aEvts);
                    xOwnElement = NULL;
                }
            }   break;
            case Removed:
            {
                Reference< XInterface >  xObj;
                xContainer->getByIndex(nIndex) >>= xObj;
                if ((XInterface *)xElement.get() == (XInterface *)xObj.get())
                {
                    Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xContainer, UNO_QUERY);
                    if (xManager.is())
                        aEvts = xManager->getScriptEvents(nIndex);
                    xContainer->removeByIndex(nIndex);
                    xOwnElement = xElement;
                }
            }   break;
        }
        rEnv.UnLock();
    }
}

//------------------------------------------------------------------------------
FmUndoModelReplaceAction::FmUndoModelReplaceAction(FmFormModel& _rMod, SdrUnoObj* _pObject, const Reference< ::com::sun::star::awt::XControlModel > & _xReplaced)
    :SdrUndoAction(_rMod)
    ,m_xReplaced(_xReplaced)
    ,m_pObject(_pObject)
{
}

//------------------------------------------------------------------------------
FmUndoModelReplaceAction::~FmUndoModelReplaceAction()
{
    // dispose our element if nobody else is responsible for
    Reference< ::com::sun::star::lang::XComponent >  xComp(m_xReplaced, UNO_QUERY);
    if (xComp.is())
    {
        Reference< ::com::sun::star::container::XChild >  xChild(m_xReplaced, UNO_QUERY);
        if (!xChild.is() || !xChild->getParent().is())
            xComp->dispose();
    }
}

//------------------------------------------------------------------------------
void FmUndoModelReplaceAction::Undo()
{
    Reference< ::com::sun::star::awt::XControlModel >  xCurrentModel(m_pObject->GetUnoControlModel());

    // replace the model within the parent
    Reference< ::com::sun::star::container::XChild >  xOldChild(m_xReplaced, UNO_QUERY);
    if (xOldChild.is())
    {
        Reference< ::com::sun::star::container::XNameContainer >  xParent(xOldChild->getParent(), UNO_QUERY);
        if (xParent.is())
        {
            // the form container works with ::com::sun::star::form::FormComponents
            Reference< ::com::sun::star::form::XFormComponent >  xComponent(m_xReplaced, UNO_QUERY);
            DBG_ASSERT(xComponent.is(), "FmUndoModelReplaceAction::Undo : the new model is no form component !");
            try
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xCurrentSet(xCurrentModel, UNO_QUERY);
                DBG_ASSERT(::comphelper::hasProperty(FM_PROP_NAME, xCurrentSet),
                    "FmUndoModelReplaceAction::Undo : one of the models is invalid !");
                xParent->replaceByName(::comphelper::getString(xCurrentSet->getPropertyValue(FM_PROP_NAME)), makeAny(xComponent));
            }
            catch(Exception&)
            {
                DBG_ERROR("FmUndoModelReplaceAction::Undo : could not replace the model !");
            }

        }
    }

    m_pObject->SetUnoControlModel(m_xReplaced);
    m_pObject->SetChanged();

    m_xReplaced = xCurrentModel;
}

//------------------------------------------------------------------------------
XubString FmUndoModelReplaceAction::GetComment() const
{
    return SVX_RES(RID_STR_UNDO_MODEL_REPLACE);
}




