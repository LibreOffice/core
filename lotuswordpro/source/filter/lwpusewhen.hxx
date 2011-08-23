/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef _LWPUSEWHEN_HXX
#define _LWPUSEWHEN_HXX

#include "lwpheader.hxx"

#define STYLE_USEONALLPAGES		0x0001U	/* repeat on all pages */
#define STYLE_USEONALLEVENPAGES	0x0002U	/* repeat on all even pages */
#define STYLE_USEONALLODDPAGES	0x0004U	/* repeat on all odd pages */
#define STYLE_USEONTHISPAGE		0x0008U	/* use this guy on this page */
#define STYLE_USEEXCEPTTHISPAGE	0x0010U	/* use guy except on this page */
#define STYLE_USESTARTINGONPAGE	0x0020U	/* use starting on page n */
#define STYLE_USEONMASK			(STYLE_USEONALLPAGES | \
                                    STYLE_USEONALLEVENPAGES | \
                                    STYLE_USEONALLODDPAGES | \
                                    STYLE_USEONTHISPAGE | \
                                    STYLE_USEEXCEPTTHISPAGE | \
                                    STYLE_USESTARTINGONPAGE)

#define STYLE_STARTONNEXTPAGE	0x0000U	// This is the default
#define STYLE_STARTONTHISPAGE	0x0040U
#define STYLE_STARTONNEXTODD	0x0080U
#define STYLE_STARTONNEXTEVEN	0x0100U
#define STYLE_STARTONTHISHF		0x0200U
#define STYLE_STARTONMASK		(STYLE_STARTONTHISPAGE | \
                                    STYLE_STARTONNEXTODD | \
                                    STYLE_STARTONNEXTEVEN | \
                                    STYLE_STARTONTHISHF)
class LwpUseWhen
{
public:
    LwpUseWhen() : m_nFlags(0), m_nUsePage(0) {}

    inline void Read(LwpObjectStream* pStrm);

    inline sal_Bool IsUseOnAllPages();

    inline sal_Bool IsUseOnAllEvenPages();

    inline sal_Bool IsUseOnAllOddPages();

    inline sal_Bool IsUseOnPage();

    inline sal_Bool IsStartOnThisPage();

    inline sal_Bool IsStartOnNextPage();

    inline sal_Bool IsStartOnNextEvenPage();

    inline sal_Bool IsStartOnNextOddPage();

    inline sal_Bool IsStartOnThisHF();

    inline sal_uInt16 GetUsePage();
private:
    sal_uInt16	m_nFlags;
    sal_uInt16	m_nUsePage;
};

inline void LwpUseWhen::Read(LwpObjectStream* pStrm)
{
    pStrm->QuickRead(&m_nFlags, 2);
    pStrm->QuickRead(&m_nUsePage, 2);
    pStrm->SkipExtra();
}
inline sal_Bool LwpUseWhen::IsUseOnAllPages()
{
    return (sal_Bool)((m_nFlags & STYLE_USEONALLPAGES) != 0);
}
inline sal_Bool LwpUseWhen::IsUseOnAllEvenPages()
{
    return (sal_Bool)((m_nFlags & STYLE_USEONALLEVENPAGES) != 0);
}
inline sal_Bool LwpUseWhen::IsUseOnAllOddPages()
{
    return (sal_Bool)((m_nFlags & STYLE_USEONALLODDPAGES) != 0);
}

inline sal_Bool LwpUseWhen::IsUseOnPage()
{
    return (sal_Bool)((m_nFlags & STYLE_USEONTHISPAGE) != 0);
}

inline sal_Bool LwpUseWhen::IsStartOnThisPage()
{
    return (sal_Bool)((m_nFlags & STYLE_STARTONTHISPAGE) != 0);
}

inline sal_Bool LwpUseWhen::IsStartOnNextPage()
{
    return (sal_Bool)((m_nFlags & STYLE_STARTONMASK) == 0);
}

inline sal_Bool LwpUseWhen::IsStartOnNextOddPage()
{
    return (sal_Bool)((m_nFlags & STYLE_STARTONNEXTODD) != 0);
}

inline sal_Bool LwpUseWhen::IsStartOnNextEvenPage()
{
    return (sal_Bool)((m_nFlags & STYLE_STARTONNEXTEVEN) != 0);
}

inline sal_Bool LwpUseWhen::IsStartOnThisHF()
{
    return (sal_Bool)((m_nFlags & STYLE_STARTONTHISHF) != 0);
}

inline sal_uInt16 LwpUseWhen::GetUsePage()
{
    return m_nUsePage;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
