/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoActions.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:43:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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

/** === begin UNO includes === **/
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
/** === end UNO includes === **/

#include <connectivity/dbtools.hxx>
#include <svtools/smplhint.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/stl_types.hxx>
#include <vcl/svapp.hxx>
#include <dbaccess/singledoccontroller.hxx>
#include <svx/unoshape.hxx>
#include <vos/mutex.hxx>

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
::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> OGroupHelper::getMemberFunction(const Reference< XSection >& _xSection)
{
    ::std::mem_fun_t<uno::Reference<report::XSection> , OGroupHelper> pMemFunSection = ::std::mem_fun(&OGroupHelper::getFooter);
    uno::Reference< report::XGroup> xGroup = _xSection->getGroup();
    if ( xGroup->getHeaderOn() && xGroup->getHeader() == _xSection )
        pMemFunSection = ::std::mem_fun(&OGroupHelper::getHeader);
    return pMemFunSection;
}
// -----------------------------------------------------------------------------
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

DECLARE_STL_USTRINGACCESS_MAP(bool, AllProperties);
DECLARE_STL_STDKEY_MAP(uno::Reference< beans::XPropertySet >, AllProperties, PropertySetInfoCache);
//------------------------------------------------------------------------------
TYPEINIT1( OCommentUndoAction,          SdrUndoAction );
DBG_NAME(rpt_OCommentUndoAction)
//----------------------------------------------------------------------------
OCommentUndoAction::OCommentUndoAction(SdrModel& _rMod,USHORT nCommentID)
    :SdrUndoAction(_rMod)
{
    DBG_CTOR(rpt_OCommentUndoAction,NULL);
    m_pController = static_cast< OReportModel& >( _rMod ).getController();
    if ( nCommentID )
        m_strComment = String(ModuleRes(nCommentID));
}
OCommentUndoAction::~OCommentUndoAction()
{
    DBG_DTOR(rpt_OCommentUndoAction,NULL);
}
//----------------------------------------------------------------------------
void OCommentUndoAction::Undo()
{
}
//----------------------------------------------------------------------------
void OCommentUndoAction::Redo()
{
}
DBG_NAME( rpt_OUndoContainerAction );
//------------------------------------------------------------------------------
OUndoContainerAction::OUndoContainerAction(SdrModel& _rMod
                                             ,Action _eAction
                                             ,const uno::Reference< container::XIndexContainer > _xContainer
                                             ,const Reference< XInterface > & xElem
                                             ,USHORT _nCommentId)
                      :OCommentUndoAction(_rMod,_nCommentId)
                      ,m_xElement(xElem)
                      ,m_xContainer(_xContainer)
                         ,m_eAction( _eAction )
{
    DBG_CTOR( rpt_OUndoContainerAction,NULL);
    // normalize
    if ( m_eAction == Removed )
        // we now own the element
        m_xOwnElement = m_xElement;
}
//------------------------------------------------------------------------------
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
            SdrObject* pObject = pShape ? pShape->GetSdrObject() : NULL;
            OSL_ENSURE( pObject ? pShape->HasSdrObjectOwnership() && !pObject->IsInserted() : true ,
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
    DBG_DTOR( rpt_OUndoContainerAction,NULL);
}
//------------------------------------------------------------------------------
void OUndoContainerAction::implReInsert( ) SAL_THROW( ( Exception ) )
{
    if ( m_xContainer.is() )
    {
        // insert the element
        m_xContainer->insertByIndex( m_xContainer->getCount(),uno::makeAny(m_xElement) );
    }
    // we don't own the object anymore
    m_xOwnElement = NULL;
}

//------------------------------------------------------------------------------
void OUndoContainerAction::implReRemove( ) SAL_THROW( ( Exception ) )
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

//------------------------------------------------------------------------------
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
                OSL_ENSURE(0,"Illegal case value");
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OUndoContainerAction::Undo: caught an exception!" );
        }
    }
}

//------------------------------------------------------------------------------
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
                OSL_ENSURE(0,"Illegal case value");
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OUndoContainerAction::Redo: caught an exception!" );
        }
    }
}
// -----------------------------------------------------------------------------
OUndoGroupSectionAction::OUndoGroupSectionAction(SdrModel& _rMod
                                             ,Action _eAction
                                             ,::std::mem_fun_t< uno::Reference< report::XSection >
                                                    ,OGroupHelper> _pMemberFunction
                                             ,const uno::Reference< report::XGroup >& _xGroup
                                             ,const Reference< XInterface > & xElem
                                             ,USHORT _nCommentId)
:OUndoContainerAction(_rMod,_eAction,NULL,xElem,_nCommentId)
,m_aGroupHelper(_xGroup)
,m_pMemberFunction(_pMemberFunction)
{
}
//------------------------------------------------------------------------------
void OUndoGroupSectionAction::implReInsert( ) SAL_THROW( ( Exception ) )
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
    m_xOwnElement = NULL;
}

//------------------------------------------------------------------------------
void OUndoGroupSectionAction::implReRemove( ) SAL_THROW( ( Exception ) )
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
//----------------------------------------------------------------------------
OUndoReportSectionAction::OUndoReportSectionAction(SdrModel& _rMod
                                             ,Action _eAction
                                             ,::std::mem_fun_t< uno::Reference< report::XSection >
                                                ,OReportHelper> _pMemberFunction
                                             ,const uno::Reference< report::XReportDefinition >& _xReport
                                             ,const Reference< XInterface > & xElem
                                             ,USHORT _nCommentId)
:OUndoContainerAction(_rMod,_eAction,NULL,xElem,_nCommentId)
,m_aReportHelper(_xReport)
,m_pMemberFunction(_pMemberFunction)
{
}
//------------------------------------------------------------------------------
void OUndoReportSectionAction::implReInsert( ) SAL_THROW( ( Exception ) )
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
    m_xOwnElement = NULL;
}

//------------------------------------------------------------------------------
void OUndoReportSectionAction::implReRemove( ) SAL_THROW( ( Exception ) )
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
//------------------------------------------------------------------------------
ORptUndoPropertyAction::ORptUndoPropertyAction(SdrModel& rNewMod, const PropertyChangeEvent& evt)
                     :OCommentUndoAction(rNewMod,0)
                     ,m_xObj(evt.Source, UNO_QUERY)
                     ,m_aPropertyName(evt.PropertyName)
                     ,m_aNewValue(evt.NewValue)
                     ,m_aOldValue(evt.OldValue)
{
}
//------------------------------------------------------------------------------
void ORptUndoPropertyAction::Undo()
{
    setProperty(sal_True);
}

//------------------------------------------------------------------------------
void ORptUndoPropertyAction::Redo()
{
    setProperty(sal_False);
}
// -----------------------------------------------------------------------------
Reference< XPropertySet> ORptUndoPropertyAction::getObject()
{
    return m_xObj;
}
// -----------------------------------------------------------------------------
void ORptUndoPropertyAction::setProperty(sal_Bool _bOld)
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
            OSL_ENSURE( sal_False, "ORptUndoPropertyAction::Redo: caught an exception!" );
        }
    }
}

//------------------------------------------------------------------------------
String ORptUndoPropertyAction::GetComment() const
{
    String aStr(ModuleRes(RID_STR_UNDO_PROPERTY));

    aStr.SearchAndReplace( '#', m_aPropertyName );
    return aStr;
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
Reference< XPropertySet> OUndoPropertyGroupSectionAction::getObject()
{
    return m_pMemberFunction(&m_aGroupHelper).get();
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
Reference< XPropertySet> OUndoPropertyReportSectionAction::getObject()
{
    return m_pMemberFunction(&m_aReportHelper).get();
}
// -----------------------------------------------------------------------------

class OXUndoEnvironmentImpl
{
    OXUndoEnvironmentImpl(OXUndoEnvironmentImpl&);
    void operator =(OXUndoEnvironmentImpl&);
public:
    OReportModel&                                       m_rModel;
    PropertySetInfoCache                                m_aPropertySetCache;
    FormatNormalizer                                    m_aFormatNormalizer;
    ConditionUpdater                                    m_aConditionUpdater;
    ::osl::Mutex                                        m_aMutex;
    ::std::vector< uno::Reference< container::XChild> > m_aSections;
    oslInterlockedCount                                 m_nLocks;
    sal_Bool                                            m_bReadOnly;

    OXUndoEnvironmentImpl(OReportModel& _rModel);
};

OXUndoEnvironmentImpl::OXUndoEnvironmentImpl(OReportModel& _rModel) : m_rModel(_rModel)
        ,m_aFormatNormalizer( _rModel )
        ,m_aConditionUpdater()
        ,m_nLocks(0)
        ,m_bReadOnly(sal_False)
{
}
//------------------------------------------------------------------------------
DBG_NAME( rpt_OXUndoEnvironment );
//------------------------------------------------------------------------------
OXUndoEnvironment::OXUndoEnvironment(OReportModel& _rModel)
                   :m_pImpl(new OXUndoEnvironmentImpl(_rModel) )
{
    DBG_CTOR( rpt_OXUndoEnvironment,NULL);
    StartListening(m_pImpl->m_rModel);
}

//------------------------------------------------------------------------------
OXUndoEnvironment::~OXUndoEnvironment()
{
    DBG_DTOR( rpt_OXUndoEnvironment,NULL);
}
// -----------------------------------------------------------------------------
void OXUndoEnvironment::Lock()
{
    OSL_ENSURE(m_refCount,"Illegall call to dead object!");
    osl_incrementInterlockedCount( &m_pImpl->m_nLocks );
}
void OXUndoEnvironment::UnLock()
{
    OSL_ENSURE(m_refCount,"Illegall call to dead object!");

    osl_decrementInterlockedCount( &m_pImpl->m_nLocks );
}
sal_Bool OXUndoEnvironment::IsLocked() const { return m_pImpl->m_nLocks != 0; }
sal_Bool OXUndoEnvironment::IsReadOnly() const {return m_pImpl->m_bReadOnly;}
void OXUndoEnvironment::SetReadOnly( sal_Bool bRead, const Accessor& ) { m_pImpl->m_bReadOnly = bRead; }
// -----------------------------------------------------------------------------
void OXUndoEnvironment::RemoveSection(OReportPage* _pPage)
{
    if ( _pPage )
    {
        Reference< XInterface > xSection(_pPage->getSection());
        if ( xSection.is() )
            RemoveElement( xSection );
    }
}
//------------------------------------------------------------------------------
void OXUndoEnvironment::Clear(const Accessor& /*_r*/)
{
    OUndoEnvLock aLock(*this);

#if OSL_DEBUG_LEVEL > 0
    PropertySetInfoCache::iterator aIter = m_pImpl->m_aPropertySetCache.begin();
    PropertySetInfoCache::iterator aEnd = m_pImpl->m_aPropertySetCache.end();
    int ndbg_len = m_pImpl->m_aPropertySetCache.size();
    ndbg_len = ndbg_len;
    for (int idbg_ = 0; aIter != aEnd; ++aIter,++idbg_)
    {
        uno::Reference<beans::XPropertySet> xProp(aIter->first,uno::UNO_QUERY);
        xProp->getPropertySetInfo();
        int nlen = aIter->second.size();
        nlen = nlen;
    }
#endif
    m_pImpl->m_aPropertySetCache.clear();

    sal_uInt16 nCount = m_pImpl->m_rModel.GetPageCount();
    sal_uInt16 i;
    for (i = 0; i < nCount; i++)
    {
        OReportPage* pPage = PTR_CAST( OReportPage, m_pImpl->m_rModel.GetPage(i) );
        RemoveSection(pPage);
    }

    nCount = m_pImpl->m_rModel.GetMasterPageCount();
    for (i = 0; i < nCount; i++)
    {
        OReportPage* pPage = PTR_CAST( OReportPage, m_pImpl->m_rModel.GetMasterPage(i) );
        RemoveSection(pPage);
    }

    m_pImpl->m_aSections.clear();

    if (IsListening(m_pImpl->m_rModel))
        EndListening(m_pImpl->m_rModel);
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::ModeChanged()
{
    m_pImpl->m_bReadOnly = !m_pImpl->m_bReadOnly;

    if (!m_pImpl->m_bReadOnly)
        StartListening(m_pImpl->m_rModel);
    else
        EndListening(m_pImpl->m_rModel);
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (rHint.ISA(SfxSimpleHint) && ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_MODECHANGED )
        ModeChanged();
}
// -----------------------------------------------------------------------------
//  XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OXUndoEnvironment::disposing(const EventObject& e) throw( RuntimeException )
{
    // check if it's an object we have cached informations about
    Reference< XPropertySet > xSourceSet(e.Source, UNO_QUERY);
    if ( xSourceSet.is() )
    {
        uno::Reference< report::XSection> xSection(xSourceSet,uno::UNO_QUERY);
        if ( xSection.is() )
            RemoveSection(xSection);
        else
            RemoveElement(xSourceSet);
        /*if (!m_pImpl->m_aPropertySetCache.empty())
            m_pImpl->m_aPropertySetCache.erase(xSourceSet);*/
    }
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
void SAL_CALL OXUndoEnvironment::propertyChange( const PropertyChangeEvent& _rEvent ) throw(uno::RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( m_pImpl->m_aMutex );

    if ( IsLocked() )
        return;

    Reference< XPropertySet >  xSet( _rEvent.Source, UNO_QUERY );
    if (!xSet.is())
        return;

    dbaui::OSingleDocumentController* pController = m_pImpl->m_rModel.getController();
    if ( !pController )
        return;

    // no Undo for transient and readonly props.
    // let's see if we know something about the set
#if OSL_DEBUG_LEVEL > 0
    int nlen = m_pImpl->m_aPropertySetCache.size();
    nlen = nlen;
#endif
    PropertySetInfoCache::iterator aSetPos = m_pImpl->m_aPropertySetCache.find(xSet);
    if (aSetPos == m_pImpl->m_aPropertySetCache.end())
    {
        AllProperties aNewEntry;
        aSetPos = m_pImpl->m_aPropertySetCache.insert(PropertySetInfoCache::value_type(xSet,aNewEntry)).first;
        DBG_ASSERT(aSetPos != m_pImpl->m_aPropertySetCache.end(), "OXUndoEnvironment::propertyChange : just inserted it ... why it's not there ?");
    }
    if ( aSetPos == m_pImpl->m_aPropertySetCache.end() )
        return;

    // now we have access to the cached info about the set
    // let's see what we know about the property
    AllProperties& rPropInfos = aSetPos->second;
    AllPropertiesIterator aPropertyPos = rPropInfos.find( _rEvent.PropertyName );
    if (aPropertyPos == rPropInfos.end())
    {   // nothing 'til now ... have to change this ....
        // the attributes
        INT32 nAttributes = xSet->getPropertySetInfo()->getPropertyByName( _rEvent.PropertyName ).Attributes;
        bool bTransReadOnly = ((nAttributes & PropertyAttribute::READONLY) != 0) || ((nAttributes & PropertyAttribute::TRANSIENT) != 0);

        // insert the new entry
        aPropertyPos = rPropInfos.insert( AllProperties::value_type( _rEvent.PropertyName, bTransReadOnly ) ).first;
        DBG_ASSERT(aPropertyPos != rPropInfos.end(), "OXUndoEnvironment::propertyChange : just inserted it ... why it's not there ?");
    }

    implSetModified();

    // now we have access to the cached info about the property affected
    // and are able to decide wether or not we need an undo action

    // no UNDO for transient/readonly properties
    if ( aPropertyPos->second )
        return;

    // give components with sub responsibilities a chance
    m_pImpl->m_aFormatNormalizer.notifyPropertyChange( _rEvent );
    m_pImpl->m_aConditionUpdater.notifyPropertyChange( _rEvent );

    aGuard.clear();
    // TODO: this is a potential race condition: two threads here could in theory
    // add their undo actions out-of-order

    ::vos::OClearableGuard aSolarGuard( Application::GetSolarMutex() );
    ORptUndoPropertyAction* pUndo = NULL;
    try
    {
        uno::Reference< report::XSection> xSection( xSet, uno::UNO_QUERY );
        if ( xSection.is() )
        {
            uno::Reference< report::XGroup> xGroup = xSection->getGroup();
            if ( xGroup.is() )
                pUndo = new OUndoPropertyGroupSectionAction( m_pImpl->m_rModel, _rEvent, OGroupHelper::getMemberFunction( xSection ), xGroup );
            else
                pUndo = new OUndoPropertyReportSectionAction( m_pImpl->m_rModel, _rEvent, OReportHelper::getMemberFunction( xSection ), xSection->getReportDefinition() );
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( pUndo == NULL )
        pUndo = new ORptUndoPropertyAction( m_pImpl->m_rModel, _rEvent );

    pController->addUndoActionAndInvalidate(pUndo);
    pController->InvalidateAll();
}
// -----------------------------------------------------------------------------
::std::vector< uno::Reference< container::XChild> >::const_iterator OXUndoEnvironment::getSection(const Reference<container::XChild>& _xContainer) const
{
    ::std::vector< uno::Reference< container::XChild> >::const_iterator aFind = m_pImpl->m_aSections.end();
    if ( _xContainer.is() )
    {
        aFind = ::std::find(m_pImpl->m_aSections.begin(),m_pImpl->m_aSections.end(),_xContainer);

        if ( aFind == m_pImpl->m_aSections.end() )
        {
            Reference<container::XChild> xParent(_xContainer->getParent(),uno::UNO_QUERY);
            aFind = getSection(xParent);
        }
    }
    return aFind;
}
// XContainerListener
//------------------------------------------------------------------------------
void SAL_CALL OXUndoEnvironment::elementInserted(const ContainerEvent& evt) throw(uno::RuntimeException)
{
    ::vos::OClearableGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    // neues Object zum lauschen
    Reference< uno::XInterface >  xIface( evt.Element, UNO_QUERY );
    if ( !IsLocked() )
    {
        Reference< report::XReportComponent >  xReportComponent( xIface, UNO_QUERY );
        if ( xReportComponent.is() )
        {
            Reference< report::XSection > xContainer(evt.Source,uno::UNO_QUERY);

            ::std::vector< uno::Reference< container::XChild> >::const_iterator aFind = getSection(xContainer.get());

            if ( aFind != m_pImpl->m_aSections.end() )
            {
                OUndoEnvLock aLock(*this);
                try
                {
                    OReportPage* pPage = m_pImpl->m_rModel.getPage(uno::Reference< report::XSection>(*aFind,uno::UNO_QUERY));
                    OSL_ENSURE(pPage,"No page could be found for section!");
                    if ( pPage )
                        pPage->insertObject(xReportComponent);
                }
                catch(uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

            }
        }
        else
        {
            uno::Reference< report::XFunctions> xContainer(evt.Source,uno::UNO_QUERY);
            if ( xContainer.is() )
            {
                dbaui::OSingleDocumentController* pController = m_pImpl->m_rModel.getController();
                pController->addUndoActionAndInvalidate(new OUndoContainerAction(m_pImpl->m_rModel
                                                                                ,rptui::Inserted
                                                                                ,xContainer.get()
                                                                                ,xIface
                                                                                ,RID_STR_UNDO_ADDFUNCTION));
            }
        }
    }

    AddElement(xIface);

    implSetModified();
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::implSetModified()
{
    //if ( !IsLocked() )
    m_pImpl->m_rModel.SetModified( sal_True );
}

//------------------------------------------------------------------------------
void SAL_CALL OXUndoEnvironment::elementReplaced(const ContainerEvent& evt) throw(uno::RuntimeException)
{
    ::vos::OClearableGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    Reference< XInterface >  xIface(evt.ReplacedElement,uno::UNO_QUERY);
    OSL_ENSURE(xIface.is(), "OXUndoEnvironment::elementReplaced: invalid container notification!");
    RemoveElement(xIface);

    xIface.set(evt.Element,uno::UNO_QUERY);
    AddElement(xIface);

    implSetModified();
}

//------------------------------------------------------------------------------
void SAL_CALL OXUndoEnvironment::elementRemoved(const ContainerEvent& evt) throw(uno::RuntimeException)
{
    ::vos::OClearableGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    Reference< uno::XInterface >  xIface( evt.Element, UNO_QUERY );
    if ( !IsLocked() )
    {
        Reference< report::XSection > xContainer(evt.Source,uno::UNO_QUERY);
        ::std::vector< uno::Reference< container::XChild> >::const_iterator aFind = getSection(xContainer.get());

        Reference< report::XReportComponent >  xReportComponent( xIface, UNO_QUERY );
        if ( aFind != m_pImpl->m_aSections.end() && xReportComponent.is() )
        {
            OXUndoEnvironment::OUndoEnvLock aLock(*this);
            try
            {
                OReportPage* pPage = m_pImpl->m_rModel.getPage(uno::Reference< report::XSection >( *aFind, uno::UNO_QUERY_THROW ) );
                OSL_ENSURE( pPage, "OXUndoEnvironment::elementRemoved: no page for the section!" );
                if ( pPage )
                    pPage->removeSdrObject(xReportComponent);
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        else
        {
            uno::Reference< report::XFunctions> xFunctions(evt.Source,uno::UNO_QUERY);
            if ( xFunctions.is() )
            {
                dbaui::OSingleDocumentController* pController = m_pImpl->m_rModel.getController();
                pController->addUndoActionAndInvalidate(new OUndoContainerAction(m_pImpl->m_rModel
                                                                                ,rptui::Removed
                                                                                ,xFunctions.get()
                                                                                ,xIface
                                                                                ,RID_STR_UNDO_ADDFUNCTION));
            }
        }
    }

    if ( xIface.is() )
        RemoveElement(xIface);

    implSetModified();
}

//------------------------------------------------------------------------------
void SAL_CALL OXUndoEnvironment::modified( const EventObject& /*aEvent*/ ) throw (RuntimeException)
{
    implSetModified();
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::AddSection(const Reference< report::XSection > & _xSection)
{
    OUndoEnvLock aLock(*this);
    try
    {
        uno::Reference<container::XChild> xChild = _xSection.get();
        uno::Reference<report::XGroup> xGroup(xChild->getParent(),uno::UNO_QUERY);
        m_pImpl->m_aSections.push_back(xChild);
        Reference< XInterface >  xInt(_xSection);
        AddElement(xInt);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::RemoveSection(const Reference< report::XSection > & _xSection)
{
    OUndoEnvLock aLock(*this);
    try
    {
        uno::Reference<container::XChild> xChild(_xSection.get());
        m_pImpl->m_aSections.erase(::std::remove(m_pImpl->m_aSections.begin(),m_pImpl->m_aSections.end(),
            xChild), m_pImpl->m_aSections.end());
        Reference< XInterface >  xInt(_xSection);
        RemoveElement(xInt);
    }
    catch(uno::Exception&){}
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::TogglePropertyListening(const Reference< XInterface > & Element)
{
    // am Container horchen
    Reference< XIndexAccess >  xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        Reference< XInterface > xInterface;
        sal_Int32 nCount = xContainer->getCount();
        for(sal_Int32 i = 0;i != nCount;++i)
        {
            xInterface.set(xContainer->getByIndex( i ),uno::UNO_QUERY);
            TogglePropertyListening(xInterface);
        }
    }

    Reference< XPropertySet >  xSet(Element, UNO_QUERY);
    if (xSet.is())
    {
        if (!m_pImpl->m_bReadOnly)
            xSet->addPropertyChangeListener( ::rtl::OUString(), this );
        else
            xSet->removePropertyChangeListener( ::rtl::OUString(), this );
    }
}


//------------------------------------------------------------------------------
void OXUndoEnvironment::switchListening( const Reference< XIndexAccess >& _rxContainer, bool _bStartListening ) SAL_THROW(())
{
    OSL_PRECOND( _rxContainer.is(), "OXUndoEnvironment::switchListening: invalid container!" );
    if ( !_rxContainer.is() )
        return;

    try
    {
        // also handle all children of this element
        Reference< XInterface > xInterface;
        sal_Int32 nCount = _rxContainer->getCount();
        for(sal_Int32 i = 0;i != nCount;++i)
        {
            xInterface.set(_rxContainer->getByIndex( i ),uno::UNO_QUERY);
            if ( _bStartListening )
                AddElement( xInterface );
            else
                RemoveElement( xInterface );
        }

        // be notified of any changes in the container elements
        Reference< XContainer > xSimpleContainer( _rxContainer, UNO_QUERY );
        // OSL_ENSURE( xSimpleContainer.is(), "OXUndoEnvironment::switchListening: how are we expected to be notified of changes in the container?" );
        if ( xSimpleContainer.is() )
            if ( _bStartListening )
                xSimpleContainer->addContainerListener( this );
            else
                xSimpleContainer->removeContainerListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::switchListening( const Reference< XInterface >& _rxObject, bool _bStartListening ) SAL_THROW(())
{
    OSL_PRECOND( _rxObject.is(), "OXUndoEnvironment::switchListening: how should I listen at a NULL object?" );

    try
    {
        if ( !m_pImpl->m_bReadOnly )
        {
            Reference< XPropertySet > xProps( _rxObject, UNO_QUERY );
            if ( xProps.is() )
                if ( _bStartListening )
                    xProps->addPropertyChangeListener( ::rtl::OUString(), this );
                else
                    xProps->removePropertyChangeListener( ::rtl::OUString(), this );
        }

        Reference< XModifyBroadcaster > xBroadcaster( _rxObject, UNO_QUERY );
        if ( xBroadcaster.is() )
            if ( _bStartListening )
                xBroadcaster->addModifyListener( this );
            else
                xBroadcaster->removeModifyListener( this );
    }
    catch( const Exception& )
    {
        //OSL_ENSURE( sal_False, "OXUndoEnvironment::switchListening: caught an exception!" );
    }
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::AddElement(const Reference< XInterface >& _rxElement )
{
    m_pImpl->m_aFormatNormalizer.notifyElementInserted( _rxElement );

    // if it's a container, start listening at all elements
    Reference< XIndexAccess > xContainer( _rxElement, UNO_QUERY );
    if ( xContainer.is() )
        switchListening( xContainer, true );

    switchListening( _rxElement, true );
}

//------------------------------------------------------------------------------
void OXUndoEnvironment::RemoveElement(const Reference< XInterface >& _rxElement)
{
    uno::Reference<beans::XPropertySet> xProp(_rxElement,uno::UNO_QUERY);
    if (!m_pImpl->m_aPropertySetCache.empty())
        m_pImpl->m_aPropertySetCache.erase(xProp);
    switchListening( _rxElement, false );

    Reference< XIndexAccess > xContainer( _rxElement, UNO_QUERY );
    if ( xContainer.is() )
        switchListening( xContainer, false );
}

//============================================================================
} // rptui
//============================================================================

