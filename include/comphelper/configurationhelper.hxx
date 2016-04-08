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

#ifndef INCLUDED_COMPHELPER_CONFIGURATIONHELPER_HXX
#define INCLUDED_COMPHELPER_CONFIGURATIONHELPER_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/comphelperdllapi.h>
#include <o3tl/typed_flags_set.hxx>


namespace comphelper
{
    /** specify all possible modes, which can be used to open a configuration access.
     *
     *  @see    openConfig()
     *  @see    readDirectKey()
     *  @see    writeDirectKey()
     */
    enum class EConfigurationModes
    {
        /// opens configuration in read/write mode (without LAZY writing!)
        Standard = 0,
        /// configuration will be opened readonly
        ReadOnly = 1,
        /// all localized nodes will be interpreted as XInterface instead of interpreting it as atomic value nodes
        AllLocales = 2,
        /// enable lazy writing
        LazyWrite = 4
    };


}

namespace o3tl
{
    template<> struct typed_flags<comphelper::EConfigurationModes> : is_typed_flags<comphelper::EConfigurationModes, 0x7> {};
}

namespace comphelper
{

class COMPHELPER_DLLPUBLIC ConfigurationHelper
{
public:
    /** returns access to the specified configuration package.
     *
     *  This method should be used, if e.g. more than one request to the same
     *  configuration package is needed. The configuration access can be cached
     *  outside and used inbetween.
     *
     *  @param  rxContext
     *          the uno service manager, which should be used to create the
     *          configuration access.
     *
     *  @param  sPackage
     *          the name of the configuration package.
     *          e.g. <ul>
     *                  <li>org.openoffice.Office.Common</li>
     *                  <li>org.openoffice.Office.Common/Menu</li>
     *               </ul>
     *
     *  @param  eMode
     *          specify the open mode for the returned configuration access.
     *          It's interpreted as a flag field and can be any useful combination
     *          of values of EConfigurationModes.
     *
     *  @throw  Any exceptions the underlying configuration can throw.
     *          E.g. css::uno::Exception if the configuration could not be opened.
     */
    static css::uno::Reference< css::uno::XInterface > openConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                                  const OUString&                                           sPackage,
                                                                        EConfigurationModes                                 eMode   );


    /** reads the value of an existing(!) configuration key,
     *  which is searched relative to the specified configuration access.
     *
     *  This method must be used in combination with openConfig().
     *  The cached configuration access must be provided here ... and
     *  all operations are made relativ to this access point.
     *
     *  @param  xCFG
     *          the configuration root, where sRelPath should be interpreted.
     *          as relativ path
     *
     *  @param  sRelPath
     *          path relative to xCFG parameter.
     *
     *  @param  sKey
     *          the configuration node, where we should read the value.
     *
     *  @return [css.uno.Any]
     *          the value of sKey.
     *
     *  @throw  Any exceptions the underlying configuration can throw.
     *          E.g. css::container::NoSuchElementException if the specified
     *          key does not exists.
     */
    static css::uno::Any readRelativeKey(const css::uno::Reference< css::uno::XInterface >& xCFG    ,
                                         const OUString&                            sRelPath,
                                         const OUString&                            sKey    );


    /** writes a new value for an existing(!) configuration key,
     *  which is searched relative to the specified configuration access.
     *
     *  This method must be used in combination with openConfig().
     *  The cached configuration access must be provided here ... and
     *  all operations are made relativ to this access point.
     *
     *  @param  xCFG
     *          the configuration root, where sRelPath should be interpreted.
     *          as relativ path
     *
     *  @param  sRelPath
     *          path relative to xCFG parameter.
     *
     *  @param  sKey
     *          the configuration node, where we should write the new value.
     *
     *  @param  aValue
     *          the new value for sKey.
     *
     *  @throw  Any exceptions the underlying configuration can throw.
     *          E.g. css::container::NoSuchElementException if the specified
     *          key does not exists or css::uno::Exception if the provided configuration
     *          access does not allow writing for this key.
     */
    static void writeRelativeKey(const css::uno::Reference< css::uno::XInterface >& xCFG    ,
                                 const OUString&                            sRelPath,
                                 const OUString&                            sKey    ,
                                 const css::uno::Any&                              aValue  );


    /** it checks if the specified set node exists ... or create an empty one
     *  otherwise.
     *
     *  This method must be used in combination with openConfig().
     *  The cached configuration access must be provided here ... and
     *  all operations are made relativ to this access point.
     *
     *  Further this method must be used only with configuration set's.
     *  Atomic keys can't be "created" ... they "exists every time".
     *
     *  @param  xCFG
     *          the configuration root, where sRelPathToSet should be interpreted
     *          as relativ path.
     *
     *  @param  sRelPathToSet
     *          path relative to xCFG parameter.
     *
     *  @param  sSetNode
     *          the set node, which should be checked if its exists ...
     *          or which should be created with default values.
     *
     *  @return A reference to the found (or new created) set node.
     *          Can't be NULL .. in such case an exception occurs!
     *
     *  @throw  Any exceptions the underlying configuration can throw.
     *          E.g. css::uno::Exception if the provided configuration
     *          access does not allow writing for this set.
     */
    static css::uno::Reference< css::uno::XInterface > makeSureSetNodeExists(const css::uno::Reference< css::uno::XInterface >& xCFG         ,
                                                                             const OUString&                            sRelPathToSet,
                                                                             const OUString&                            sSetNode     );


    /** commit all changes made on the specified configuration access.
     *
     *  This method must be used in combination with openConfig().
     *  The cached configuration access must be provided here.
     *
     *  @param  xCFG
     *          the configuration root, where changes should be committed.
     *
     *  @throw  Any exceptions the underlying configuration can throw.
     *          E.g. css::uno::Exception if the provided configuration
     *          access does not allow writing for this set.
     */
    static void flush(const css::uno::Reference< css::uno::XInterface >& xCFG);


    /** does the same then openConfig() & readRelativeKey() together.
     *
     * This method should be used for reading one key at one code place only.
     * Because it opens the specified configuration package, reads the key and
     * closes the configuration again.
     *
     * So its not very useful to use this method for reading multiple keys at the same time.
     * (Excepting these keys exists inside different configuration packages ...))
     */
    static css::uno::Any readDirectKey(const css::uno::Reference< css::uno::XComponentContext >&    rxContext,
                                       const OUString&                                       sPackage,
                                       const OUString&                                       sRelPath,
                                       const OUString&                                       sKey    ,
                                             EConfigurationModes                             eMode   );


    /** does the same then openConfig() / writeRelativeKey() & flush() together.
     *
     * This method should be used for writing one key at one code place only.
     * Because it opens the specified configuration package, writes the key, flush
     * all changes and closes the configuration again.
     *
     * So its not very useful to use this method for writing multiple keys at the same time.
     * (Excepting these keys exists inside different configuration packages ...))
     */
    static void writeDirectKey(const css::uno::Reference< css::uno::XComponentContext >&    rxContext,
                               const OUString&                                       sPackage,
                               const OUString&                                       sRelPath,
                               const OUString&                                       sKey    ,
                               const css::uno::Any&                                  aValue  ,
                                     EConfigurationModes                             eMode   );
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_CONFIGURATIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
