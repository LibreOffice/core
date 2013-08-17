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

#ifndef DBACCESS_MIGRATIONPROGRESS_HXX
#define DBACCESS_MIGRATIONPROGRESS_HXX

namespace dbmm
{

    // IMigrationProgress
    class SAL_NO_VTABLE IMigrationProgress
    {
    public:
        virtual void    startObject( const OUString& _rObjectName, const OUString& _rCurrentAction, const sal_uInt32 _nRange ) = 0;
        virtual void    setObjectProgressText( const OUString& _rText ) = 0;
        virtual void    setObjectProgressValue( const sal_uInt32 _nValue ) = 0;
        virtual void    endObject() = 0;
        virtual void    start( const sal_uInt32 _nOverallRange ) = 0;
        virtual void    setOverallProgressText( const OUString& _rText ) = 0;
        virtual void    setOverallProgressValue( const sal_uInt32 _nValue ) = 0;

    protected:
        ~IMigrationProgress() {}
    };

} // namespace dbmm

#endif // DBACCESS_MIGRATIONPROGRESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
