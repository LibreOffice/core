/*************************************************************************
 *
 *  $RCSfile: InterfaceContainer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:06 $
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

#ifndef _FRM_INTERFACE_CONTAINER_HXX_
#include "InterfaceContainer.hxx"
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _UNOTOOLS_EVENTATTACHERMGR_HXX_
#include <unotools/eventattachermgr.hxx>
#endif
#ifndef _UTL_TYPES_HXX_
#include <unotools/types.hxx>
#endif
#ifndef _UNOTOOLS_ENUMHELPER_HXX_
#include <unotools/enumhelper.hxx>
#endif
#ifndef _UTL_PROPERTY_HXX_
#include <unotools/property.hxx>
#endif
#ifndef _UTL_CONTAINER_HXX_
#include <unotools/container.hxx>
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif

#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#ifndef _FRM_RESOURCE_HXX_
#include "frm_resource.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_WRONGFORMATEXCEPTION_HPP_
#include <com/sun/star/io/WrongFormatException.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//------------------------------------------------------------------
OInterfaceContainer::OInterfaceContainer(
                const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory,
                ::osl::Mutex& _rMutex,
                const staruno::Type& _rElementType)
        :m_rMutex(_rMutex)
        ,m_aContainerListeners(_rMutex)
        ,m_aElementType(_rElementType)
        ,m_xServiceFactory(_rxFactory)
{
    m_xEventAttacher = ::utl::createEventAttacherManager(m_xServiceFactory);
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OInterfaceContainer::queryInterface(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn =
        ::cppu::queryInterface(_rType
            ,static_cast<starcontainer::XElementAccess*>(static_cast<starcontainer::XNameAccess*>(this))
            ,static_cast<starcontainer::XIndexAccess*>(this)
            ,static_cast<starcontainer::XNameAccess*>(this)
            ,static_cast<starcontainer::XIndexReplace*>(this)
            ,static_cast<starcontainer::XNameReplace*>(this)
            ,static_cast<starcontainer::XIndexContainer*>(this)
            ,static_cast<starcontainer::XNameContainer*>(this)
            ,static_cast<starcontainer::XEnumerationAccess*>(this)
            ,static_cast<starcontainer::XContainer*>(this)
            ,static_cast<starlang::XEventListener*>(this)
            ,static_cast<starbeans::XPropertyChangeListener*>(this)
            ,static_cast<starscript::XEventAttacherManager*>(this)
        );

    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<stario::XPersistObject*>(this)
        );

    return aReturn;
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> SAL_CALL OInterfaceContainer::getTypes() throw(staruno::RuntimeException)
{
    staruno::Sequence<staruno::Type> aTypes(7);
    aTypes.getArray()[0] = ::getCppuType(static_cast<staruno::Reference<starcontainer::XNameContainer>*>(NULL));
    aTypes.getArray()[1] = ::getCppuType(static_cast<staruno::Reference<starcontainer::XIndexContainer>*>(NULL));
    aTypes.getArray()[2] = ::getCppuType(static_cast<staruno::Reference<starcontainer::XContainer>*>(NULL));
    aTypes.getArray()[3] = ::getCppuType(static_cast<staruno::Reference<starcontainer::XEnumerationAccess>*>(NULL));
    aTypes.getArray()[4] = ::getCppuType(static_cast<staruno::Reference<starscript::XEventAttacherManager>*>(NULL));
    aTypes.getArray()[5] = ::getCppuType(static_cast<staruno::Reference<starbeans::XPropertyChangeListener>*>(NULL));
    aTypes.getArray()[6] = ::getCppuType(static_cast<staruno::Reference<stario::XPersistObject>*>(NULL));

    return aTypes;
}

//------------------------------------------------------------------------------
void OInterfaceContainer::disposing()
{
    // dispose aller elemente
    for (sal_Int32 i = m_aItems.size(); i > 0; --i)
    {
        staruno::Reference<starbeans::XPropertySet>  xSet(m_aItems[i - 1], staruno::UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener(PROPERTY_NAME, this);

        // Eventverknüpfungen aufheben
        InterfaceRef  xIfc(xSet, staruno::UNO_QUERY);
        m_xEventAttacher->detach(i - 1, xIfc);
        m_xEventAttacher->removeEntry(i - 1);

        staruno::Reference<starlang::XComponent>  xComponent(xSet, staruno::UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }
    m_aMap.clear();
    m_aItems.clear();

    starlang::EventObject aEvt(static_cast<starcontainer::XContainer*>(this));
    m_aContainerListeners.disposeAndClear(aEvt);
}

// stario::XPersistObject
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::writeEvents(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
{
    staruno::Reference<stario::XMarkableStream>  xMark(_rxOutStream, staruno::UNO_QUERY);
    sal_Int32 nMark = xMark->createMark();

    sal_Int32 nObjLen = 0;
    _rxOutStream->writeLong(nObjLen);

    staruno::Reference<stario::XPersistObject>  xScripts(m_xEventAttacher, staruno::UNO_QUERY);
    if (xScripts.is())
        xScripts->write(_rxOutStream);

    // feststellen der Laenge
    nObjLen = xMark->offsetToMark(nMark) - 4;
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nObjLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::readEvents(const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_Int32 nCount)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    if (nCount)
    {
        // Scripting Info lesen
        staruno::Reference<stario::XMarkableStream>  xMark(_rxInStream, staruno::UNO_QUERY);
        sal_Int32 nObjLen = _rxInStream->readLong();
        if (nObjLen)
        {
            sal_Int32 nMark = xMark->createMark();
            staruno::Reference<stario::XPersistObject>  xObj(m_xEventAttacher, staruno::UNO_QUERY);
            if (xObj.is())
                xObj->read(_rxInStream);
            xMark->jumpToMark(nMark);
            _rxInStream->skipBytes(nObjLen);
            xMark->deleteMark(nMark);
        }

        // Attachement lesen
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            InterfaceRef  xIfc(m_aItems[i], staruno::UNO_QUERY);
            staruno::Reference<starbeans::XPropertySet>  xSet(xIfc, staruno::UNO_QUERY);
            staruno::Any aHelper;
            aHelper <<= xSet;
            m_xEventAttacher->attach( i, xIfc, aHelper );
        }
    }
    else
    {
        // neuen EventManager
        m_xEventAttacher = ::utl::createEventAttacherManager(m_xServiceFactory);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::write( const staruno::Reference< stario::XObjectOutputStream >& _rxOutStream ) throw(stario::IOException, staruno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    sal_Int32 nLen = m_aItems.size();

    // schreiben der laenge
    _rxOutStream->writeLong(nLen);

    if (nLen)
    {
        // 1. Version
        _rxOutStream->writeShort(0x0001);

        // 2. Objekte
        for (sal_Int32 i = 0; i < nLen; i++)
        {
            staruno::Reference<stario::XPersistObject>  xObj(m_aItems[i], staruno::UNO_QUERY);
            if (xObj.is())
                _rxOutStream->writeObject(xObj);
            else
            {
                // ::com::sun::star::chaos::Error
            }
        }

        // 3. Scripts
        writeEvents(_rxOutStream);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::read( const staruno::Reference< stario::XObjectInputStream >& _rxInStream ) throw(stario::IOException, staruno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );

    // after ::read the object is expected to be in the state it was when ::write was called, so we have
    // to empty ourself here
    // FS - 71598 - 12.01.00
    while (getCount())
        removeByIndex(0);

    // Schreibt nur in Abhaengigkeit der Länge
    sal_Int32 nLen = _rxInStream->readLong();

    if (nLen)
    {
        // 1. Version
        sal_uInt16 nVersion = _rxInStream->readShort();

        // 2. Objekte
        for (sal_Int32 i = 0; i < nLen; i++)
        {
            staruno::Reference<stario::XPersistObject>  xObj;
            try
            {
                xObj = _rxInStream->readObject();
            }
            catch(stario::WrongFormatException& e)
            {
                e;  // make compiler happy
                // the object could not be read
                xObj = NULL;
                // create a dummy starform (so the readEvents below will assign the events to the right controls)
                xObj = staruno::Reference<stario::XPersistObject> (m_xServiceFactory->createInstance(FRM_COMPONENT_HIDDENCONTROL), staruno::UNO_QUERY);
                DBG_ASSERT(xObj.is(), "OInterfaceContainer::read : could not create a substitute for the unknown object !");
                if (!xObj.is())
                    // couldn't handle it ...
                    throw;

                // set some properties describing what we did
                staruno::Reference<starbeans::XPropertySet>  xObjProps(xObj, staruno::UNO_QUERY);
                if (xObjProps.is())
                {
                    try
                    {
                        xObjProps->setPropertyValue(PROPERTY_NAME, staruno::makeAny(FRM_RES_STRING(RID_STR_CONTROL_SUBSTITUTED_NAME)));
                        xObjProps->setPropertyValue(PROPERTY_TAG, staruno::makeAny(FRM_RES_STRING(RID_STR_CONTROL_SUBSTITUTED_EPXPLAIN)));
                    }
                    catch(...)
                    {
                    }
                }
                // 72133 - 09.02.00 - FS
            }
            catch(...)
            {
                // unsere Map leeren
                while (!m_aItems.empty())
                    removeElementsNoEvents(0);

                // und die staruno::Exception nach aussen
                throw;
            }

            if (xObj.is())
            {
                staruno::Any aElement = xObj->queryInterface(m_aElementType);
                if (aElement.hasValue())
                    insert(m_aItems.size(), *static_cast<const InterfaceRef*>(aElement.getValue()), sal_False);
                else
                    ; // form konnte nicht gelesen werden; nyi
            }
        }
    }

    readEvents(_rxInStream, nLen);
}

// starcontainer::XContainer
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::addContainerListener(const staruno::Reference<starcontainer::XContainerListener>& _rxListener) throw( staruno::RuntimeException )
{
    m_aContainerListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeContainerListener(const staruno::Reference<starcontainer::XContainerListener>& _rxListener) throw( staruno::RuntimeException )
{
    m_aContainerListeners.removeInterface(_rxListener);
}

// starlang::XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::disposing(const starlang::EventObject& _rSource) throw( staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );

    OInterfaceArray::iterator j = find(m_aItems.begin(), m_aItems.end(), _rSource.Source);
    if (j != m_aItems.end())
    {
        OInterfaceMap::iterator i = m_aMap.begin();
        while (i != m_aMap.end() && (*i).second != _rSource.Source)
            ++i;

        m_aMap.erase(i);
        m_aItems.erase(j);
    }
}

// starbeans::XPropertyChangeListener
//------------------------------------------------------------------------------
void OInterfaceContainer::propertyChange(const starbeans::PropertyChangeEvent& evt)
{
    if (evt.PropertyName == PROPERTY_NAME)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        OInterfaceMap::iterator i = find(m_aMap.begin(), m_aMap.end(),
            pair<const ::rtl::OUString, InterfaceRef >(::utl::getString(evt.OldValue),evt.Source));
        if (i != m_aMap.end())
        {
            InterfaceRef  xCorrectType((*i).second);
            m_aMap.erase(i);
            m_aMap.insert(pair<const ::rtl::OUString, InterfaceRef >(::utl::getString(evt.NewValue),xCorrectType));
        }
    }
}

// starcontainer::XElementAccess
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OInterfaceContainer::hasElements() throw( staruno::RuntimeException )
{
    return !m_aMap.empty();
}

//------------------------------------------------------------------------------
staruno::Type SAL_CALL OInterfaceContainer::getElementType() throw(staruno::RuntimeException)
{
    return m_aElementType;
}

// starcontainer::XEnumerationAccess
//------------------------------------------------------------------------------
staruno::Reference<starcontainer::XEnumeration> SAL_CALL OInterfaceContainer::createEnumeration() throw( staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    return new ::utl::OEnumerationByIndex(static_cast<starcontainer::XIndexAccess*>(this));
}

// starcontainer::XNameAccess
//------------------------------------------------------------------------------
staruno::Any SAL_CALL OInterfaceContainer::getByName( const ::rtl::OUString& _rName ) throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(_rName);

    if (aPair.first == aPair.second)
        throw starcontainer::NoSuchElementException();

    return staruno::Any(&(*aPair.first).second, m_aElementType);
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OInterfaceContainer::getElementNames() throw(staruno::RuntimeException)
{
    StringSequence aNameList(m_aItems.size());
    ::rtl::OUString* pStringArray = aNameList.getArray();

    for (OInterfaceMap::const_iterator i = m_aMap.begin(); i != m_aMap.end(); ++i, ++pStringArray)
    {
        *pStringArray = (*i).first;
    }
    return aNameList;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OInterfaceContainer::hasByName( const ::rtl::OUString& _rName ) throw(staruno::RuntimeException)
{
    pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(_rName);
    return aPair.first != aPair.second;
}

// starcontainer::XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 OInterfaceContainer::getCount() throw( staruno::RuntimeException )
{
    return m_aItems.size();
}

//------------------------------------------------------------------------------
staruno::Any OInterfaceContainer::getByIndex(sal_Int32 _nIndex) throw( starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException )
{
    if (_nIndex < 0 || _nIndex >= m_aItems.size())
        throw starlang::IndexOutOfBoundsException();

    return staruno::Any(&m_aItems[_nIndex], m_aElementType);
}

//------------------------------------------------------------------------------
void OInterfaceContainer::insert(sal_Int32 _nIndex, const InterfaceRef& xElement, sal_Bool bEvents) throw( starlang::IllegalArgumentException )
{
    // das richtige Interface besorgen
    staruno::Any aCorrectType = xElement->queryInterface(m_aElementType);
    if (!aCorrectType.hasValue())
        throw starlang::IllegalArgumentException();
    InterfaceRef xCorrectType = *static_cast<const InterfaceRef*>(aCorrectType.getValue());

    ::rtl::OUString sName;
    staruno::Reference<starbeans::XPropertySet>  xSet(xElement, staruno::UNO_QUERY);
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            throw starlang::IllegalArgumentException();

        staruno::Any aValue = xSet->getPropertyValue(PROPERTY_NAME);
        aValue >>= sName;
        xSet->addPropertyChangeListener(PROPERTY_NAME, this);
    }
    else
        throw starlang::IllegalArgumentException();

    if (_nIndex > m_aItems.size()) // ermitteln des tatsaechlichen Indexs
    {
        _nIndex = m_aItems.size();
        m_aItems.push_back(xCorrectType);
    }
    else
        m_aItems.insert(m_aItems.begin() + _nIndex, xCorrectType);

    m_aMap.insert(pair<const ::rtl::OUString, InterfaceRef  >(sName,xCorrectType));

    staruno::Reference<starcontainer::XChild>  xChild(xElement, staruno::UNO_QUERY);
    if (xChild.is())
        xChild->setParent(static_cast<starcontainer::XContainer*>(this));

    if (bEvents)
    {
        m_xEventAttacher->insertEntry(_nIndex);
        InterfaceRef  xIfc(xElement, staruno::UNO_QUERY);// wichtig
        staruno::Any aHelper;
        aHelper <<= xSet;
        m_xEventAttacher->attach(_nIndex, xIfc, aHelper);
    }

    // notify derived classes
    implInserted(xCorrectType);

    // notify listeners
    starcontainer::ContainerEvent aEvt;
    aEvt.Source   = static_cast<starcontainer::XContainer*>(this);
    aEvt.Accessor <<= _nIndex;
    aEvt.Element  = staruno::Any(&xCorrectType, m_aElementType);
    NOTIFY_LISTENERS(m_aContainerListeners, starcontainer::XContainerListener, elementInserted, aEvt);
}

//------------------------------------------------------------------------------
void OInterfaceContainer::removeElementsNoEvents(sal_Int32 nIndex)
{
    OInterfaceArray::iterator i = m_aItems.begin() + nIndex;
    InterfaceRef  xElement(*i);

    OInterfaceMap::iterator j = m_aMap.begin();
    while (j != m_aMap.end() && (*j).second != xElement) ++j;

    m_aItems.erase(i);
    m_aMap.erase(j);

    staruno::Reference<starbeans::XPropertySet>  xSet(xElement, staruno::UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    staruno::Reference<starcontainer::XChild>  xChild(xElement, staruno::UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());
}

// starcontainer::XIndexContainer
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertByIndex(sal_Int32 _nIndex, const staruno::Any& Element) throw(starlang::IllegalArgumentException, starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    if (Element.getValueType().getTypeClass() != staruno::TypeClass_INTERFACE)
        throw starlang::IllegalArgumentException();

    ::osl::MutexGuard aGuard( m_rMutex );
    insert(_nIndex, InterfaceRef (*(InterfaceRef *)Element.getValue()), sal_True);
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::replaceByIndex(sal_Int32 _nIndex, const staruno::Any& Element) throw( starlang::IllegalArgumentException, starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException )
{
    if (Element.getValueType().getTypeClass() != staruno::TypeClass_INTERFACE)
        throw starlang::IllegalArgumentException();

    ::osl::MutexGuard aGuard( m_rMutex );
    if (_nIndex < 0 || _nIndex >= m_aItems.size())
        throw starlang::IndexOutOfBoundsException();


    InterfaceRef  xOldElement(m_aItems[_nIndex]);
    InterfaceRef  xNewElement(*(InterfaceRef *)Element.getValue());

    OInterfaceMap::iterator j = m_aMap.begin();
    while (j != m_aMap.end() && (*j).second != xOldElement) ++j;

    // Eventverknüpfungen aufheben
    InterfaceRef  xIfc(xOldElement, staruno::UNO_QUERY);// wichtig
    m_xEventAttacher->detach(_nIndex, xIfc);
    m_xEventAttacher->removeEntry(_nIndex);

    staruno::Reference<starbeans::XPropertySet>  xSet(xOldElement, staruno::UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    staruno::Reference<starcontainer::XChild>  xChild(xOldElement, staruno::UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());

    // neue einfuegen
    ::rtl::OUString sName;
    xSet = staruno::Reference<starbeans::XPropertySet> (xNewElement, staruno::UNO_QUERY);
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            throw starlang::IllegalArgumentException();

        staruno::Any aValue = xSet->getPropertyValue(PROPERTY_NAME);
        aValue >>= sName;
        xSet->addPropertyChangeListener(PROPERTY_NAME, this);
    }
    else
        throw starlang::IllegalArgumentException();

    // remove the old one
    m_aMap.erase(j);

    // insert the new one
    m_aMap.insert(pair<const ::rtl::OUString, InterfaceRef  >(sName,xNewElement));
    m_aItems[_nIndex] = xNewElement;

    xChild = staruno::Reference<starcontainer::XChild> (xNewElement, staruno::UNO_QUERY);
    if (xChild.is())
        xChild->setParent(static_cast<starcontainer::XContainer*>(this));

    m_xEventAttacher->insertEntry(_nIndex);
    xIfc = InterfaceRef (xNewElement, staruno::UNO_QUERY);// wichtig
    staruno::Any aHelper;
    aHelper <<= xSet;
    m_xEventAttacher->attach(_nIndex, xIfc, aHelper);

    // benachrichtigen
    starcontainer::ContainerEvent aEvt;
    aEvt.Source   = static_cast<starcontainer::XContainer*>(this);
    aEvt.Accessor <<= _nIndex;
    aEvt.Element  = staruno::Any(&xNewElement, m_aElementType);
    aEvt.ReplacedElement = staruno::Any(&xOldElement, m_aElementType);
    NOTIFY_LISTENERS(m_aContainerListeners, starcontainer::XContainerListener, elementReplaced, aEvt);
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeByIndex(sal_Int32 _nIndex) throw( starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    if (_nIndex < 0 || _nIndex >= m_aItems.size())
        throw starlang::IndexOutOfBoundsException();

    OInterfaceArray::iterator i = m_aItems.begin() + _nIndex;
    InterfaceRef  xElement(*i);

    OInterfaceMap::iterator j = m_aMap.begin();
    while (j != m_aMap.end() && (*j).second != xElement) ++j;

    m_aItems.erase(i);
    m_aMap.erase(j);

    // Eventverknüpfungen aufheben
    InterfaceRef  xIfc(xElement, staruno::UNO_QUERY);// wichtig
    m_xEventAttacher->detach(_nIndex, xIfc);
    m_xEventAttacher->removeEntry(_nIndex);

    staruno::Reference<starbeans::XPropertySet>  xSet(xElement, staruno::UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    staruno::Reference<starcontainer::XChild>  xChild(xElement, staruno::UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());

    // notify derived classes
    implRemoved(xElement);

    // notify listeners
    starcontainer::ContainerEvent aEvt;
    aEvt.Source     = static_cast<starcontainer::XContainer*>(this);
    aEvt.Element    = staruno::Any(&xElement, m_aElementType);
    aEvt.Accessor   <<= _nIndex;
    NOTIFY_LISTENERS(m_aContainerListeners, starcontainer::XContainerListener, elementRemoved, aEvt);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertByName(const ::rtl::OUString& Name, const staruno::Any& Element) throw( starlang::IllegalArgumentException, starcontainer::ElementExistException, starlang::WrappedTargetException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    if (Element.getValueType().getTypeClass() != staruno::TypeClass_INTERFACE)
        throw starlang::IllegalArgumentException();

    InterfaceRef  xElement(*(InterfaceRef *)Element.getValue());
    staruno::Reference<starbeans::XPropertySet>  xSet(xElement, staruno::UNO_QUERY);
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            throw starlang::IllegalArgumentException();

        xSet->setPropertyValue(PROPERTY_NAME, staruno::makeAny(Name));
    }

    insertByIndex(m_aItems.size(), Element);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::replaceByName(const ::rtl::OUString& Name, const staruno::Any& Element) throw( starlang::IllegalArgumentException, starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(Name);
    if (aPair.first == aPair.second)
        throw starcontainer::NoSuchElementException();

    if (Element.getValueType().getTypeClass() != staruno::TypeClass_INTERFACE)
        throw starlang::IllegalArgumentException();

    staruno::Reference<starbeans::XPropertySet>  xSet(*(InterfaceRef *)Element.getValue(), staruno::UNO_QUERY);
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            throw starlang::IllegalArgumentException();

        xSet->setPropertyValue(PROPERTY_NAME, staruno::makeAny(Name));
    }

    // determine the element pos
    sal_Int32 nPos = find(m_aItems.begin(), m_aItems.end(), (*aPair.first).second) - m_aItems.begin();
    replaceByIndex(nPos, Element);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeByName(const ::rtl::OUString& Name) throw( starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(Name);
    if (aPair.first == aPair.second)
        throw starcontainer::NoSuchElementException();

    sal_Int32 nPos = find(m_aItems.begin(), m_aItems.end(), (*aPair.first).second) - m_aItems.begin();
    removeByIndex(nPos);
}


// starscript::XEventAttacherManager
//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::registerScriptEvent( sal_Int32 nIndex, const starscript::ScriptEventDescriptor& aScriptEvent ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->registerScriptEvent(nIndex, aScriptEvent);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::registerScriptEvents( sal_Int32 nIndex, const staruno::Sequence< starscript::ScriptEventDescriptor >& aScriptEvents ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->registerScriptEvents(nIndex, aScriptEvents);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::revokeScriptEvent( sal_Int32 nIndex, const ::rtl::OUString& aListenerType, const ::rtl::OUString& aEventMethod, const ::rtl::OUString& aRemoveListenerParam ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->revokeScriptEvent(nIndex,
                        aListenerType, aEventMethod, aRemoveListenerParam );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::revokeScriptEvents( sal_Int32 nIndex ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->revokeScriptEvents(nIndex);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertEntry( sal_Int32 nIndex ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->insertEntry(nIndex);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeEntry( sal_Int32 nIndex ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->removeEntry(nIndex);
}

//------------------------------------------------------------------------
staruno::Sequence< starscript::ScriptEventDescriptor > SAL_CALL OInterfaceContainer::getScriptEvents( sal_Int32 nIndex ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    return m_xEventAttacher->getScriptEvents(nIndex);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::attach( sal_Int32 nIndex, const staruno::Reference< staruno::XInterface >& xObject, const staruno::Any& aHelper ) throw(starlang::IllegalArgumentException, starlang::ServiceNotRegisteredException, staruno::RuntimeException)
{
    m_xEventAttacher->attach(nIndex, xObject, aHelper);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::detach( sal_Int32 nIndex, const staruno::Reference< staruno::XInterface >& xObject ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->detach(nIndex, xObject);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::addScriptListener( const staruno::Reference< starscript::XScriptListener >& xListener ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->addScriptListener(xListener);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeScriptListener( const staruno::Reference< starscript::XScriptListener >& xListener ) throw(starlang::IllegalArgumentException, staruno::RuntimeException)
{
    m_xEventAttacher->removeScriptListener(xListener);
}

//==================================================================
//= OFormComponents
//==================================================================
//------------------------------------------------------------------------------
staruno::Any SAL_CALL OFormComponents::queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException)
{
    staruno::Any aReturn = ::cppu::queryInterface(
        _rType,
        static_cast<starform::XFormComponent*>(this),
        static_cast<starcontainer::XChild*>(static_cast<starform::XFormComponent*>(this))
    );

    if (!aReturn.hasValue())
        aReturn = OInterfaceContainer::queryInterface(_rType);

    if (!aReturn.hasValue())
        aReturn = FormComponentsBase::queryAggregation(_rType);

    return aReturn;
}

//------------------------------------------------------------------
staruno::Sequence<staruno::Type> SAL_CALL OFormComponents::getTypes() throw(staruno::RuntimeException)
{
    staruno::Sequence<staruno::Type> aBaseTypes = OInterfaceContainer::getTypes();
    staruno::Sequence<staruno::Type> aComponentTypes = FormComponentsBase::getTypes();

    staruno::Sequence<staruno::Type> aOwnTypes(1);
    aOwnTypes.getArray()[0] = ::getCppuType(static_cast<staruno::Reference<starform::XFormComponent>*>(NULL));

    return ::utl::concatSequences(aBaseTypes, aComponentTypes, aOwnTypes);
}

//------------------------------------------------------------------------------
OFormComponents::OFormComponents(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
                  :FormComponentsBase(m_aMutex)
                  ,OInterfaceContainer(_rxFactory, m_aMutex, ::getCppuType(static_cast<staruno::Reference<starform::XFormComponent>*>(NULL)))
{
}

//------------------------------------------------------------------------------
OFormComponents::~OFormComponents()
{
    if (!FormComponentsBase::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}

// OComponentHelper
//------------------------------------------------------------------------------
void OFormComponents::disposing()
{
    OInterfaceContainer::disposing();
    FormComponentsBase::disposing();
    m_xParent = NULL;
}

//XChild
//------------------------------------------------------------------------------
void OFormComponents::setParent(const InterfaceRef& Parent) throw( starlang::NoSupportException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_xParent = Parent;
}

//------------------------------------------------------------------------------
InterfaceRef OFormComponents::getParent() throw( staruno::RuntimeException )
{
    return m_xParent;
}

//.........................................................................
}   // namespace frm
//.........................................................................

