/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configurationhelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-30 10:26:21 $
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

//_______________________________________________
// includes

#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

//_______________________________________________
// namespace

namespace comphelper{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > ConfigurationHelper::openConfig(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR   ,
                                                                            const ::rtl::OUString&                                       sPackage,
                                                                                  sal_Int32                                              eMode   )
{
    css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
        xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider")), css::uno::UNO_QUERY_THROW);

    ::comphelper::SequenceAsVector< css::uno::Any > lParams;
    css::beans::PropertyValue                       aParam ;

    // set root path
    aParam.Name    = ::rtl::OUString::createFromAscii("nodepath");
    aParam.Value <<= sPackage;
    lParams.push_back(css::uno::makeAny(aParam));

    // enable all locales mode
    if ((eMode & ConfigurationHelper::E_ALL_LOCALES)==ConfigurationHelper::E_ALL_LOCALES)
    {
        aParam.Name    = ::rtl::OUString::createFromAscii("locale");
        aParam.Value <<= ::rtl::OUString::createFromAscii("*");
        lParams.push_back(css::uno::makeAny(aParam));
    }

    // enable lazy writing
    sal_Bool bLazy = ((eMode & ConfigurationHelper::E_LAZY_WRITE)==ConfigurationHelper::E_LAZY_WRITE);
    aParam.Name    = ::rtl::OUString::createFromAscii("lazywrite");
    aParam.Value   = css::uno::makeAny(bLazy);
    lParams.push_back(css::uno::makeAny(aParam));

    // open it
    css::uno::Reference< css::uno::XInterface > xCFG;

    sal_Bool bReadOnly = ((eMode & ConfigurationHelper::E_READONLY)==ConfigurationHelper::E_READONLY);
    if (bReadOnly)
        xCFG = xConfigProvider->createInstanceWithArguments(
                ::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess"),
                lParams.getAsConstList());
    else
        xCFG = xConfigProvider->createInstanceWithArguments(
                ::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess"),
                lParams.getAsConstList());

    return xCFG;
}

//-----------------------------------------------
css::uno::Any ConfigurationHelper::readRelativeKey(const css::uno::Reference< css::uno::XInterface > xCFG    ,
                                                   const ::rtl::OUString&                            sRelPath,
                                                   const ::rtl::OUString&                            sKey    )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::beans::XPropertySet > xProps;
    xAccess->getByHierarchicalName(sRelPath) >>= xProps;
    if (!xProps.is())
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("The requested path \"");
        sMsg.append     (sRelPath               );
        sMsg.appendAscii("\" does not exists."  );

        throw css::container::NoSuchElementException(
                    sMsg.makeStringAndClear(),
                    css::uno::Reference< css::uno::XInterface >());
    }
    return xProps->getPropertyValue(sKey);
}

//-----------------------------------------------
void ConfigurationHelper::writeRelativeKey(const css::uno::Reference< css::uno::XInterface > xCFG    ,
                                           const ::rtl::OUString&                            sRelPath,
                                           const ::rtl::OUString&                            sKey    ,
                                           const css::uno::Any&                              aValue  )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::beans::XPropertySet > xProps;
    xAccess->getByHierarchicalName(sRelPath) >>= xProps;
    if (!xProps.is())
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("The requested path \"");
        sMsg.append     (sRelPath               );
        sMsg.appendAscii("\" does not exists."  );

        throw css::container::NoSuchElementException(
                    sMsg.makeStringAndClear(),
                    css::uno::Reference< css::uno::XInterface >());
    }
    xProps->setPropertyValue(sKey, aValue);
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > ConfigurationHelper::makeSureSetNodeExists(const css::uno::Reference< css::uno::XInterface > xCFG         ,
                                                                                       const ::rtl::OUString&                            sRelPathToSet,
                                                                                       const ::rtl::OUString&                            sSetNode     )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameAccess > xSet;
    xAccess->getByHierarchicalName(sRelPathToSet) >>= xSet;
    if (!xSet.is())
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("The requested path \"");
        sMsg.append     (sRelPathToSet          );
        sMsg.appendAscii("\" does not exists."  );

        throw css::container::NoSuchElementException(
                    sMsg.makeStringAndClear(),
                    css::uno::Reference< css::uno::XInterface >());
    }

    css::uno::Reference< css::uno::XInterface > xNode;
    if (xSet->hasByName(sSetNode))
        xSet->getByName(sSetNode) >>= xNode;
    else
    {
        css::uno::Reference< css::lang::XSingleServiceFactory > xNodeFactory(xSet, css::uno::UNO_QUERY_THROW);
        xNode = xNodeFactory->createInstance();
        css::uno::Reference< css::container::XNameContainer > xSetReplace(xSet, css::uno::UNO_QUERY_THROW);
        xSetReplace->insertByName(sSetNode, css::uno::makeAny(xNode));
    }

    return xNode;
}

//-----------------------------------------------
css::uno::Any ConfigurationHelper::readDirectKey(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR   ,
                                                 const ::rtl::OUString&                                       sPackage,
                                                 const ::rtl::OUString&                                       sRelPath,
                                                 const ::rtl::OUString&                                       sKey    ,
                                                       sal_Int32                                              eMode   )
{
    css::uno::Reference< css::uno::XInterface > xCFG = ConfigurationHelper::openConfig(xSMGR, sPackage, eMode);
    return ConfigurationHelper::readRelativeKey(xCFG, sRelPath, sKey);
}

//-----------------------------------------------
void ConfigurationHelper::writeDirectKey(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR   ,
                                         const ::rtl::OUString&                                       sPackage,
                                         const ::rtl::OUString&                                       sRelPath,
                                         const ::rtl::OUString&                                       sKey    ,
                                         const css::uno::Any&                                         aValue  ,
                                               sal_Int32                                              eMode   )
{
    css::uno::Reference< css::uno::XInterface > xCFG = ConfigurationHelper::openConfig(xSMGR, sPackage, eMode);
    ConfigurationHelper::writeRelativeKey(xCFG, sRelPath, sKey, aValue);
    ConfigurationHelper::flush(xCFG);
}

//-----------------------------------------------
void ConfigurationHelper::flush(const css::uno::Reference< css::uno::XInterface >& xCFG)
{
    css::uno::Reference< css::util::XChangesBatch > xBatch(xCFG, css::uno::UNO_QUERY_THROW);
    xBatch->commitChanges();
}

} // namespace comphelper
