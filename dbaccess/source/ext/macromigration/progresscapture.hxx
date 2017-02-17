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

#ifndef INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_PROGRESSCAPTURE_HXX
#define INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_PROGRESSCAPTURE_HXX

#include <com/sun/star/task/XStatusIndicator.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>

namespace dbmm
{

    class IMigrationProgress;

    // ProgressCapture
    typedef ::cppu::WeakImplHelper<   css::task::XStatusIndicator
                                  >   ProgressCapture_Base;

    struct ProgressCapture_Data;

    class ProgressCapture : public ProgressCapture_Base
    {
    public:
        ProgressCapture( const OUString& _rObjectName, IMigrationProgress& _rMasterProgress );

        void    dispose();

        // XStatusIndicator
        virtual void SAL_CALL start( const OUString& Text, ::sal_Int32 Range ) override;
        virtual void SAL_CALL end(  ) override;
        virtual void SAL_CALL setText( const OUString& Text ) override;
        virtual void SAL_CALL setValue( ::sal_Int32 Value ) override;
        virtual void SAL_CALL reset(  ) override;

    protected:
        virtual ~ProgressCapture() override;

    private:
        std::unique_ptr< ProgressCapture_Data > m_pData;
    };

} // namespace dbmm

#endif // INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_PROGRESSCAPTURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
