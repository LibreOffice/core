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
#ifndef _SFX_OPENFLAG_HXX
#define _SFX_OPENFLAG_HXX
namespace binfilter {

// Datei zum Bearbeiten "offnen, anschliessend funktioniert nur noch
// die dritte Variante (Lesen einer Kopie)
#define SFX_STREAM_READWRITE  (STREAM_READWRITE |  STREAM_SHARE_DENYWRITE)
// Ich arbeite roh auf dem Original, keine Kopie
// -> Datei kann anschliessend nicht zum Bearbeiten ge"offnet werden
#define SFX_STREAM_READONLY   (STREAM_READ | STREAM_SHARE_DENYWRITE) // + !bDirect
// Jemand anders bearbeitet das File, es wird eine Kopie erstellt
// -> Datei kann anschliessend zum Bearbeiten ge"offnet werden
#define SFX_STREAM_READONLY_MAKECOPY   (STREAM_READ | STREAM_SHARE_DENYNONE)


}//end of namespace binfilter
#endif
