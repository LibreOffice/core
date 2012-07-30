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
#ifndef REPORTDESIGN_CORE_RESOURCE_HXX_
#define REPORTDESIGN_CORE_RESOURCE_HXX_


#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

class SimpleResMgr;
//.........................................................................
namespace reportdesign
{

#define RPT_RESSTRING(id,_rM) ResourceManager::loadString(id,_rM)

    //==================================================================
    //= ResourceManager
    //= handling resources within the DBA-Core library
    //==================================================================
    class ResourceManager
    {
        static SimpleResMgr*    m_pImpl;

    private:
        // no instantiation allowed
        ResourceManager() { }
        ~ResourceManager() { }

        // we'll instantiate one static member of the following class, which, in it's dtor,
        // ensures that m_pImpl will be deleted
        class EnsureDelete
        {
        public:
            EnsureDelete() { }
            ~EnsureDelete();
        };
        friend class EnsureDelete;

    protected:
        static void ensureImplExists(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >& _rM);

    public:
        /** loads the string with the specified resource id from the FormLayer resource file
        */
        static ::rtl::OUString loadString(sal_uInt16 _nResId,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >& _rM);
    };

//.........................................................................
}
//.........................................................................

#endif // REPORTDESIGN_CORE_RESOURCE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
