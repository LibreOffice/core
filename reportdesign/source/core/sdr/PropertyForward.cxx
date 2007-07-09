/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyForward.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:16 $
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
#ifndef RPTUI_PROPERTYSETFORWARD_HXX
#include "PropertyForward.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include "corestrings.hrc"

//........................................................................
namespace rptui
{
//........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace reportdesign;

DBG_NAME( rpt_OPropertyMediator )
OPropertyMediator::OPropertyMediator(const Reference< XPropertySet>& _xSource
                                     ,const Reference< XPropertySet>& _xDest
                                     ,const TPropertyNamePair& _aNameMap
                                     ,sal_Bool _bReverse)
                                : OPropertyForward_Base(m_aMutex)
                                ,m_aNameMap(_aNameMap)
                                ,m_xSource(_xSource)
                                ,m_xDest(_xDest)
                                ,m_bInChange(sal_False)
{
    DBG_CTOR( rpt_OPropertyMediator,NULL);
    osl_incrementInterlockedCount(&m_refCount);
    OSL_ENSURE(m_xDest.is(),"Dest is NULL!");
    OSL_ENSURE(m_xSource.is(),"Source is NULL!");
    if ( m_xDest.is() && m_xSource.is() )
    {
        try
        {
            m_xDestInfo = m_xDest->getPropertySetInfo();
            m_xSourceInfo = m_xSource->getPropertySetInfo();
            if ( _bReverse )
            {
                ::comphelper::copyProperties(m_xDest,m_xSource);
                TPropertyNamePair::iterator aIter = m_aNameMap.begin();
                TPropertyNamePair::iterator aEnd = m_aNameMap.end();
                for (; aIter != aEnd; ++aIter)
                {
                    Any aValue = _xDest->getPropertyValue(aIter->second);
                    Property aProp = m_xSourceInfo->getPropertyByName(aIter->first);
                    if (0 == (aProp.Attributes & PropertyAttribute::READONLY))
                    {
                        if ( 0 != (aProp.Attributes & PropertyAttribute::MAYBEVOID) || aValue.hasValue() )
                            _xSource->setPropertyValue(aIter->first,aValue);
                    }
                }
            }
            else
            {
                ::comphelper::copyProperties(m_xSource,m_xDest);
                TPropertyNamePair::iterator aIter = m_aNameMap.begin();
                TPropertyNamePair::iterator aEnd = m_aNameMap.end();
                for (; aIter != aEnd; ++aIter)
                    _xDest->setPropertyValue(aIter->second,_xSource->getPropertyValue(aIter->first));
            }
            startListening();
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OPropertyMediator::OPropertyMediator: caught an exception!");
        }
    } // if ( m_xDest.is() && m_xSource.is() )
    osl_decrementInterlockedCount(&m_refCount);
}
// -----------------------------------------------------------------------------
OPropertyMediator::~OPropertyMediator()
{
    DBG_DTOR( rpt_OPropertyMediator,NULL);
}
// -----------------------------------------------------------------------------
void SAL_CALL OPropertyMediator::propertyChange( const PropertyChangeEvent& evt ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_bInChange )
    {
        m_bInChange = sal_True;
        try
        {
            sal_Bool bDest = (evt.Source == m_xDest);
            Reference<XPropertySet> xProp =  bDest ? m_xSource : m_xDest;
            Reference<XPropertySetInfo> xPropInfo = bDest ? m_xSourceInfo : m_xDestInfo;
            if ( xProp.is() )
            {
                if ( xPropInfo.is() )
                {
                    if ( xPropInfo->hasPropertyByName(evt.PropertyName) )
                        xProp->setPropertyValue(evt.PropertyName,evt.NewValue);
                    else
                    {
                        TPropertyNamePair::iterator aFind = m_aNameMap.find(evt.PropertyName);
                        ::rtl::OUString sPropName;
                        if ( aFind != m_aNameMap.end() )
                            sPropName = aFind->second;
                        else
                        {
                            aFind = ::std::find_if(
                                m_aNameMap.begin(),
                                m_aNameMap.end(),
                                ::std::compose1(
                                ::std::bind2nd(::std::equal_to< ::rtl::OUString >(), evt.PropertyName),
                                    ::std::select2nd<TPropertyNamePair::value_type>()
                                )
                            );
                            if ( aFind != m_aNameMap.end() )
                                sPropName = aFind->first;
                        }
                        if ( sPropName.getLength() && xPropInfo->hasPropertyByName(sPropName) )
                            xProp->setPropertyValue(sPropName,evt.NewValue);
                        else if (   evt.PropertyName == PROPERTY_CHARFONTNAME
                                ||  evt.PropertyName == PROPERTY_CHARFONTSTYLENAME
                                ||  evt.PropertyName == PROPERTY_CHARSTRIKEOUT
                                ||  evt.PropertyName == PROPERTY_CHARWORDMODE
                                ||  evt.PropertyName == PROPERTY_CHARROTATION
                                ||  evt.PropertyName == PROPERTY_CHARSCALEWIDTH
                                ||  evt.PropertyName == PROPERTY_CHARFONTFAMILY
                                ||  evt.PropertyName == PROPERTY_CHARFONTCHARSET
                                ||  evt.PropertyName == PROPERTY_CHARFONTPITCH
                                ||  evt.PropertyName == PROPERTY_CHARHEIGHT
                                ||  evt.PropertyName == PROPERTY_CHARUNDERLINE
                                ||  evt.PropertyName == PROPERTY_CHARWEIGHT
                                ||  evt.PropertyName == PROPERTY_CHARPOSTURE)
                        {
                            xProp->setPropertyValue(PROPERTY_FONTDESCRIPTOR,m_xSource->getPropertyValue(PROPERTY_FONTDESCRIPTOR));
                        }
                    }
                }
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"Exception catched!");
        }
        m_bInChange = sal_False;
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OPropertyMediator::disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    disposing();
}
// -----------------------------------------------------------------------------
void SAL_CALL OPropertyMediator::disposing()
{
    stopListening();
    m_xSource.clear();
    m_xSourceInfo.clear();
    m_xDest.clear();
    m_xDestInfo.clear();
}
// -----------------------------------------------------------------------------
void OPropertyMediator::stopListening()
{
    if ( m_xSource.is() )
        m_xSource->removePropertyChangeListener(::rtl::OUString(), this);
    if ( m_xDest.is() )
        m_xDest->removePropertyChangeListener(::rtl::OUString(), this);
}
// -----------------------------------------------------------------------------
void OPropertyMediator::startListening()
{
    if ( m_xSource.is() )
        m_xSource->addPropertyChangeListener(::rtl::OUString(), this);
    if ( m_xDest.is() )
        m_xDest->addPropertyChangeListener(::rtl::OUString(), this);
}
// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................

