/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sndstyle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:11:00 $
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

#ifndef _SV_SNDSTYLE_HXX
#define _SV_SNDSTYLE_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

// ---------------
// - Sound-Types -
// ---------------

typedef USHORT SoundType;
#define SOUND_DEFAULT                   ((SoundType)0)
#define SOUND_INFO                      ((SoundType)1)
#define SOUND_WARNING                   ((SoundType)2)
#define SOUND_ERROR                     ((SoundType)3)
#define SOUND_QUERY                     ((SoundType)4)

#define SOUND_DISABLE                   (SOUND_DEFAULT)

#endif // _SV_SNDSTYLE_HXX
