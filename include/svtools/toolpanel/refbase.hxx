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

#ifndef INCLUDED_SVTOOLS_TOOLPANEL_REFBASE_HXX
#define INCLUDED_SVTOOLS_TOOLPANEL_REFBASE_HXX

#include <svtools/svtdllapi.h>

#include <rtl/ref.hxx>


namespace svt
{



    //= RefBase

    class SVT_DLLPUBLIC RefBase : public ::rtl::IReference
    {
    protected:
        RefBase()
            :m_refCount( 0 )
        {
        }

        virtual ~RefBase()
        {
        }

        virtual oslInterlockedCount SAL_CALL acquire() SAL_OVERRIDE;
        virtual oslInterlockedCount SAL_CALL release() SAL_OVERRIDE;

    private:
        oslInterlockedCount m_refCount;
    };

#define DECLARE_IREFERENCE()    \
    virtual oslInterlockedCount SAL_CALL acquire() SAL_OVERRIDE; \
    virtual oslInterlockedCount SAL_CALL release() SAL_OVERRIDE;


#define IMPLEMENT_IREFERENCE( classname )   \
    oslInterlockedCount classname::acquire()    \
    {   \
        return RefBase::acquire();  \
    }   \
    oslInterlockedCount classname::release()    \
    {   \
        return RefBase::release();  \
    }


} // namespace svt


#endif // INCLUDED_SVTOOLS_TOOLPANEL_REFBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
