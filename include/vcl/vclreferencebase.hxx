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
#include <vcl/vclptr.hxx>
#include <cassert>

class VclReferenceBase;

class VCL_DLLPUBLIC VclReferenceBase
{
    mutable int mnRefCnt;

    template<typename T> friend class ::rtl::Reference;
    template<typename T> friend class ::VclPtr;

public:
    inline void acquire() const
    {
        assert(mnRefCnt>0);
        mnRefCnt++;
    }

    inline void release() const
    {
        assert(mnRefCnt>0);
        if (!--mnRefCnt)
            delete this;
    }
private:
    VclReferenceBase(const VclReferenceBase&) = delete;
    VclReferenceBase& operator=(const VclReferenceBase&) = delete;

    bool                        mbDisposed : 1;

protected:
                                VclReferenceBase();
public:
    virtual                     ~VclReferenceBase();

protected:
    virtual void                dispose();

public:
    void                        disposeOnce();
    bool                        isDisposed() const { return mbDisposed; }

};
#endif
