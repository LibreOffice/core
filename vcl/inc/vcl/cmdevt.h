/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cmdevt.h,v $
 * $Revision: 1.3 $
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

#ifndef _VCL_CMDEVT_H
#define _VCL_CMDEVT_H

// --------------------
// - CommandVoiceData -
// --------------------

#define DICTATIONCOMMAND_SYSTEM         0x1000
#define DICTATIONCOMMAND_USER           0x2000

#define DICTATIONCOMMAND_UNKNOWN        0x0001
#define DICTATIONCOMMAND_NEWPARAGRAPH   0x0002
#define DICTATIONCOMMAND_NEWLINE        0x0003
#define DICTATIONCOMMAND_BOLD_ON        0x0004
#define DICTATIONCOMMAND_BOLD_OFF       0x0005
#define DICTATIONCOMMAND_ITALIC_ON      0x0006
#define DICTATIONCOMMAND_ITALIC_OFF     0x0007
#define DICTATIONCOMMAND_UNDERLINE_ON   0x0008
#define DICTATIONCOMMAND_UNDERLINE_OFF  0x0009
#define DICTATIONCOMMAND_NUMBERING_ON   0x0010
#define DICTATIONCOMMAND_NUMBERING_OFF  0x0011
#define DICTATIONCOMMAND_TAB            0x0012
#define DICTATIONCOMMAND_LEFT           0x0013
#define DICTATIONCOMMAND_RIGHT          0x0014
#define DICTATIONCOMMAND_UP             0x0015
#define DICTATIONCOMMAND_DOWN           0x0016
#define DICTATIONCOMMAND_UNDO           0x0017
#define DICTATIONCOMMAND_REPEAT         0x0018
#define DICTATIONCOMMAND_DEL            0x0019

#define DICTATIONCOMMAND_COMMANDMODE    (DICTATIONCOMMAND_SYSTEM + 1)
#define DICTATIONCOMMAND_DICTATIONMODE  (DICTATIONCOMMAND_SYSTEM + 2)

#endif // _VCL_CMDEVT_H


