/*************************************************************************
 *
 *  $RCSfile: InterfaceContainer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-23 09:42:45 $
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
#ifndef _COMPHELPER_EVENTATTACHERMGR_HXX_
#include <comphelper/eventattachermgr.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
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
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::form;
//------------------------------------------------------------------
OInterfaceContainer::OInterfaceContainer(
                const Reference<XMultiServiceFactory>& _rxFactory,
                ::osl::Mutex& _rMutex,
                const Type& _rElementType)
        :m_rMutex(_rMutex)
        ,m_aContainerListeners(_rMutex)
        ,m_aElementType(_rElementType)
        ,m_xServiceFactory(_rxFactory)
{
    m_xEventAttacher = ::comphelper::createEventAttacherManager(m_xServiceFactory);
}

//------------------------------------------------------------------------------
void OInterfaceContainer::disposing()
{
    // dispose aller elemente
    for (sal_Int32 i = m_aItems.size(); i > 0; --i)
    {
        Reference<XPropertySet>  xSet(m_aItems[i - 1], UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener(PROPERTY_NAME, this);

        // Eventverknüpfungen aufheben
        InterfaceRef  xIfc(xSet, UNO_QUERY);
        m_xEventAttacher->detach(i - 1, xIfc);
        m_xEventAttacher->removeEntry(i - 1);

        Reference<XComponent>  xComponent(xSet, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }
    m_aMap.clear();
    m_aItems.clear();

    EventObject aEvt(static_cast<XContainer*>(this));
    m_aContainerListeners.disposeAndClear(aEvt);
}

// XPersistObject
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::writeEvents(const Reference<XObjectOutputStream>& _rxOutStream)
{
    Reference<XMarkableStream>  xMark(_rxOutStream, UNO_QUERY);
    sal_Int32 nMark = xMark->createMark();

    sal_Int32 nObjLen = 0;
    _rxOutStream->writeLong(nObjLen);

    Reference<XPersistObject>  xScripts(m_xEventAttacher, UNO_QUERY);
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
void SAL_CALL OInterfaceContainer::readEvents(const Reference<XObjectInputStream>& _rxInStream, sal_Int32 nCount)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    if (nCount)
    {
        // Scripting Info lesen
        Reference<XMarkableStream>  xMark(_rxInStream, UNO_QUERY);
        sal_Int32 nObjLen = _rxInStream->readLong();
        if (nObjLen)
        {
            sal_Int32 nMark = xMark->createMark();
            Reference<XPersistObject>  xObj(m_xEventAttacher, UNO_QUERY);
            if (xObj.is())
                xObj->read(_rxInStream);
            xMark->jumpToMark(nMark);
            _rxInStream->skipBytes(nObjLen);
            xMark->deleteMark(nMark);
        }

        // Attachement lesen
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            InterfaceRef  xIfc(m_aItems[i], UNO_QUERY);
            Reference<XPropertySet>  xSet(xIfc, UNO_QUERY);
            Any aHelper;
            aHelper <<= xSet;
            m_xEventAttacher->attach( i, xIfc, aHelper );
        }
    }
    else
    {
        // neuen EventManager
        m_xEventAttacher = ::comphelper::createEventAttacherManager(m_xServiceFactory);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::write( const Reference< XObjectOutputStream >& _rxOutStream ) throw(IOException, RuntimeException)
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
            Reference<XPersistObject>  xObj(m_aItems[i], UNO_QUERY);
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
void SAL_CALL OInterfaceContainer::read( const Reference< XObjectInputStream >& _rxInStream ) throw(IOException, RuntimeException)
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
            Reference<XPersistObject>  xObj;
            try
            {
                xObj = _rxInStream->readObject();
            }
            catch(WrongFormatException& e)
            {
                e;  // make compiler happy
                // the object could not be read
                xObj = NULL;
                // create a dummy starform (so the readEvents below will assign the events to the right controls)
                xObj = Reference<XPersistObject> (m_xServiceFactory->createInstance(FRM_COMPONENT_HIDDENCONTROL), UNO_QUERY);
                DBG_ASSERT(xObj.is(), "OInterfaceContainer::read : could not create a substitute for the unknown object !");
                if (!xObj.is())
                    // couldn't handle it ...
                    throw;

                // set some properties describing what we did
                Reference<XPropertySet>  xObjProps(xObj, UNO_QUERY);
                if (xObjProps.is())
                {
                    try
                    {
                        xObjProps->setPropertyValue(PROPERTY_NAME, makeAny(FRM_RES_STRING(RID_STR_CONTROL_SUBSTITUTED_NAME)));
                        xObjProps->setPropertyValue(PROPERTY_TAG, makeAny(FRM_RES_STRING(RID_STR_CONTROL_SUBSTITUTED_EPXPLAIN)));
                    }
                    catch(Exception&)
                    {
                    }
                }
                // 72133 - 09.02.00 - FS
            }
            catch(Exception&)
            {
                // unsere Map leeren
                while (!m_aItems.empty())
                    removeElementsNoEvents(0);

                // und die Exception nach aussen
                throw;
            }

            if (xObj.is())
            {
                Any aElement = xObj->queryInterface(m_aElementType);
                if (aElement.hasValue())
                    insert(m_aItems.size(), *static_cast<const InterfaceRef*>(aElement.getValue()), sal_False);
                else
                    ; // form konnte nicht gelesen werden; nyi
            }
        }
    }

    readEvents(_rxInStream, nLen);
}

// XContainer
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::addContainerListener(const Reference<XContainerListener>& _rxListener) throw( RuntimeException )
{
    m_aContainerListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeContainerListener(const Reference<XContainerListener>& _rxListener) throw( RuntimeException )
{
    m_aContainerListeners.removeInterface(_rxListener);
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::disposing(const EventObject& _rSource) throw( RuntimeException )
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

// XPropertyChangeListener
//------------------------------------------------------------------------------
void OInterfaceContainer::propertyChange(const PropertyChangeEvent& evt)
{
    if (evt.PropertyName == PROPERTY_NAME)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        OInterfaceMap::iterator i = find(m_aMap.begin(), m_aMap.end(),
            pair<const ::rtl::OUString, InterfaceRef >(::comphelper::getString(evt.OldValue),evt.Source));
        if (i != m_aMap.end())
        {
            InterfaceRef  xCorrectType((*i).second);
            m_aMap.erase(i);
            m_aMap.insert(pair<const ::rtl::OUString, InterfaceRef >(::comphelper::getString(evt.NewValue),xCorrectType));
        }
    }
}

// XElementAccess
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OInterfaceContainer::hasElements() throw( RuntimeException )
{
    return !m_aMap.empty();
}

//------------------------------------------------------------------------------
Type SAL_CALL OInterfaceContainer::getElementType() throw(RuntimeException)
{
    return m_aElementType;
}

// XEnumerationAccess
//------------------------------------------------------------------------------
Reference<XEnumeration> SAL_CALL OInterfaceContainer::createEnumeration() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    return new ::comphelper::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
}

// XNameAccess
//------------------------------------------------------------------------------
Any SAL_CALL OInterfaceContainer::getByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(_rName);

    if (aPair.first == aPair.second)
        throw NoSuchElementException();

    return Any(&(*aPair.first).second, m_aElementType);
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OInterfaceContainer::getElementNames() throw(RuntimeException)
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
sal_Bool SAL_CALL OInterfaceContainer::hasByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
{
    pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(_rName);
    return aPair.first != aPair.second;
}

// XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 OInterfaceContainer::getCount() throw( RuntimeException )
{
    return m_aItems.size();
}

//------------------------------------------------------------------------------
Any OInterfaceContainer::getByIndex(sal_Int32 _nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if (_nIndex < 0 || _nIndex >= m_aItems.size())
        throw IndexOutOfBoundsException();

    return Any(&m_aItems[_nIndex], m_aElementType);
}

//------------------------------------------------------------------------------
void OInterfaceContainer::insert(sal_Int32 _nIndex, const InterfaceRef& xElement, sal_Bool bEvents) throw( IllegalArgumentException )
{
    // das richtige Interface besorgen
    Any aCorrectType = xElement->queryInterface(m_aElementType);
    if (!aCorrectType.hasValue())
        throw IllegalArgumentException();
    InterfaceRef xCorrectType = *static_cast<const InterfaceRef*>(aCorrectType.getValue());

    ::rtl::OUString sName;
    Reference<XPropertySet>  xSet(xElement, UNO_QUERY);
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            throw IllegalArgumentException();

        Any aValue = xSet->getPropertyValue(PROPERTY_NAME);
        aValue >>= sName;
        xSet->addPropertyChangeListener(PROPERTY_NAME, this);
    }
    else
        throw IllegalArgumentException();

    if (_nIndex > m_aItems.size()) // ermitteln des tatsaechlichen Indexs
    {
        _nIndex = m_aItems.size();
        m_aItems.push_back(xCorrectType);
    }
    else
        m_aItems.insert(m_aItems.begin() + _nIndex, xCorrectType);

    m_aMap.insert(pair<const ::rtl::OUString, InterfaceRef  >(sName,xCorrectType));

    Reference<XChild>  xChild(xElement, UNO_QUERY);
    if (xChild.is())
        xChild->setParent(static_cast<XContainer*>(this));

    if (bEvents)
    {
        m_xEventAttacher->insertEntry(_nIndex);
        InterfaceRef  xIfc(xElement, UNO_QUERY);// wichtig
        Any aHelper;
        aHelper <<= xSet;
        m_xEventAttacher->attach(_nIndex, xIfc, aHelper);
    }

    // notify derived classes
    implInserted(xCorrectType);

    // notify listeners
    ContainerEvent aEvt;
    aEvt.Source   = static_cast<XContainer*>(this);
    aEvt.Accessor <<= _nIndex;
    aEvt.Element  = Any(&xCorrectType, m_aElementType);
    NOTIFY_LISTENERS(m_aContainerListeners, XContainerListener, elementInserted, aEvt);
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

    Reference<XPropertySet>  xSet(xElement, UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    Reference<XChild>  xChild(xElement, UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());
}

// XIndexContainer
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertByIndex(sal_Int32 _nIndex, const Any& Element) throw(IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if (Element.getValueType().getTypeClass() != TypeClass_INTERFACE)
        throw IllegalArgumentException();

    ::osl::MutexGuard aGuard( m_rMutex );
    insert(_nIndex, InterfaceRef (*(InterfaceRef *)Element.getValue()), sal_True);
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::replaceByIndex(sal_Int32 _nIndex, const Any& Element) throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if (Element.getValueType().getTypeClass() != TypeClass_INTERFACE)
        throw IllegalArgumentException();

    ::osl::MutexGuard aGuard( m_rMutex );
    if (_nIndex < 0 || _nIndex >= m_aItems.size())
        throw IndexOutOfBoundsException();


    InterfaceRef  xOldElement(m_aItems[_nIndex]);
    InterfaceRef  xNewElement(*(InterfaceRef *)Element.getValue());

    OInterfaceMap::iterator j = m_aMap.begin();
    while (j != m_aMap.end() && (*j).second != xOldElement) ++j;

    // Eventverknüpfungen aufheben
    InterfaceRef  xIfc(xOldElement, UNO_QUERY);// wichtig
    m_xEventAttacher->detach(_nIndex, xIfc);
    m_xEventAttacher->removeEntry(_nIndex);

    Reference<XPropertySet>  xSet(xOldElement, UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    Reference<XChild>  xChild(xOldElement, UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());

    // neue einfuegen
    ::rtl::OUString sName;
    xSet = Reference<XPropertySet> (xNewElement, UNO_QUERY);
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            throw IllegalArgumentException();

        Any aValue = xSet->getPropertyValue(PROPERTY_NAME);
        aValue >>= sName;
        xSet->addPropertyChangeListener(PROPERTY_NAME, this);
    }
    else
        throw IllegalArgumentException();

    // remove the old one
    m_aMap.erase(j);

    // insert the new one
    m_aMap.insert(pair<const ::rtl::OUString, InterfaceRef  >(sName,xNewElement));
    m_aItems[_nIndex] = xNewElement;

    xChild = Reference<XChild> (xNewElement, UNO_QUERY);
    if (xChild.is())
        xChild->setParent(static_cast<XContainer*>(this));

    m_xEventAttacher->insertEntry(_nIndex);
    xIfc = InterfaceRef (xNewElement, UNO_QUERY);// wichtig
    Any aHelper;
    aHelper <<= xSet;
    m_xEventAttacher->attach(_nIndex, xIfc, aHelper);

    // benachrichtigen
    ContainerEvent aEvt;
    aEvt.Source   = static_cast<XContainer*>(this);
    aEvt.Accessor <<= _nIndex;
    aEvt.Element  = Any(&xNewElement, m_aElementType);
    aEvt.ReplacedElement = Any(&xOldElement, m_aElementType);
    NOTIFY_LISTENERS(m_aContainerListeners, XContainerListener, elementReplaced, aEvt);
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeByIndex(sal_Int32 _nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    if (_nIndex < 0 || _nIndex >= m_aItems.size())
        throw IndexOutOfBoundsException();

    OInterfaceArray::iterator i = m_aItems.begin() + _nIndex;
    InterfaceRef  xElement(*i);

    OInterfaceMap::iterator j = m_aMap.begin();
    while (j != m_aMap.end() && (*j).second != xElement) ++j;

    m_aItems.erase(i);
    m_aMap.erase(j);

    // Eventverknüpfungen aufheben
    InterfaceRef  xIfc(xElement, UNO_QUERY);// wichtig
    m_xEventAttacher->detach(_nIndex, xIfc);
    m_xEventAttacher->removeEntry(_nIndex);

    Reference<XPropertySet>  xSet(xElement, UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    Reference<XChild>  xChild(xElement, UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());

    // notify derived classes
    implRemoved(xElement);

    // notify listeners
    ContainerEvent aEvt;
    aEvt.Source     = static_cast<XContainer*>(this);
    aEvt.Element    = Any(&xElement, m_aElementType);
    aEvt.Accessor   <<= _nIndex;
    NOTIFY_LISTENERS(m_aContainerListeners, XContainerListener, elementRemoved, aEvt);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertByName(const ::rtl::OUString& Name, const Any& Element) throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    if (Element.getValueType().getTypeClass() != TypeClass_INTERFACE)
        throw IllegalArgumentException();

    InterfaceRef  xElement(*(InterfaceRef *)Element.getValue());
    Reference<XPropertySet>  xSet(xElement, UNO_QUERY);
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            throw IllegalArgumentException();

        xSet->setPropertyValue(PROPERTY_NAME, makeAny(Name));
    }

    insertByIndex(m_aItems.size(), Element);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::replaceByName(const ::rtl::OUString& Name, const Any& Element) throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(Name);
    if (aPair.first == aPair.second)
        throw NoSuchElementException();

    if (Element.getValueType().getTypeClass() != TypeClass_INTERFACE)
        throw IllegalArgumentException();

    Reference<XPropertySet>  xSet(*(InterfaceRef *)Element.getValue(), UNO_QUERY);
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            throw IllegalArgumentException();

        xSet->setPropertyValue(PROPERTY_NAME, makeAny(Name));
    }

    // determine the element pos
    sal_Int32 nPos = find(m_aItems.begin(), m_aItems.end(), (*aPair.first).second) - m_aItems.begin();
    replaceByIndex(nPos, Element);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeByName(const ::rtl::OUString& Name) throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(Name);
    if (aPair.first == aPair.second)
        throw NoSuchElementException();

    sal_Int32 nPos = find(m_aItems.begin(), m_aItems.end(), (*aPair.first).second) - m_aItems.begin();
    removeByIndex(nPos);
}


// XEventAttacherManager
//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::registerScriptEvent( sal_Int32 nIndex, const ScriptEventDescriptor& aScriptEvent ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->registerScriptEvent(nIndex, aScriptEvent);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::registerScriptEvents( sal_Int32 nIndex, const Sequence< ScriptEventDescriptor >& aScriptEvents ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->registerScriptEvents(nIndex, aScriptEvents);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::revokeScriptEvent( sal_Int32 nIndex, const ::rtl::OUString& aListenerType, const ::rtl::OUString& aEventMethod, const ::rtl::OUString& aRemoveListenerParam ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->revokeScriptEvent(nIndex,
                        aListenerType, aEventMethod, aRemoveListenerParam );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::revokeScriptEvents( sal_Int32 nIndex ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->revokeScriptEvents(nIndex);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertEntry( sal_Int32 nIndex ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->insertEntry(nIndex);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeEntry( sal_Int32 nIndex ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->removeEntry(nIndex);
}

//------------------------------------------------------------------------
Sequence< ScriptEventDescriptor > SAL_CALL OInterfaceContainer::getScriptEvents( sal_Int32 nIndex ) throw(IllegalArgumentException, RuntimeException)
{
    return m_xEventAttacher->getScriptEvents(nIndex);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::attach( sal_Int32 nIndex, const Reference< XInterface >& xObject, const Any& aHelper ) throw(IllegalArgumentException, ServiceNotRegisteredException, RuntimeException)
{
    m_xEventAttacher->attach(nIndex, xObject, aHelper);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::detach( sal_Int32 nIndex, const Reference< XInterface >& xObject ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->detach(nIndex, xObject);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::addScriptListener( const Reference< XScriptListener >& xListener ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->addScriptListener(xListener);
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeScriptListener( const Reference< XScriptListener >& xListener ) throw(IllegalArgumentException, RuntimeException)
{
    m_xEventAttacher->removeScriptListener(xListener);
}

//==================================================================
//= OFormComponents
//==================================================================
//------------------------------------------------------------------------------
Any SAL_CALL OFormComponents::queryAggregation(const Type& _rType) throw(RuntimeException)
{
    Any aReturn = OFormComponents_BASE::queryInterface(_rType);
    if (!aReturn.hasValue())
    {
        aReturn = OInterfaceContainer::queryInterface(_rType);

        if (!aReturn.hasValue())
            aReturn = FormComponentsBase::queryAggregation(_rType);
    }

    return aReturn;
}

//------------------------------------------------------------------
Sequence<Type> SAL_CALL OFormComponents::getTypes() throw(RuntimeException)
{
    return ::comphelper::concatSequences(OInterfaceContainer::getTypes(), FormComponentsBase::getTypes(), OFormComponents_BASE::getTypes());
}

//------------------------------------------------------------------------------
OFormComponents::OFormComponents(const Reference<XMultiServiceFactory>& _rxFactory)
                  :FormComponentsBase(m_aMutex)
                  ,OInterfaceContainer(_rxFactory, m_aMutex, ::getCppuType(static_cast<Reference<XFormComponent>*>(NULL)))
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
void OFormComponents::setParent(const InterfaceRef& Parent) throw( NoSupportException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_xParent = Parent;
}

//------------------------------------------------------------------------------
InterfaceRef OFormComponents::getParent() throw( RuntimeException )
{
    return m_xParent;
}

//.........................................................................
}   // namespace frm
//.........................................................................

