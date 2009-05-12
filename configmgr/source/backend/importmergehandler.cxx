/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: importmergehandler.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "importmergehandler.hxx"
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <rtl/ustrbuf.hxx>


// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace beans = ::com::sun::star::beans;

// -----------------------------------------------------------------------------

ImportMergeHandler::ImportMergeHandler(
    uno::Reference< backenduno::XBackend > const & xTargetBackend, Mode mode, rtl::OUString const & aEntity, sal_Bool const & bNotify )
: BasicImportHandler(xTargetBackend,aEntity, bNotify)
, m_xOutputHandler()
, m_mode(mode)
{
}
// -----------------------------------------------------------------------------

void ImportMergeHandler::failNotStarted()
{
    OSL_ENSURE(!hasComponent(), "Import handler after failure to create output handler or after closing");
    raiseMalformedDataException("configmgr::backend::ImportHandler: Trying to generate output before identifying the target component");
}
// -----------------------------------------------------------------------------

inline bool ImportMergeHandler::isStarted()    const
{
    return !! m_xOutputHandler.is();
}
// -----------------------------------------------------------------------------

inline void ImportMergeHandler::checkStarted()
{
    if (!isStarted()) failNotStarted();
}
// -----------------------------------------------------------------------------

inline uno::Reference< backenduno::XUpdateHandler > ImportMergeHandler::getOutputHandler()
{
    checkStarted();
    return m_xOutputHandler;
}
// -----------------------------------------------------------------------------

static
bool setHandlerProperty(uno::Reference< uno::XInterface > const & xHandler, char const * property, sal_Bool value)
{
    OSL_ASSERT(property);
    uno::Reference< lang::XInitialization > xInitHandler( xHandler, uno::UNO_QUERY );
    if (xHandler.is())
    try
    {
        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<= beans::NamedValue( rtl::OUString::createFromAscii(property), uno::makeAny(value) );
        xInitHandler->initialize(aArgs);
        return true;
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE("Configuration Import Handler - Could not set output handler property '%s': %s\n",
                    property,rtl::OUStringToOString(e.Message,RTL_TEXTENCODING_ASCII_US).getStr());

        OSL_ENSURE(false, "Output Handler does not support expected property" );
    }
    else
    {
        OSL_TRACE("Configuration Import Handler - Could not set output handler property '%s': %s\n",
                    property,"Object does not support expected interface");

        OSL_ENSURE(false, "Output Handler does not support expected interface" );
    }
    return false;
}
// -----------------------------------------------------------------------------
uno::Reference< backenduno::XUpdateHandler > ImportMergeHandler::createOutputHandler()
{
    OSL_PRECOND( hasComponent(), "Trying to create output-handler for Import Merger without setting a component first") ;
    rtl::OUString const aComponentName = this->getComponent();

    uno::Reference< backenduno::XUpdateHandler > xOutputHandler;
    try
    {
        xOutputHandler =    hasEntity() ? getBackend()->getUpdateHandler(aComponentName,getEntity())
                                        : getBackend()->getOwnUpdateHandler(aComponentName);
    }
    catch (lang::NoSupportException & e)
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("configmgr::backend::ImportHandler: ");
        sMessage.appendAscii("Could not get output handler for component ").append(aComponentName);
        sMessage.appendAscii(": Backend does not support updates - ").append( e.Message );

        throw lang::WrappedTargetException(sMessage.makeStringAndClear(), *this, uno::makeAny(e));
    }
    catch (lang::IllegalArgumentException & e)
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("configmgr::backend::ImportHandler: ");
        sMessage.appendAscii("Could not get output handler for component ").append(aComponentName);
        sMessage.appendAscii(" due to a backend exception: ").append( e.Message );

        throw lang::WrappedTargetException(sMessage.makeStringAndClear(), *this, uno::makeAny(e));
    }

    if (!xOutputHandler.is())
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("configmgr::backend::ImportHandler: ");
        sMessage.appendAscii("Cannot import. ERROR - The backend returns a NULL handler for component ")
                .append(aComponentName).append( sal_Unicode('.') );

        throw uno::RuntimeException(sMessage.makeStringAndClear(), *this);
    }

    switch (m_mode)
    {
    case merge: break;
    case copy:          setHandlerProperty(xOutputHandler,"Truncate", sal_True);  break;
    case no_overwrite:  setHandlerProperty(xOutputHandler,"Overwrite",sal_False); break;

    default: OSL_ASSERT(false); break;
    }

    return xOutputHandler;
}
// -----------------------------------------------------------------------------

// XLayerHandler

void SAL_CALL ImportMergeHandler::startLayer(  )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_xOutputHandler.clear();

    BasicImportHandler::startLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::endLayer(  )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (isStarted())
    try
    {
        getOutputHandler()->endUpdate();
    }
    catch (lang::IllegalAccessException & iae)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM("ImportHandler - no write access to layer: "));
        throw lang::WrappedTargetException(sMsg.concat(iae.Message),*this,uno::makeAny(iae));
    }

    BasicImportHandler::endLayer();
    m_xOutputHandler.clear();
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::overrideNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isStarted() && startComponent(aName))
    try
    {
        (m_xOutputHandler = createOutputHandler())->startUpdate(  );
    }
    catch (lang::IllegalAccessException & iae)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM("ImportHandler - no write access to layer: "));
        throw lang::WrappedTargetException(sMsg.concat(iae.Message),*this,uno::makeAny(iae));
    }

    OSL_ENSURE(!bClear,"'clear' operation not supported properly on import");

    bool bReset = (m_mode != merge) || bClear; // is not relevant for no_overwrite,but might be cheaper there
    getOutputHandler()->modifyNode(aName,aAttributes,aAttributes,bReset);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->addOrReplaceNode(aName,aAttributes);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::addOrReplaceNodeFromTemplate( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->addOrReplaceNodeFromTemplate(aName,aAttributes,aTemplate);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::endNode(  )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->endNode();
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::dropNode( const rtl::OUString& aName )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->removeNode(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::overrideProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    (void) bClear; // avoid warning about unused parameter
    OSL_ENSURE(!bClear,"'clear' operation not supported on import");
    getOutputHandler()->modifyProperty(aName,aAttributes,aAttributes,aType);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::endProperty(  )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->endProperty();
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::setPropertyValue( const uno::Any& aValue )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->setPropertyValue(aValue);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString & aLocale )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->setPropertyValueForLocale(aValue,aLocale);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->addOrReplaceProperty(aName, aAttributes, aType);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::addPropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->addOrReplacePropertyWithValue(aName, aAttributes, aValue);
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

