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

#include "UndoActions.hxx"
#include "UndoEnv.hxx"
#include "formatnormalizer.hxx"
#include "conditionupdater.hxx"
#include "corestrings.hrc"
#include "rptui_slotid.hrc"
#include "RptDef.hxx"
#include "ModuleHelper.hxx"
#include "RptObject.hxx"
#include "RptPage.hxx"
#include "RptResId.hrc"
#include "RptModel.hxx"

#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <connectivity/dbtools.hxx>
#include <svl/smplhint.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/stl_types.hxx>
#include <vcl/svapp.hxx>
#include <dbaccess/dbsubcomponentcontroller.hxx>
#include <svx/unoshape.hxx>
#include <osl/mutex.hxx>

namespace rptui
{
    using namespace ::com::sun::star;
    using namespace uno;
    using namespace lang;
    using namespace script;
    using namespace beans;
    using namespace awt;
    using namespace util;
    using namespace container;
    using namespace report;

::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> OGroupHelper::getMemberFunction(const Reference< XSection >& _xSection)
{
    ::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> pMemFunSection = ::std::mem_fun(&OGroupHelper::getFooter);
    uno::Reference< report::XGroup> xGroup = _xSection->getGroup();
    if ( xGroup->getHeaderOn() && xGroup->getHeader() == _xSection )
        pMemFunSection = ::std::mem_fun(&OGroupHelper::getHeader);
    return pMemFunSection;
}

::std::mem_fun_t<uno::Reference<report::XSection> , OReportHelper> OReportHelper::getMemberFunction(const Reference< XSection >& _xSection)
{
    uno::Reference< report::XReportDefinition> xReportDefinition(_xSection->getReportDefinition());
    ::std::mem_fun_t<uno::Reference<report::XSection> , OReportHelper> pMemFunSection = ::std::mem_fun(&OReportHelper::getReportFooter);
    if ( xReportDefinition->getReportHeaderOn() && xReportDefinition->getReportHeader() == _xSection )
        pMemFunSection = ::std::mem_fun(&OReportHelper::getReportHeader);
    else if ( xReportDefinition->getPageHeaderOn() && xReportDefinition->getPageHeader() == _xSection )
        pMemFunSection = ::std::mem_fun(&OReportHelper::getPageHeader);
    else if ( xReportDefinition->getPageFooterOn() && xReportDefinition->getPageFooter() == _xSection )
        pMemFunSection = ::std::mem_fun(&OReportHelper::getPageFooter);
    else if ( xReportDefinition->getDetail() == _xSection )
        pMemFunSection = ::std::mem_fun(&OReportHelper::getDetail);
    return pMemFunSection;
}


TYPEINIT1( OCommentUndoAction,          SdrUndoAction );

OCommentUndoAction::OCommentUndoAction(SdrModel& _rMod,sal_uInt16 nCommentID)
    :SdrUndoAction(_rMod)
{
    m_pController = static_cast< OReportModel& >( _rMod ).getController();
    if ( nCommentID )
        m_strComment = ModuleRes(nCommentID);
}
OCommentUndoAction::~OCommentUndoAction()
{
}

void OCommentUndoAction::Undo()
{
}

void OCommentUndoAction::Redo()
{
}

OUndoContainerAction::OUndoContainerAction(SdrModel& _rMod
                                             ,Action _eAction
                                             ,const uno::Reference< container::XIndexContainer >& rContainer
                                             ,const Reference< XInterface > & xElem
                                             ,sal_uInt16 _nCommentId)
                      :OCommentUndoAction(_rMod,_nCommentId)
                      ,m_xElement(xElem)
                      ,m_xContainer(rContainer)
                         ,m_eAction( _eAction )
{
    // normalize
    if ( m_eAction == Removed )
        // we now own the element
        m_xOwnElement = m_xElement;
}

OUndoContainerAction::~OUndoContainerAction()
{
    // if we own the object ....
    Reference< XComponent > xComp( m_xOwnElement, UNO_QUERY );
    if ( xComp.is() )
    {
        // and the object does not have a parent
        Reference< XChild >  xChild( m_xOwnElement, UNO_QUERY );
        if ( xChild.is() && !xChild->getParent().is() )
        {
            OXUndoEnvironment& rEnv = static_cast< OReportModel& >( rMod ).GetUndoEnv();
            rEnv.RemoveElement( m_xOwnElement );

#if OSL_DEBUG_LEVEL > 0
            SvxShape* pShape = SvxShape::getImplementation( xChild );
            SdrObject* pObject = pShape ? pShape->GetSdrObject() : nullptr;
            OSL_ENSURE( pObject == nullptr || (pShape->HasSdrObjectOwnership() && !pObject->IsInserted()),
                "OUndoContainerAction::~OUndoContainerAction: inconsistency in the shape/object ownership!" );
#endif
            // -> dispose it
            try
            {
                comphelper::disposeComponent( xComp );
            }
            catch ( const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
}

void OUndoContainerAction::implReInsert( )
{
    if ( m_xContainer.is() )
    {
        // insert the element
        m_xContainer->insertByIndex( m_xContainer->getCount(),uno::makeAny(m_xElement) );
    }
    // we don't own the object anymore
    m_xOwnElement = nullptr;
}


void OUndoContainerAction::implReRemove( )
{
    OXUndoEnvironment& rEnv = static_cast< OReportModel& >( rMod ).GetUndoEnv();
    try
    {
        OXUndoEnvironment::OUndoEnvLock aLock(rEnv);
        if ( m_xContainer.is() )
        {
            const sal_Int32 nCount = m_xContainer->getCount();
            for (sal_Int32 i = 0; i < nCount; ++i)
            {
                uno::Reference< uno::XInterface> xObj(m_xContainer->getByIndex(i),uno::UNO_QUERY);
                if ( xObj == m_xElement )
                {
                    m_xContainer->removeByIndex( i );
                    break;
                }
            }
        }
    }
    catch(uno::Exception&){}
    // from now on, we own this object
    m_xOwnElement = m_xElement;
}


void OUndoContainerAction::Undo()
{
    if ( m_xElement.is() )
    {
        // prevents that an undo action will be created for elementInserted
        try
        {
            switch ( m_eAction )
            {
            case Inserted:
                implReRemove();
                break;

            case Removed:
                implReInsert();
                break;
            default:
                OSL_FAIL("Illegal case value");
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OUndoContainerAction::Undo: caught an exception!" );
        }
    }
}


void OUndoContainerAction::Redo()
{
    if ( m_xElement.is() )
    {
        try
        {
            switch ( m_eAction )
            {
            case Inserted:
                implReInsert();
                break;

            case Removed:
                implReRemove();
                break;
            default:
                OSL_FAIL("Illegal case value");
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OUndoContainerAction::Redo: caught an exception!" );
        }
    }
}

OUndoGroupSectionAction::OUndoGroupSectionAction(SdrModel& _rMod
                                             ,Action _eAction
                                             ,::std::mem_fun_t< uno::Reference< report::XSection >
                                                    ,OGroupHelper> _pMemberFunction
                                             ,const uno::Reference< report::XGroup >& _xGroup
                                             ,const Reference< XInterface > & xElem
                                             ,sal_uInt16 _nCommentId)
:OUndoContainerAction(_rMod,_eAction,nullptr,xElem,_nCommentId)
,m_aGroupHelper(_xGroup)
,m_pMemberFunction(_pMemberFunction)
{
}

void OUndoGroupSectionAction::implReInsert( )
{
    OXUndoEnvironment& rEnv = static_cast< OReportModel& >( rMod ).GetUndoEnv();
    try
    {
        OXUndoEnvironment::OUndoEnvLock aLock(rEnv);
        uno::Reference< report::XSection> xSection = m_pMemberFunction(&m_aGroupHelper);
        if ( xSection.is() )
            xSection->add(uno::Reference< drawing::XShape>(m_xElement,uno::UNO_QUERY));
    }
    catch(uno::Exception&){}

    // we don't own the object anymore
    m_xOwnElement = nullptr;
}


void OUndoGroupSectionAction::implReRemove( )
{
        OXUndoEnvironment& rEnv = static_cast< OReportModel& >( rMod ).GetUndoEnv();
    try
    {
        OXUndoEnvironment::OUndoEnvLock aLock(rEnv);
        uno::Reference< report::XSection> xSection = m_pMemberFunction(&m_aGroupHelper);
        if ( xSection.is() )
            xSection->remove(uno::Reference< drawing::XShape>(m_xElement,uno::UNO_QUERY));
    }
    catch(uno::Exception&){}

    // from now on, we own this object
    m_xOwnElement = m_xElement;
}

OUndoReportSectionAction::OUndoReportSectionAction(SdrModel& _rMod
                                             ,Action _eAction
                                             ,::std::mem_fun_t< uno::Reference< report::XSection >
                                                ,OReportHelper> _pMemberFunction
                                             ,const uno::Reference< report::XReportDefinition >& _xReport
                                             ,const Reference< XInterface > & xElem
                                             ,sal_uInt16 _nCommentId)
:OUndoContainerAction(_rMod,_eAction,nullptr,xElem,_nCommentId)
,m_aReportHelper(_xReport)
,m_pMemberFunction(_pMemberFunction)
{
}

void OUndoReportSectionAction::implReInsert( )
{
    OXUndoEnvironment& rEnv = static_cast< OReportModel& >( rMod ).GetUndoEnv();
    try
    {
        OXUndoEnvironment::OUndoEnvLock aLock(rEnv);
        uno::Reference< report::XSection> xSection = m_pMemberFunction(&m_aReportHelper);
        if ( xSection.is() )
        {
            uno::Reference< drawing::XShape> xShape(m_xElement,uno::UNO_QUERY_THROW);
            awt::Point aPos = xShape->getPosition();
            awt::Size aSize = xShape->getSize();
            xSection->add(xShape);
            xShape->setPosition( aPos );
            xShape->setSize( aSize );
        }
    }
    catch(uno::Exception&){}
    // we don't own the object anymore
    m_xOwnElement = nullptr;
}


void OUndoReportSectionAction::implReRemove( )
{
    OXUndoEnvironment& rEnv = static_cast< OReportModel& >( rMod ).GetUndoEnv();
    try
    {
        OXUndoEnvironment::OUndoEnvLock aLock(rEnv);
        uno::Reference< report::XSection> xSection = m_pMemberFunction(&m_aReportHelper);
        if ( xSection.is() )
            xSection->remove(uno::Reference< drawing::XShape>(m_xElement,uno::UNO_QUERY));
    }
    catch(uno::Exception&){}
    // from now on, we own this object
    m_xOwnElement = m_xElement;
}

ORptUndoPropertyAction::ORptUndoPropertyAction(SdrModel& rNewMod, const PropertyChangeEvent& evt)
                     :OCommentUndoAction(rNewMod,0)
                     ,m_xObj(evt.Source, UNO_QUERY)
                     ,m_aPropertyName(evt.PropertyName)
                     ,m_aNewValue(evt.NewValue)
                     ,m_aOldValue(evt.OldValue)
{
}

void ORptUndoPropertyAction::Undo()
{
    setProperty(true);
}


void ORptUndoPropertyAction::Redo()
{
    setProperty(false);
}

Reference< XPropertySet> ORptUndoPropertyAction::getObject()
{
    return m_xObj;
}

void ORptUndoPropertyAction::setProperty(bool _bOld)
{
    Reference< XPropertySet> xObj = getObject();

    if (xObj.is() )
    {
        try
        {
            xObj->setPropertyValue( m_aPropertyName, _bOld ? m_aOldValue : m_aNewValue );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ORptUndoPropertyAction::Redo: caught an exception!" );
        }
    }
}

OUString ORptUndoPropertyAction::GetComment() const
{
    OUString aStr( ModuleRes(RID_STR_UNDO_PROPERTY).toString() );

    return aStr.replaceFirst("#", m_aPropertyName);
}

OUndoPropertyGroupSectionAction::OUndoPropertyGroupSectionAction(SdrModel& _rMod
                                             ,const PropertyChangeEvent& evt
                                             ,::std::mem_fun_t< uno::Reference< report::XSection >
                                                    ,OGroupHelper> _pMemberFunction
                                             ,const uno::Reference< report::XGroup >& _xGroup
                                             )
:ORptUndoPropertyAction(_rMod,evt)
,m_aGroupHelper(_xGroup)
,m_pMemberFunction(_pMemberFunction)
{
}

Reference< XPropertySet> OUndoPropertyGroupSectionAction::getObject()
{
    return m_pMemberFunction(&m_aGroupHelper).get();
}

OUndoPropertyReportSectionAction::OUndoPropertyReportSectionAction(SdrModel& _rMod
                                             ,const PropertyChangeEvent& evt
                                             ,::std::mem_fun_t< uno::Reference< report::XSection >
                                                ,OReportHelper> _pMemberFunction
                                             ,const uno::Reference< report::XReportDefinition >& _xReport
                                             )
:ORptUndoPropertyAction(_rMod,evt)
,m_aReportHelper(_xReport)
,m_pMemberFunction(_pMemberFunction)
{
}

Reference< XPropertySet> OUndoPropertyReportSectionAction::getObject()
{
    return m_pMemberFunction(&m_aReportHelper).get();
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
