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

#ifndef _SVT_DOCUMENTLOCKFILE_HXX
#define _SVT_DOCUMENTLOCKFILE_HXX

#include <svl/svldllapi.h>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <svl/lockfilecommon.hxx>

namespace svt {

class SVL_DLLPUBLIC DocumentLockFile : public LockFileCommon
{
    // the workaround for automated testing!
    static sal_Bool m_bAllowInteraction;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > OpenStream();

    void WriteEntryToStream( ::com::sun::star::uno::Sequence< ::rtl::OUString > aEntry, ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xStream );

public:
    DocumentLockFile( const ::rtl::OUString& aOrigURL, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() );
    ~DocumentLockFile();

    sal_Bool CreateOwnLockFile();
    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetLockData();
    sal_Bool OverwriteOwnLockFile();
    void RemoveFile();

    // the methods allow to control whether UI interaction regarding the locked document file is allowed
    // this is a workaround for automated tests
    static void AllowInteraction( sal_Bool bAllow ) { m_bAllowInteraction = bAllow; }
    static sal_Bool IsInteractionAllowed() { return m_bAllowInteraction; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
