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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _ENVIMG_HXX
#include <envimg.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
namespace binfilter {

#ifdef WIN
#define NEXTLINE  UniString::CreateFromAscii("\r\n")
#else
#define NEXTLINE  '\n'
#endif

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)

/*N*/ TYPEINIT1_AUTOFACTORY( SwEnvItem, SfxPoolItem );

/*N*/ SwEnvItem::SwEnvItem() :
/*N*/ 	SfxPoolItem(FN_ENVELOP)
/*N*/ {
/*N*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	aAddrText       = aEmptyStr;
/*N*/ }
// --------------------------------------------------------------------------
/*N*/ SwEnvItem::SwEnvItem(const SwEnvItem& rItem) :
/*N*/ 	SfxPoolItem(FN_ENVELOP),
/*N*/ 	aAddrText      (rItem.aAddrText),
/*N*/ 	bSend          (rItem.bSend),
/*N*/ 	aSendText      (rItem.aSendText),
/*N*/ 	lSendFromLeft  (rItem.lSendFromLeft),
/*N*/ 	lSendFromTop   (rItem.lSendFromTop),
/*N*/ 	lAddrFromLeft  (rItem.lAddrFromLeft),
/*N*/ 	lAddrFromTop   (rItem.lAddrFromTop),
/*N*/ 	lWidth         (rItem.lWidth),
/*N*/ 	lHeight        (rItem.lHeight),
/*N*/ 	eAlign         (rItem.eAlign),
/*N*/ 	bPrintFromAbove(rItem.bPrintFromAbove),
/*N*/ 	lShiftRight    (rItem.lShiftRight),
/*N*/ 	lShiftDown     (rItem.lShiftDown)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

// --------------------------------------------------------------------------
/*N*/ int SwEnvItem::operator ==(const SfxPoolItem& rItem) const
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	const SwEnvItem& rEnv = (const SwEnvItem&) rItem;
/*N*/ }

// --------------------------------------------------------------------------
/*N*/ SfxPoolItem* SwEnvItem::Clone(SfxItemPool*) const
/*N*/ {
/*N*/ 	return new SwEnvItem(*this);
/*N*/ }
// --------------------------------------------------------------------------
}
