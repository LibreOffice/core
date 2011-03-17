/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"

#include "RptUndo.hxx"
#include "uistrings.hrc"
#include "rptui_slotid.hrc"
#include "UITools.hxx"
#include "UndoEnv.hxx"

#include <dbaccess/IController.hxx>
#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <svx/unoshape.hxx>
#include <boost/bind.hpp>
#include <functional>

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

//----------------------------------------------------------------------------
namespace
{
    void lcl_collectElements(const uno::Reference< report::XSection >& _xSection,::std::vector< uno::Reference< drawing::XShape> >& _rControls)
    {
        if ( _xSection.is() )
        {
            sal_Int32 nCount = _xSection->getCount();
            _rControls.reserve(nCount);
            while ( nCount )
            {
                uno::Reference< drawing::XShape> xShape(_xSection->getByIndex(nCount-1),uno::UNO_QUERY);
                _rControls.push_back(xShape);
                _xSection->remove(xShape);
                --nCount;
            }
        }
    }
    //----------------------------------------------------------------------------
    void lcl_insertElements(const uno::Reference< report::XSection >& _xSection,const ::std::vector< uno::Reference< drawing::XShape> >& _aControls)
    {
        if ( _xSection.is() )
        {
            ::std::vector< uno::Reference< drawing::XShape> >::const_reverse_iterator aIter = _aControls.rbegin();
            ::std::vector< uno::Reference< drawing::XShape> >::const_reverse_iterator aEnd = _aControls.rend();
            for (; aIter != aEnd; ++aIter)
            {
                try
                {
                    const awt::Point aPos = (*aIter)->getPosition();
                    const awt::Size aSize = (*aIter)->getSize();
                    _xSection->add(*aIter);
                    (*aIter)->setPosition( aPos );
                    (*aIter)->setSize( aSize );
                }
                catch(const uno::Exception&)
                {
                    OSL_FAIL("lcl_insertElements:Exception caught!");
                }
            }
        }
    }
    //----------------------------------------------------------------------------
    void lcl_setValues(const uno::Reference< report::XSection >& _xSection,const ::std::vector< ::std::pair< ::rtl::OUString ,uno::Any> >& _aValues)
    {
        if ( _xSection.is() )
        {
            ::std::vector< ::std::pair< ::rtl::OUString ,uno::Any> >::const_iterator aIter = _aValues.begin();
            ::std::vector< ::std::pair< ::rtl::OUString ,uno::Any> >::const_iterator aEnd = _aValues.end();
            for (; aIter != aEnd; ++aIter)
            {
                try
                {
                    _xSection->setPropertyValue(aIter->first,aIter->second);
                }
                catch(const uno::Exception&)
                {
                    OSL_FAIL("lcl_setValues:Exception caught!");
                }
            }
        }
    }
}
//----------------------------------------------------------------------------
TYPEINIT1( OSectionUndo,         OCommentUndoAction );
DBG_NAME(rpt_OSectionUndo)
//----------------------------------------------------------------------------
OSectionUndo::OSectionUndo(OReportModel& _rMod
                           ,sal_uInt16 _nSlot
                           ,Action _eAction
                           ,sal_uInt16 nCommentID)
: OCommentUndoAction(_rMod,nCommentID)
,m_eAction(_eAction)
,m_nSlot(_nSlot)
,m_bInserted(false)
{
    DBG_CTOR(rpt_OSectionUndo,NULL);
}
// -----------------------------------------------------------------------------
OSectionUndo::~OSectionUndo()
{
    if ( !m_bInserted )
    {
        OXUndoEnvironment& rEnv = static_cast< OReportModel& >( rMod ).GetUndoEnv();
        ::std::vector< uno::Reference< drawing::XShape> >::iterator aEnd = m_aControls.end();
        for (::std::vector< uno::Reference< drawing::XShape> >::iterator aIter = m_aControls.begin(); aIter != aEnd; ++aIter)
        {
            uno::Reference< drawing::XShape> xShape = *aIter;
            rEnv.RemoveElement(xShape);

#if OSL_DEBUG_LEVEL > 0
            SvxShape* pShape = SvxShape::getImplementation( xShape );
            SdrObject* pObject = pShape ? pShape->GetSdrObject() : NULL;
            OSL_ENSURE( pShape && pShape->HasSdrObjectOwnership() && pObject && !pObject->IsInserted(),
                "OSectionUndo::~OSectionUndo: inconsistency in the shape/object ownership!" );
#endif
            try
            {
                comphelper::disposeComponent(xShape);
            }
            catch(uno::Exception)
            {
                OSL_FAIL("Exception caught!");
            }
        }
    }
    DBG_DTOR(rpt_OSectionUndo,NULL);
}
// -----------------------------------------------------------------------------
void OSectionUndo::collectControls(const uno::Reference< report::XSection >& _xSection)
{
    m_aControls.clear();
    try
    {
        // copy all properties for restoring
        uno::Reference< beans::XPropertySetInfo> xInfo = _xSection->getPropertySetInfo();
        uno::Sequence< beans::Property> aSeq = xInfo->getProperties();
        const beans::Property* pIter = aSeq.getConstArray();
        const beans::Property* pEnd  = pIter + aSeq.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( 0 == (pIter->Attributes & beans::PropertyAttribute::READONLY) )
                m_aValues.push_back(::std::pair< ::rtl::OUString ,uno::Any>(pIter->Name,_xSection->getPropertyValue(pIter->Name)));
        }
        lcl_collectElements(_xSection,m_aControls);
    }
    catch(uno::Exception&)
    {
    }
}
//----------------------------------------------------------------------------
void OSectionUndo::Undo()
{
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
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "OSectionUndo::Undo: caught an exception!" );
    }
}
//----------------------------------------------------------------------------
void OSectionUndo::Redo()
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
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "OSectionUndo::Redo: caught an exception!" );
    }
}
//----------------------------------------------------------------------------
TYPEINIT1( OReportSectionUndo,         OSectionUndo );
//----------------------------------------------------------------------------
OReportSectionUndo::OReportSectionUndo(OReportModel& _rMod,sal_uInt16 _nSlot
                                       ,::std::mem_fun_t< uno::Reference< report::XSection >
                                            ,OReportHelper> _pMemberFunction
                                       ,const uno::Reference< report::XReportDefinition >& _xReport
                                       ,Action _eAction
                                       ,sal_uInt16 nCommentID)
: OSectionUndo(_rMod,_nSlot,_eAction,nCommentID)
,m_aReportHelper(_xReport)
,m_pMemberFunction(_pMemberFunction)
{
    if( m_eAction == Removed )
        collectControls(m_pMemberFunction(&m_aReportHelper));
}
// -----------------------------------------------------------------------------
OReportSectionUndo::~OReportSectionUndo()
{
}
//----------------------------------------------------------------------------
void OReportSectionUndo::implReInsert( )
{
    const uno::Sequence< beans::PropertyValue > aArgs;
    m_pController->executeChecked(m_nSlot,aArgs);
    uno::Reference< report::XSection > xSection = m_pMemberFunction(&m_aReportHelper);
    lcl_insertElements(xSection,m_aControls);
    lcl_setValues(xSection,m_aValues);
    m_bInserted = true;
}
//----------------------------------------------------------------------------
void OReportSectionUndo::implReRemove( )
{
    if( m_eAction == Removed )
        collectControls(m_pMemberFunction(&m_aReportHelper));
    const uno::Sequence< beans::PropertyValue > aArgs;
    m_pController->executeChecked(m_nSlot,aArgs);
    m_bInserted = false;
}
//----------------------------------------------------------------------------
TYPEINIT1( OGroupSectionUndo,         OSectionUndo );
//----------------------------------------------------------------------------
OGroupSectionUndo::OGroupSectionUndo(OReportModel& _rMod,sal_uInt16 _nSlot
                                       ,::std::mem_fun_t< uno::Reference< report::XSection >
                                            ,OGroupHelper> _pMemberFunction
                                       ,const uno::Reference< report::XGroup >& _xGroup
                                       ,Action _eAction
                                       ,sal_uInt16 nCommentID)
: OSectionUndo(_rMod,_nSlot,_eAction,nCommentID)
,m_aGroupHelper(_xGroup)
,m_pMemberFunction(_pMemberFunction)
{
    if( m_eAction == Removed )
    {
        uno::Reference< report::XSection > xSection = m_pMemberFunction(&m_aGroupHelper);
        if ( xSection.is() )
            m_sName = xSection->getName();
        collectControls(xSection);
    }
}
//----------------------------------------------------------------------------
String OGroupSectionUndo::GetComment() const
{
    if ( !m_sName.getLength() )
    {
        try
        {
            uno::Reference< report::XSection > xSection = const_cast<OGroupSectionUndo*>(this)->m_pMemberFunction(&const_cast<OGroupSectionUndo*>(this)->m_aGroupHelper);

            if ( xSection.is() )
                m_sName = xSection->getName();
        }
        catch(uno::Exception&)
        {}
    }
    return m_strComment + m_sName;
}
//----------------------------------------------------------------------------
void OGroupSectionUndo::implReInsert( )
{
    uno::Sequence< beans::PropertyValue > aArgs(2);

    aArgs[0].Name = SID_GROUPHEADER_WITHOUT_UNDO == m_nSlot? PROPERTY_HEADERON : PROPERTY_FOOTERON;
    aArgs[0].Value <<= sal_True;
    aArgs[1].Name = PROPERTY_GROUP;
    aArgs[1].Value <<= m_aGroupHelper.getGroup();
    m_pController->executeChecked(m_nSlot,aArgs);

    uno::Reference< report::XSection > xSection = m_pMemberFunction(&m_aGroupHelper);
    lcl_insertElements(xSection,m_aControls);
    lcl_setValues(xSection,m_aValues);
    m_bInserted = true;
}
//----------------------------------------------------------------------------
void OGroupSectionUndo::implReRemove( )
{
    if( m_eAction == Removed )
        collectControls(m_pMemberFunction(&m_aGroupHelper));

    uno::Sequence< beans::PropertyValue > aArgs(2);

    aArgs[0].Name = SID_GROUPHEADER_WITHOUT_UNDO == m_nSlot? PROPERTY_HEADERON : PROPERTY_FOOTERON;
    aArgs[0].Value <<= sal_False;
    aArgs[1].Name = PROPERTY_GROUP;
    aArgs[1].Value <<= m_aGroupHelper.getGroup();

    m_pController->executeChecked(m_nSlot,aArgs);
    m_bInserted = false;
}
//----------------------------------------------------------------------------
TYPEINIT1( OGroupUndo,         OCommentUndoAction );
//----------------------------------------------------------------------------
OGroupUndo::OGroupUndo(OReportModel& _rMod
                       ,sal_uInt16 nCommentID
                       ,Action  _eAction
                       ,const uno::Reference< report::XGroup>& _xGroup
                       ,const uno::Reference< report::XReportDefinition >& _xReportDefinition)
: OCommentUndoAction(_rMod,nCommentID)
,m_xGroup(_xGroup)
,m_xReportDefinition(_xReportDefinition)
,m_eAction(_eAction)
{
    m_nLastPosition = getPositionInIndexAccess(m_xReportDefinition->getGroups().get(),m_xGroup);
}
//----------------------------------------------------------------------------
void OGroupUndo::implReInsert( )
{
    try
    {
        m_xReportDefinition->getGroups()->insertByIndex(m_nLastPosition,uno::makeAny(m_xGroup));
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Exception catched while undoing remove group");
    }
}
//----------------------------------------------------------------------------
void OGroupUndo::implReRemove( )
{
    try
    {
        m_xReportDefinition->getGroups()->removeByIndex(m_nLastPosition);
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Exception catched while redoing remove group");
    }
}
//----------------------------------------------------------------------------
void OGroupUndo::Undo()
{
    switch ( m_eAction )
    {
    case Inserted:
        implReRemove();
        break;

    case Removed:
        implReInsert();
        break;
    }

}
//----------------------------------------------------------------------------
void OGroupUndo::Redo()
{
    switch ( m_eAction )
    {
    case Inserted:
        implReInsert();
        break;

    case Removed:
        implReRemove();
        break;
    }
}
//----------------------------------------------------------------------------
//============================================================================
} // rptui
//============================================================================


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
