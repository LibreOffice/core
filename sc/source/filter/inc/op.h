/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_OP_H
#define SC_OP_H

#include <patattr.hxx>

// OP-Code-Funktionen
class SvStream;
void NI( SvStream &aStream, sal_uInt16 nLaenge );
void OP_BOF( SvStream &aStream, sal_uInt16 nLaenge );
void OP_EOF( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Integer( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Number( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Label( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Integer3( SvStream &aStream, sal_uInt16 nLaenge );  // WK3
void OP_Number3( SvStream &aStream, sal_uInt16 nLaenge );   // WK3
void OP_Formula( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Formula3( SvStream &aStream, sal_uInt16 nLaenge );  // WK3
void OP_ColumnWidth( SvStream &aStream, sal_uInt16 nLaenge );
void OP_NamedRange( SvStream &aStream, sal_uInt16 nLaenge );
void OP_SymphNamedRange( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Footer( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Header( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Margins( SvStream &aStream, sal_uInt16 nLaenge );
void OP_HiddenCols( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Window1( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Blank( SvStream &aStream, sal_uInt16 nLaenge );
// Lotus 123 bits.
void OP_BOF123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_EOF123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Number123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Label123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_Formula123( SvStream &aStream, sal_uInt16 nLaenge );
void OP_IEEENumber123(SvStream& r, sal_uInt16 n);
void OP_Note123(SvStream &aStream, sal_uInt16 nLaenge);
void OP_CreatePattern123(SvStream &aStream, sal_uInt16 nLaenge);
void OP_SheetName123( SvStream &rStream, sal_uInt16 nLength );
void OP_HorAlign123(sal_uInt8 nAlignPattern, SfxItemSet& rPattern /*  const ScPatternAttr& rPattern*/  );
void OP_VerAlign123(sal_uInt8 nAlignPattern, SfxItemSet& rPattern /*  const ScPatternAttr& rPattern*/  );
void OP_ApplyPatternArea123(SvStream& r);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
