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
