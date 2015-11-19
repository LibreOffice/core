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

/*TODO
    - late init
    - order by number!
    - insert default detector and loader as last ones in hashes ... don't hold it as an extra member!
      => CheckedIterator will be obsolete!
 */

#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_FILTERCACHE_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_FILTERCACHE_HXX

#include <classes/filtercachedata.hxx>

#include <threadhelp/transactionbase.hxx>
#include <general.h>
#include <queries.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>

#include <rtl/ustring.hxx>

#ifdef ENABLE_GENERATEFILTERCACHE
    #include <rtl/ustrbuf.hxx>
#endif

namespace framework{

/*-************************************************************************************************************
    @short          cache for all filter and type information
    @descr          Frameloader - and filterfactory need some information about our current registered filters and types.
                    For better performance its necessary to cache all needed values.
    @base           TransactionBase

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/

class FilterCache   :   private TransactionBase
{
    public:

    //  public methods

    public:

        //  constructor / destructor

        FilterCache( sal_Int32      nVersion = DEFAULT_FILTERCACHE_VERSION,
                     ConfigItemMode nMode    = DEFAULT_FILTERCACHE_MODE   );

        /*-****************************************************************************************************
            @short      standard destructor to delete instance
            @descr      This will clear the cache if last owner release it.
        *//*-*****************************************************************************************************/

        virtual ~FilterCache();

    //  private variables

    private:

        static sal_Int32        m_nRefCount;
        static sal_Int32        m_nVersion;
        static sal_Int16        m_nMode;

};      //  class FilterCache

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_FILTERCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
