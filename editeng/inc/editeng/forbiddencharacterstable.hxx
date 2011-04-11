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

#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#define _FORBIDDENCHARACTERSTABLE_HXX

#include <tools/table.hxx>

#include <salhelper/simplereferenceobject.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#include "editeng/editengdllapi.h"

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory;
}}}}

struct ForbiddenCharactersInfo
{
    com::sun::star::i18n::ForbiddenCharacters aForbiddenChars;
    sal_Bool bTemporary;
};

DECLARE_TABLE( SvxForbiddenCharactersTableImpl, ForbiddenCharactersInfo* )

class EDITENG_DLLPUBLIC SvxForbiddenCharactersTable : public SvxForbiddenCharactersTableImpl, public salhelper::SimpleReferenceObject
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;

public:
            SvxForbiddenCharactersTable( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF, sal_uInt16 nISize = 4, sal_uInt16 nGrow = 4 );
            ~SvxForbiddenCharactersTable();

    const com::sun::star::i18n::ForbiddenCharacters* GetForbiddenCharacters( sal_uInt16 nLanuage, sal_Bool bGetDefault ) const;
    void    SetForbiddenCharacters(  sal_uInt16 nLanuage , const com::sun::star::i18n::ForbiddenCharacters& );
    void    ClearForbiddenCharacters( sal_uInt16 nLanuage );
};

#endif // _FORBIDDENCHARACTERSTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
