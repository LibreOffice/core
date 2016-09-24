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

#ifndef INCLUDED_EDITENG_INC_EDITXML_HXX
#define INCLUDED_EDITENG_INC_EDITXML_HXX

class EditEngine;
class SvStream;
struct ESelection;

/** this function exports the selected content of an edit engine into a xml stream*/
extern void SvxWriteXML( EditEngine& rEditEngine, SvStream& rStream, const ESelection& rSel );

/** this function imports xml from the stream into the selected of an edit engine */
extern void SvxReadXML( EditEngine& rEditEngine, SvStream& rStream, const ESelection& rSel );

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
