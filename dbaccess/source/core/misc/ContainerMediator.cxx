/*************************************************************************
 *
 *  $RCSfile: ContainerMediator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:15:47 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


//........................................................................
namespace dbaccess
{
//........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
DBG_NAME(OContainerMediator)
OContainerMediator::OContainerMediator( const Reference< XContainer >& _xContainer
                                       ,const Reference< XNameAccess >& _xSettings
                                       ,sal_Bool _bTables)
    : m_xContainer(_xContainer)
    , m_xSettings(_xSettings)
    , m_bTables(_bTables)
{
    DBG_CTOR(OContainerMediator,NULL);
    osl_incrementInterlockedCount(&m_refCount);
    {
        m_xContainer->addContainerListener(this);
        Reference< XContainer > xContainer(m_xSettings, UNO_QUERY);
        if ( xContainer.is() )
            xContainer->addContainerListener(this);
    }
    osl_decrementInterlockedCount( &m_refCount );
}
// -----------------------------------------------------------------------------
OContainerMediator::~OContainerMediator()
{
    DBG_DTOR(OContainerMediator,NULL);
    osl_incrementInterlockedCount(&m_refCount);
    {
        if ( m_xContainer.is() )
            m_xContainer->removeContainerListener(this);
        Reference< XContainer > xContainer(m_xSettings, UNO_QUERY);
        if ( xContainer.is() )
            xContainer->removeContainerListener(this);
    }
    osl_decrementInterlockedCount( &m_refCount );
}
// -----------------------------------------------------------------------------
void SAL_CALL OContainerMediator::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( _rEvent.Source == m_xSettings && m_xSettings.is() )
    {
        ::rtl::OUString sElementName;
        _rEvent.Accessor >>= sElementName;
        PropertyForwardList::iterator aFind = m_aForwardList.find(sElementName);
        if ( aFind != m_aForwardList.end() )
        {
            Reference< XPropertySet> xDest(_rEvent.Element,UNO_QUERY);
            aFind->second->setDefinition(xDest);
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OContainerMediator::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( _rEvent.Source == m_xContainer && m_xSettings.is() )
    {
        ::rtl::OUString sElementName;
        _rEvent.Accessor >>= sElementName;
        m_aForwardList.erase(sElementName);
        Reference<XNameContainer> xNameContainer(m_xSettings,UNO_QUERY);
        if ( xNameContainer.is() && m_xSettings->hasByName(sElementName) )
            xNameContainer->removeByName(sElementName);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OContainerMediator::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
{

}
// -----------------------------------------------------------------------------
void SAL_CALL OContainerMediator::disposing( const EventObject& Source ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( Source.Source == m_xContainer || Source.Source == m_xSettings )
    {
        Reference< XContainer > xContainer(m_xSettings, UNO_QUERY);
        if ( xContainer.is() )
        {
            xContainer->removeContainerListener(this);
        }
        m_xSettings = NULL;

        if ( m_xContainer.is() )
        {
            m_xContainer->removeContainerListener(this);
            m_xContainer = NULL;
        }
    }
}
// -----------------------------------------------------------------------------
void OContainerMediator::notifyElementCreated(const ::rtl::OUString& _sName,const Reference<XPropertySet>& _xDest)
{
    PropertyForwardList::iterator aFind = m_aForwardList.find(_sName);
    if ( aFind == m_aForwardList.end() && m_xSettings.is() )
    {
        if ( m_xSettings->hasByName(_sName) )
        {
            Reference<XPropertySet> xSetting(m_xSettings->getByName(_sName),UNO_QUERY);
            if ( xSetting.is() )
                ::comphelper::copyProperties(xSetting,_xDest);
        }

        ::std::vector< ::rtl::OUString> aPropertyList;
        if ( m_bTables )
        {
            aPropertyList.reserve(26);
            aPropertyList.push_back(PROPERTY_FILTER);
            aPropertyList.push_back(PROPERTY_ORDER);
            aPropertyList.push_back(PROPERTY_APPLYFILTER);
            aPropertyList.push_back(PROPERTY_FONT);
            aPropertyList.push_back(PROPERTY_ROW_HEIGHT);
            aPropertyList.push_back(PROPERTY_TEXTCOLOR);
            aPropertyList.push_back(PROPERTY_TEXTLINECOLOR);
            aPropertyList.push_back(PROPERTY_TEXTEMPHASIS);
            aPropertyList.push_back(PROPERTY_TEXTRELIEF);
            aPropertyList.push_back(PROPERTY_FONTNAME);
            aPropertyList.push_back(PROPERTY_FONTHEIGHT);
            aPropertyList.push_back(PROPERTY_FONTWIDTH);
            aPropertyList.push_back(PROPERTY_FONTSTYLENAME);
            aPropertyList.push_back(PROPERTY_FONTFAMILY);
            aPropertyList.push_back(PROPERTY_FONTCHARSET);
            aPropertyList.push_back(PROPERTY_FONTPITCH);
            aPropertyList.push_back(PROPERTY_FONTCHARWIDTH);
            aPropertyList.push_back(PROPERTY_FONTWEIGHT);
            aPropertyList.push_back(PROPERTY_FONTSLANT);
            aPropertyList.push_back(PROPERTY_FONTUNDERLINE);
            aPropertyList.push_back(PROPERTY_FONTSTRIKEOUT);
            aPropertyList.push_back(PROPERTY_FONTORIENTATION);
            aPropertyList.push_back(PROPERTY_FONTKERNING);
            aPropertyList.push_back(PROPERTY_FONTWORDLINEMODE);
            aPropertyList.push_back(PROPERTY_FONTTYPE);
        }
        else
        {
            aPropertyList.reserve(9);
            aPropertyList.push_back(PROPERTY_ALIGN);
            aPropertyList.push_back(PROPERTY_NUMBERFORMAT);
            aPropertyList.push_back(PROPERTY_RELATIVEPOSITION);
            aPropertyList.push_back(PROPERTY_WIDTH);
            aPropertyList.push_back(PROPERTY_HIDDEN);
            aPropertyList.push_back(PROPERTY_CONTROLMODEL);
            aPropertyList.push_back(PROPERTY_HELPTEXT);
            aPropertyList.push_back(PROPERTY_CONTROLDEFAULT);
        }

        OPropertyForward* pForward = new OPropertyForward(_xDest,m_xSettings,_sName,aPropertyList);
        m_aForwardList.insert( PropertyForwardList::value_type(_sName,pForward));
    }
    else
    {
    }
}
// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................
