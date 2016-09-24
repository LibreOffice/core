/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TUBES_CONSTANTS_H
#define INCLUDED_TUBES_CONSTANTS_H

/* DBusTube.ServiceName.
 *
 * FIXME: Should be something like
 *
 *   org.libreoffice.calc
 *   org.libreoffice.writer
 *
 * etc. This does not need to include the org.freedesktop.Telepathy.Client
 * stuff.
 */
#define LIBO_DTUBE_SERVICE "org.libreoffice.calc"

/* Client name suffix, for passing as 'name' to
 * tp_simple_handler_new_with_am(). */
#define LIBO_CLIENT_SUFFIX "LibreOffice"

/* Key value storing UUID for TeleConference
 */
#define LIBO_TUBES_UUID "LIBO_TUBES_UUID"

#endif // INCLUDED_TUBES_CONSTANTS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
