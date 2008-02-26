/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmhtml.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:57:46 $
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

#ifndef _FRMHTML_HXX
#define _FRMHTML_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxhtml.hxx>

class SfxFrameHTMLParser;
class _SfxFrameHTMLContexts;
class SfxFrameSetObjectShell;
class SvULongs;
class SfxMedium;
namespace svtools { class AsynchronLink; }

class SFX2_DLLPUBLIC SfxFrameHTMLParser : public SfxHTMLParser
{
    friend class _SfxFrameHTMLContext;

protected:
   SfxFrameHTMLParser( SvStream& rStream, BOOL bIsNewDoc=TRUE, SfxMedium *pMediumPtr=0 ):
   SfxHTMLParser( rStream, bIsNewDoc, pMediumPtr ) {};

public:
    // Diese Methoden koennen auch von anderen Parsern benutzt werden
    static void             ParseFrameOptions(SfxFrameDescriptor*, const HTMLOptions*, const String& );

};

#endif

