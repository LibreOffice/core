/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#ifndef INCLUDED_BINARYURP_SOURCE_INCOMINGREQUEST_HXX
#define INCLUDED_BINARYURP_SOURCE_INCOMINGREQUEST_HXX

#include "sal/config.h"

#include <vector>

#include "boost/noncopyable.hpp"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"
#include "uno/dispatcher.hxx"

namespace binaryurp {
    class BinaryAny;
    class Bridge;
}

namespace binaryurp {

class IncomingRequest: private boost::noncopyable {
public:
    IncomingRequest(
        rtl::Reference< Bridge > const & bridge, rtl::ByteSequence const & tid,
        rtl::OUString const & oid,
        com::sun::star::uno::UnoInterfaceReference const & object,
        com::sun::star::uno::TypeDescription const & type,
        sal_uInt16 functionId, bool synchronous,
        com::sun::star::uno::TypeDescription const & member, bool setter,
        std::vector< BinaryAny > const & inArguments, bool currentContextMode,
        com::sun::star::uno::UnoInterfaceReference const & currentContext);

    ~IncomingRequest();

    void execute() const;

private:
    bool execute_throw(
        BinaryAny * returnValue, std::vector< BinaryAny > * outArguments) const;

    rtl::Reference< Bridge > bridge_;
    rtl::ByteSequence tid_;
    rtl::OUString oid_; // initial object queryInterface; release
    com::sun::star::uno::UnoInterfaceReference object_;
    com::sun::star::uno::TypeDescription type_;
    sal_uInt16 functionId_;
    bool synchronous_;
    com::sun::star::uno::TypeDescription member_;
    bool setter_;
    std::vector< BinaryAny > inArguments_;
    bool currentContextMode_;
    com::sun::star::uno::UnoInterfaceReference currentContext_;
};

}

#endif
