/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: openflag.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:08:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFX_OPENFLAG_HXX
#define _SFX_OPENFLAG_HXX

// Datei zum Bearbeiten "offnen, anschliessend funktioniert nur noch
// die dritte Variante (Lesen einer Kopie)
#define SFX_STREAM_READWRITE  (STREAM_READWRITE |  STREAM_SHARE_DENYWRITE)
// Ich arbeite roh auf dem Original, keine Kopie
// -> Datei kann anschliessend nicht zum Bearbeiten ge"offnet werden
#define SFX_STREAM_READONLY   (STREAM_READ | STREAM_SHARE_DENYWRITE) // + !bDirect
// Jemand anders bearbeitet das File, es wird eine Kopie erstellt
// -> Datei kann anschliessend zum Bearbeiten ge"offnet werden
#define SFX_STREAM_READONLY_MAKECOPY   (STREAM_READ | STREAM_SHARE_DENYNONE)


#endif
