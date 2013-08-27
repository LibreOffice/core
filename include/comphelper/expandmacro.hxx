/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMPHELPER_EXPANDMACRO_HXX_INCLUDED
#define COMPHELPER_EXPANDMACRO_HXX_INCLUDED
#include "rtl/ustring.hxx"
#include "comphelper/comphelperdllapi.h"
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/processfactory.hxx>

namespace comphelper
{
    /**
     A helper function to get expanded version of macro for system filepath.

     This function is supposed to expand the macro
     "vnd.sun.star.expand:" to appropriate file path.And it is used
     for configuration files.(for e.g ,refer to macros present at
     'Impress.xcs' and where it is used to expand prop
     value of "LayoutListFiles")

     @param filepath refers to the system filepath that is to be expanded.

     @return macro expansion of system filepath.
    */
    COMPHELPER_DLLPUBLIC rtl::OUString getExpandedFilePath(const rtl::OUString& filepath);
    /**
     A helper function to get expanded version of macro for system filepath.

     This function is supposed to expand the macro
     "vnd.sun.star.expand:" to appropriate file path.And it is used
     for configuration files.(for e.g ,refer to macros present at
     'Impress.xcs' and where it is used to expand prop
     value of "LayoutListFiles")

     @param filepath refers to the system filepath that is to be expanded.
     @param xContext refers to the component context of the process.

     @return macro expansion of system filepath.
    */
    COMPHELPER_DLLPUBLIC rtl::OUString getExpandedFilePath(const rtl::OUString& filepath,const ::com::sun::star::uno::Reference<css::uno::XComponentContext>& xContext);
}
#endif