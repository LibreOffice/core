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

#ifndef GIO_MOUNT_HXX
#define GIO_MOUNT_HXX

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <gio/gio.h>

G_BEGIN_DECLS

#define OOO_TYPE_MOUNT_OPERATION         (ooo_mount_operation_get_type ())
#define OOO_MOUNT_OPERATION(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), OOO_TYPE_MOUNT_OPERATION, OOoMountOperation))
#define OOO_MOUNT_OPERATION_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), OOO_TYPE_MOUNT_OPERATION, OOoMountOperationClass))
#define OOO_IS_MOUNT_OPERATION(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), OOO_TYPE_MOUNT_OPERATION))
#define OOO_IS_MOUNT_OPERATION_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), OOO_TYPE_MOUNT_OPERATION))
#define OOO_MOUNT_OPERATION_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), OOO_TYPE_MOUNT_OPERATION, OOoMountOperationClass))

struct OOoMountOperation
{
    GMountOperation parent_instance;

    const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > *pEnv;
    char *m_pPrevUsername;
    char *m_pPrevPassword;
};

struct OOoMountOperationClass
{
    GMountOperationClass parent_class;

    /* Padding for future expansion */
    void (*_gtk_reserved1) (void);
    void (*_gtk_reserved2) (void);
    void (*_gtk_reserved3) (void);
    void (*_gtk_reserved4) (void);
};


GType            ooo_mount_operation_get_type   (void);
GMountOperation *ooo_mount_operation_new(const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& rEnv);

G_END_DECLS
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
