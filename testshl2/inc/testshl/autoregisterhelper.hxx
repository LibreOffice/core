/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef AUTOREGISTER_HXX
#define AUTOREGISTER_HXX

#include <vector>
#include <rtl/ustring.hxx>

#include <testshl/autoregister/testfunc.h>

#include "testshl/dynamicregister.hxx"
#include "testshl/getopt.hxx"

typedef std::vector<FktPtr> FunctionList;

// -----------------------------------------------------------------------------
class JobList;
class AutomaticRegisterHelper : public DynamicLibraryHelper
{
    FunctionList m_aFunctionList;
    bool m_bLoadLibraryOK;
    // GetOpt & m_aOptions; // use getOptions() instead!

public:
    AutomaticRegisterHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions/* , JobList * _pJobList = NULL */ );

    // FunctionList getFunctions(){return m_aFunctionList;}
    void CallAll(hTestResult _hResult) const;

    /// @return true, if the given DLLName library could load and initialised.
    bool isOkToStartTests() const {return m_bLoadLibraryOK;}

    virtual ~AutomaticRegisterHelper();
};

#endif

