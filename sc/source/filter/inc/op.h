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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_OP_H
#define INCLUDED_SC_SOURCE_FILTER_INC_OP_H

#include <patattr.hxx>

// OP-Code-Funktionen
class SvStream;
struct LotusContext;
void NI(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_BOF(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_EOF(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Integer(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Number(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Label(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Formula(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_ColumnWidth(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_NamedRange(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_SymphNamedRange(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Footer(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Header(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Margins(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_HiddenCols(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Window1(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Blank(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
// Lotus 123 bits.
void OP_BOF123(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_EOF123(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Number123(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Label123(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_Formula123(LotusContext &rContext, SvStream &aStream, sal_uInt16 nLaenge );
void OP_IEEENumber123(LotusContext &rContext,SvStream& r, sal_uInt16 n);
void OP_Note123(LotusContext &rContext,SvStream &aStream, sal_uInt16 nLaenge);
void OP_CreatePattern123(LotusContext &rContext,SvStream &aStream, sal_uInt16 nLaenge);
void OP_SheetName123(LotusContext &rContext, SvStream &rStream, sal_uInt16 nLength );
void OP_HorAlign123(LotusContext &rContext,sal_uInt8 nAlignPattern, SfxItemSet& rPattern /*  const ScPatternAttr& rPattern*/  );
void OP_VerAlign123(LotusContext &rContext,sal_uInt8 nAlignPattern, SfxItemSet& rPattern /*  const ScPatternAttr& rPattern*/  );
void OP_ApplyPatternArea123(LotusContext &rContext,SvStream& r);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
