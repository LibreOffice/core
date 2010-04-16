/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jobconst.cxx,v $
 * $Revision: 1.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//________________________________
//  my own includes

#include <jobs/jobconst.hxx>

//________________________________
//  interface includes

//________________________________
//  includes of other projects

//________________________________
//  namespace

namespace framework{

//________________________________
//  non exported const

//________________________________
//  non exported definitions

//________________________________
//  declarations

const ::rtl::OUString JobConst::ANSWER_DEACTIVATE_JOB()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("Deactivate");
    return PROP;
}

const ::rtl::OUString JobConst::ANSWER_SAVE_ARGUMENTS()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("SaveArguments");
    return PROP;
}

const ::rtl::OUString JobConst::ANSWER_SEND_DISPATCHRESULT()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("SendDispatchResult");
    return PROP;
}

} // namespace framework
