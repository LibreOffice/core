/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#ifndef INCLUDED_CONFIGMGR_STATUS_HXX
#define INCLUDED_CONFIGMGR_STATUS_HXX

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "access.hxx"

namespace configmgr {

class Status: public boost::noncopyable {
public:
    virtual ~Status() /* = 0 */ {}

protected:
    Status() {}
};

class ChangedStatus: public Status {
public:
    explicit ChangedStatus(com::sun::star::uno::Any const & value):
        value_(value) {}

    com::sun::star::uno::Any getValue() const { return value_; }

private:
    virtual ~ChangedStatus() {}

    com::sun::star::uno::Any value_;
};

class RemovedStatus: public Status {
public:
    RemovedStatus() {}

private:
    virtual ~RemovedStatus() {}
};

class InsertedStatus: public Status {
public:
    InsertedStatus() {}

private:
    virtual ~InsertedStatus() {}
};

class TransferedStatus: public Status {
public:
    TransferedStatus(
        rtl::Reference< Access > const & oldParent,
        rtl::OUString const & oldName):
        oldParent_(oldParent), oldName_(oldName)
    { OSL_ASSERT(oldParent.is() && oldName.getLength() != 0); }

    rtl::Reference< Access > getOldParent() const { return oldParent_; }

    rtl::OUString getOldName() const { return oldName_; }

private:
    virtual ~TransferedStatus() {}

    rtl::Reference< Access > oldParent_;
    rtl::OUString oldName_;
};

}

#endif
