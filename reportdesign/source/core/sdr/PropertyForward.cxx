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
#include <PropertyForward.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <strings.hxx>

namespace rptui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

OPropertyMediator::OPropertyMediator(const Reference< XPropertySet>& _xSource
                                     ,const Reference< XPropertySet>& _xDest
                                     ,const TPropertyNamePair& _aNameMap
                                     ,bool _bReverse)
                                : OPropertyForward_Base(m_aMutex)
                                ,m_aNameMap(_aNameMap)
                                ,m_xSource(_xSource)
                                ,m_xDest(_xDest)
                                ,m_bInChange(false)
{
    osl_atomic_increment(&m_refCount);
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
                for (const auto& [rName, rPropConv] : m_aNameMap)
                {
                    Property aProp = m_xSourceInfo->getPropertyByName(rName);
                    if (0 == (aProp.Attributes & PropertyAttribute::READONLY))
                    {
                        Any aValue = _xDest->getPropertyValue(rPropConv.first);
                        if ( 0 != (aProp.Attributes & PropertyAttribute::MAYBEVOID) || aValue.hasValue() )
                            _xSource->setPropertyValue(rName, rPropConv.second->operator()(rPropConv.first, aValue));
                    }
                }
            }
            else
            {
                ::comphelper::copyProperties(m_xSource,m_xDest);
                for (const auto& [rName, rPropConv] : m_aNameMap)
                    _xDest->setPropertyValue(rPropConv.first, rPropConv.second->operator()(rPropConv.first, _xSource->getPropertyValue(rName)));
            }
            startListening();
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
    }
    osl_atomic_decrement(&m_refCount);
}

OPropertyMediator::~OPropertyMediator()
{
}

void SAL_CALL OPropertyMediator::propertyChange( const PropertyChangeEvent& evt )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_bInChange )
    {
        m_bInChange = true;
        try
        {
            bool bDest = (evt.Source == m_xDest);
            Reference<XPropertySet> xProp =  bDest ? m_xSource : m_xDest;
            Reference<XPropertySetInfo> xPropInfo = bDest ? m_xSourceInfo : m_xDestInfo;
            if ( xProp.is() && xPropInfo.is() )
            {
                if ( xPropInfo->hasPropertyByName(evt.PropertyName) )
                    xProp->setPropertyValue(evt.PropertyName,evt.NewValue);
                else
                {
                    TPropertyNamePair::const_iterator aFind = m_aNameMap.find(evt.PropertyName);
                    OUString sPropName;
                    if ( aFind != m_aNameMap.end() )
                        sPropName = aFind->second.first;
                    else
                    {
                        aFind = ::std::find_if(
                            m_aNameMap.begin(),
                            m_aNameMap.end(),
                            [&evt] (const TPropertyNamePair::value_type& namePair) {
                                return namePair.second.first == evt.PropertyName;
                            });
                        if ( aFind != m_aNameMap.end() )
                            sPropName = aFind->first;
                    }
                    if (aFind != m_aNameMap.end() && !sPropName.isEmpty() && xPropInfo->hasPropertyByName(sPropName))
                        xProp->setPropertyValue(sPropName,aFind->second.second->operator()(sPropName,evt.NewValue));
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
        catch(Exception&)
        {
            OSL_FAIL("Exception caught!");
        }
        m_bInChange = false;
    }
}

void SAL_CALL OPropertyMediator::disposing( const css::lang::EventObject& /*_rSource*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    disposing();
}

void SAL_CALL OPropertyMediator::disposing()
{
    stopListening();
    m_xSource.clear();
    m_xSourceInfo.clear();
    m_xDest.clear();
    m_xDestInfo.clear();
}

void OPropertyMediator::stopListening()
{
    if ( m_xSource.is() )
        m_xSource->removePropertyChangeListener(OUString(), this);
    if ( m_xDest.is() )
        m_xDest->removePropertyChangeListener(OUString(), this);
}

void OPropertyMediator::startListening()
{
    if ( m_xSource.is() )
        m_xSource->addPropertyChangeListener(OUString(), this);
    if ( m_xDest.is() )
        m_xDest->addPropertyChangeListener(OUString(), this);
}


}   // namespace dbaccess


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
