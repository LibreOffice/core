/*************************************************************************
 *
 *  $RCSfile: importmergehandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:15:01 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "importmergehandler.hxx"

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace beans = ::com::sun::star::beans;
// -----------------------------------------------------------------------------

ImportMergeHandler::ImportMergeHandler( Backend const & xTargetBackend, Mode mode, OUString const & aEntity )
: BasicImportHandler(xTargetBackend,aEntity)
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

inline ImportMergeHandler::OutputHandler ImportMergeHandler::getOutputHandler()
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
        aArgs[0] <<= beans::NamedValue( OUString::createFromAscii(property), uno::makeAny(value) );
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
ImportMergeHandler::OutputHandler ImportMergeHandler::createOutputHandler()
{
    using rtl::OUStringBuffer;

    OSL_PRECOND( hasComponent(), "Trying to create output-handler for Import Merger without setting a component first") ;
    OUString const aComponentName = this->getComponent();

    OutputHandler xOutputHandler;
    try
    {
        xOutputHandler =    hasEntity() ? getBackend()->getUpdateHandler(aComponentName,getEntity())
                                        : getBackend()->getOwnUpdateHandler(aComponentName);
    }
    catch (lang::NoSupportException & e)
    {
        OUStringBuffer sMessage;
        sMessage.appendAscii("configmgr::backend::ImportHandler: ");
        sMessage.appendAscii("Could not get output handler for component ").append(aComponentName);
        sMessage.appendAscii(": Backend does not support updates - ").append( e.Message );

        throw lang::WrappedTargetException(sMessage.makeStringAndClear(), *this, uno::makeAny(e));
    }
    catch (lang::IllegalArgumentException & e)
    {
        OUStringBuffer sMessage;
        sMessage.appendAscii("configmgr::backend::ImportHandler: ");
        sMessage.appendAscii("Could not get output handler for component ").append(aComponentName);
        sMessage.appendAscii(" due to a backend exception: ").append( e.Message );

        throw lang::WrappedTargetException(sMessage.makeStringAndClear(), *this, uno::makeAny(e));
    }

    if (!xOutputHandler.is())
    {
        OUStringBuffer sMessage;
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
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_xOutputHandler.clear();

    BasicImportHandler::startLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::endLayer(  )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (isStarted())
        getOutputHandler()->endUpdate();

    BasicImportHandler::endLayer();

    m_xOutputHandler.clear();
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isStarted() && startComponent(aName))
    {
        (m_xOutputHandler = createOutputHandler())->startUpdate(  );
    }

    OSL_ENSURE(!bClear,"'clear' operation not supported properly on import");

    bool bReset = (m_mode != merge) || bClear; // is not relevant for no_overwrite,but might be cheaper there
    getOutputHandler()->modifyNode(aName,aAttributes,aAttributes,bReset);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->addOrReplaceNode(aName,aAttributes);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->addOrReplaceNodeFromTemplate(aName,aAttributes,aTemplate);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::endNode(  )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->endNode();
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::dropNode( const OUString& aName )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->removeNode(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_ENSURE(!bClear,"'clear' operation not supported on import");
    getOutputHandler()->modifyProperty(aName,aAttributes,aAttributes,aType);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::endProperty(  )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->endProperty();
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::setPropertyValue( const uno::Any& aValue )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->setPropertyValue(aValue);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::setPropertyValueForLocale( const uno::Any& aValue, const OUString & aLocale )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->setPropertyValueForLocale(aValue,aLocale);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->addOrReplaceProperty(aName, aAttributes, aType);
}
// -----------------------------------------------------------------------------

void SAL_CALL ImportMergeHandler::addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    getOutputHandler()->addOrReplacePropertyWithValue(aName, aAttributes, aValue);
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

