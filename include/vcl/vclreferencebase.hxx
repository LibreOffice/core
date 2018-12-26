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
#ifndef INCLUDED_VCL_Reference_HXX
#define INCLUDED_VCL_Reference_HXX

#include <vcl/dllapi.h>
#include <osl/interlck.h>

class VCL_DLLPUBLIC VclReferenceBase
{
    mutable oslInterlockedCount mnRefCnt;

    template<typename T> friend class VclPtr;

public:
    void acquire() const
    {
        osl_atomic_increment(&mnRefCnt);
    }

    void release() const
    {
        if (osl_atomic_decrement(&mnRefCnt) == 0)
            delete this;
    }
#ifdef DBG_UTIL
#ifndef WNT
    sal_Int32 getRefCount() const { return mnRefCnt; }
#endif
#endif


private:
    VclReferenceBase(const VclReferenceBase&) = delete;
    VclReferenceBase& operator=(const VclReferenceBase&) = delete;

    bool                        mbDisposed : 1;

protected:
                                VclReferenceBase();
protected:
    virtual                     ~VclReferenceBase();

protected:
    virtual void                dispose();

public:
    void                        disposeOnce();
    bool                        isDisposed() const { return mbDisposed; }

};
#endif
