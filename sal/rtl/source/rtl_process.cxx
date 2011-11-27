/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "sal/config.h"

#include <cstring>

#include "boost/noncopyable.hpp"
#include "rtl/instance.hxx"
#include "rtl/process.h"
#include "rtl/uuid.h"
#include "sal/types.h"

namespace {

class Id: private boost::noncopyable {
public:
    Id() { rtl_createUuid(uuid_, 0, false); }

    void copy(sal_uInt8 * target) const
    { std::memcpy(target, uuid_, UUID_SIZE); }

private:
    enum { UUID_SIZE = 16 };

    sal_uInt8 uuid_[UUID_SIZE];
};

struct theId: public rtl::Static< Id, theId > {};

}

void rtl_getGlobalProcessId(sal_uInt8 * pTargetUUID) {
    theId::get().copy(pTargetUUID);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
