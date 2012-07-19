/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Collabora Ltd.
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
